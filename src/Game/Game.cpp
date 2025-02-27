#include "Game.h"
#include "LevelLoader.h"
#include "../Logger/Logger.h"
#include "../ECS/ECS.h"

#include "../Systems/MovementSystem.h"
#include "../Systems/RenderSystem.h"
#include "../Systems/AnimationSystem.h"
#include "../Systems/CollisionSystem.h"
#include "../Systems/RenderColliderSystem.h"
#include "../Systems/DamageSystem.h"
#include "../Systems/KeyboardControlSystem.h"
#include "../Systems/CameraMovementSystem.h"
#include "../Systems/ProjectileEmitSystem.h"
#include "../Systems/ProjectileLifecycleSystem.h"
#include "../Systems/RenderTextSystem.h"
#include "../Systems/RenderHealthBarSystem.h"
#include "../Systems/RenderGUISystem.h"
#include "../Systems/ScriptSystem.h"
#include <SDL2/SDL.h>
#include <glm/glm.hpp>
#include <imgui/imgui.h>
#include <imgui/imgui_sdl.h>
#include <imgui/imgui_impl_sdl.h>
#include <SDL2/SDL_image.h>
#include <fstream>
#include <iostream>

int Game::windowWidth;
int Game::windowHeight;
int Game::mapWidth;
int Game::mapHeight;

Game::Game() {
    isRunning = false;
    isDebug = false;
    registry = std::make_unique<Registry>();
    assetStore = std::make_unique<AssetStore>();
    eventBus = std::make_unique<EventBus>();
    Logger::Log("Game constructor called");
}

Game::~Game() {
    Logger::Log("Game destructor called");
}

void Game::Initialize() {
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        Logger::Err("Error initializing SDL.");
        return;
    }

    if (TTF_Init() != 0) {
        Logger::Err("Error initializing TTF.");
        return;
    }

    SDL_DisplayMode displayMode;
    SDL_GetCurrentDisplayMode(0, &displayMode); // populate paremeters to displayMode

    windowWidth = 1200;
    windowHeight = 900;

    window = SDL_CreateWindow(
        "My Game Engine", 
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        windowWidth,
        windowHeight,
        SDL_WINDOW_BORDERLESS
    );

    if (!window) {
        Logger::Err("Error creating SDL window.");
        return; 
    }

    renderer = SDL_CreateRenderer(window, -1, 0);

    if (!renderer) {
        Logger::Err("Error creating SDL renderer.");
        return;
    }

    // initialize ImGui context
    ImGui::CreateContext();
    ImGuiSDL::Initialize(renderer, windowWidth, windowHeight);

    // initilize camera view with entire screen area
    camera.x = 0;
    camera.y = 0;
    camera.w = windowWidth;
    camera.h = windowHeight;


    // SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);
    isRunning = true;
}


void Game::ProcessInput() {
    SDL_Event sdlEvent;
    while (SDL_PollEvent(&sdlEvent)) {
        // ImGui SDL input
        ImGui_ImplSDL2_ProcessEvent(&sdlEvent);
        ImGuiIO& io = ImGui::GetIO();
        int mouseX, mouseY;
        const int buttons = SDL_GetMouseState(&mouseX, &mouseY);
        io.MousePos = ImVec2(mouseX, mouseY);
        io.MouseDown[0] = buttons & SDL_BUTTON(SDL_BUTTON_LEFT);
        io.MouseDown[1] = buttons & SDL_BUTTON(SDL_BUTTON_RIGHT);

        switch (sdlEvent.type) {
            case SDL_QUIT:
                isRunning = false;
                break;
            case SDL_KEYDOWN:
                if (sdlEvent.key.keysym.sym == SDLK_ESCAPE) {
                    isRunning = false;
                }
                if (sdlEvent.key.keysym.sym == SDLK_d) {
                    isDebug = !isDebug;
                }
                eventBus->EmitEvent<KeyPressedEvent>(sdlEvent.key.keysym.sym);
                break;

        }
    };
}

void Game::Setup() {
    // add the systems that need to be processed in game
    registry->AddSystem<MovementSystem>();
    registry->AddSystem<RenderSystem>();
    registry->AddSystem<AnimationSystem>();
    registry->AddSystem<CollisionSystem>();
    registry->AddSystem<RenderColliderSystem>();
    registry->AddSystem<DamageSystem>();
    registry->AddSystem<KeyboardControlSystem>();
    registry->AddSystem<CameraMovementSystem>();
    registry->AddSystem<ProjectileEmitSystem>();
    registry->AddSystem<ProjectileLifecycleSystem>();
    registry->AddSystem<RenderTextSystem>();
    registry->AddSystem<RenderHealthBarSystem>();
    registry->AddSystem<RenderGUISystem>();
    registry->AddSystem<ScriptSystem>();

    // create bindings between C++ and lua
    registry->GetSystem<ScriptSystem>().CreateLuaBindings(lua);

    LevelLoader loader;
    lua.open_libraries(sol::lib::base, sol::lib::math, sol::lib::os);
    loader.LoadLevel(lua, registry, assetStore, renderer, 2);
}

void Game::Run() {
    Setup();
    while (isRunning) {
        ProcessInput();
        Update();
        Render();
    }
}

void Game::Update() {
    // if too fast, wait until required time elapse
    int timeToWait = MILLISECS_PER_FRAME - (SDL_GetTicks() - millisecsPreviousFrame);
    
    if (timeToWait > 0 && timeToWait <= MILLISECS_PER_FRAME) {
        SDL_Delay(timeToWait);
    }

    // the difference in ticks since the last frame
    double deltaTime = (SDL_GetTicks() - millisecsPreviousFrame) / 1000.0;

    // store the current frame time
    millisecsPreviousFrame = SDL_GetTicks();

    // reset all event handlers
    eventBus->Reset();

    // perform subscription of events for all systems
    registry->GetSystem<MovementSystem>().SubscribeToEvents(eventBus);
    registry->GetSystem<DamageSystem>().SubscribeToEvents(eventBus);
    registry->GetSystem<KeyboardControlSystem>().SubscribeToEvents(eventBus);
    registry->GetSystem<ProjectileEmitSystem>().SubscribeToEvents(eventBus);

    // update registry to process entities that are waiting to be created / deleted
    registry->Update();

    // ask all the systems to update
    registry->GetSystem<MovementSystem>().Update(deltaTime);
    registry->GetSystem<AnimationSystem>().Update();
    registry->GetSystem<CollisionSystem>().Update(eventBus);
    registry->GetSystem<ProjectileEmitSystem>().Update(registry);
    registry->GetSystem<CameraMovementSystem>().Update(camera);
    registry->GetSystem<ProjectileLifecycleSystem>().Update();
    registry->GetSystem<ScriptSystem>().Update(deltaTime, SDL_GetTicks());
}

void Game::Render() {

    SDL_SetRenderDrawColor(renderer, 21, 21, 21, 255);
    SDL_RenderClear(renderer);

    // invoke all the systems that need to render
    registry->GetSystem<RenderSystem>().Update(renderer, assetStore, camera);
    registry->GetSystem<RenderTextSystem>().Update(renderer, assetStore, camera);
    registry->GetSystem<RenderHealthBarSystem>().Update(renderer, assetStore, camera);

    if (isDebug) {
        registry->GetSystem<RenderColliderSystem>().Update(renderer, camera);
        registry->GetSystem<RenderGUISystem>().Update(registry, camera);
    }


    SDL_RenderPresent(renderer);


}

void Game::Destroy() {
    ImGuiSDL::Deinitialize();
    ImGui::DestroyContext();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}