#ifndef GAME_H
#define GAME_H

#include "../ECS/ECS.h"
#include <SDL2/SDL.h>
#include <memory>

const int FPS = 2;
const int MILLISECS_PER_FRAME = 1000 / FPS;
class Game {
    private:
        bool isRunning;
        int millisecPreviousFrame = 0;
        SDL_Window* window;
        SDL_Renderer* renderer;

        std::unique_ptr<Registry> registry;

    public:
        Game();
        ~Game();

        void Setup();
        void Initialize();
        void Run();
        void ProcessInput();
        void Update();
        void Render();
        void Destroy();

        int windowWidth;
        int windowHeight;
};

#endif