#ifndef CAMERAMOVEMENTSYSTEM_H
#define CAMERAMOVEMENTSYSTEM_H

#include "../ECS/ECS.h"
#include "../Components/CameraFollowComponent.h"
#include "../Components/TransformComponent.h"
#include <SDL2/SDL.h>

class CameraMovementSystem: public System {
    public:
        CameraMovementSystem() {
            RequireComponent<CameraFollowComponent>();
            RequireComponent<TransformComponent>();
        }

        void Update(SDL_Rect& camera) {
            for (auto entity: GetSystemEntities()) {
                auto transform = entity.GetComponent<TransformComponent>();

                if (transform.position.x < Game::mapWidth) {
                    camera.x = transform.position.x - Game::windowWidth / 2;
                }

                if (transform.position.y < Game::mapHeight) {
                    camera.y = transform.position.y - Game::windowHeight / 2;
                }

                // keep camera rectangle view inside the screen limits
                camera.x = camera.x < 0 ? 0 : camera.x;
                camera.y = camera.y < 0 ? 0 : camera.y;
                camera.x = camera.x > camera.w ? camera.w : camera.x;
                camera.y = camera.y > camera.h ? camera.h : camera.y;

                Logger::Log("Camera changed its position to " + std::to_string(camera.x) + " " + std::to_string(camera.y));
            }
        }
};

#endif