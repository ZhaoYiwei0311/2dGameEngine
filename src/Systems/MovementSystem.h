#ifndef MOVEMENTSYSYEM_H
#define MOVEMENTSYSYEM_H

#include "../ECS/ECS.h"
#include "../EventBus/EventBus.h"
#include "../Events/CollisionEvent.h"
#include "../Components/TransformComponent.h"
#include "../Components/RigidBodyComponent.h"
#include "../Components/SpriteComponent.h"

class MovementSystem: public System {
    public:
        MovementSystem() {
            RequireComponent<TransformComponent>();
            RequireComponent<RigidBodyComponent>();

        }

        void SubscribeToEvents(std::unique_ptr<EventBus>& eventBus) {
            eventBus->SubscribeToEvent<CollisionEvent>(this, &MovementSystem::OnCollision);
        }

        void Update(double deltaTime) {
            // loop all entities that the system is interested in
            for (auto entity : GetSystemEntities()) {
                // update entity position based on its velocity
                auto& transform = entity.GetComponent<TransformComponent>();
                const auto rigidBody = entity.GetComponent<RigidBodyComponent>();

                transform.position.x += rigidBody.velocity.x * deltaTime;
                transform.position.y += rigidBody.velocity.y * deltaTime;

                if (entity.HasTag("player")) {
                    int paddingLeft = 10;
                    int paddingTop = 10;
                    int paddingRight = 50;
                    int paddingBottom = 50;
                    transform.position.x = transform.position.x < paddingLeft ? paddingLeft : transform.position.x;
                    transform.position.x = transform.position.x > Game::mapWidth - paddingRight ? Game::mapWidth - paddingRight : transform.position.x;
                    transform.position.y = transform.position.y < paddingTop ? paddingTop : transform.position.y;
                    transform.position.y = transform.position.y > Game::mapHeight - paddingBottom ? Game::mapHeight - paddingBottom : transform.position.y;
                }

                int margin = 100;
                bool isEntityOutsideMap = (
                    transform.position.x < -margin ||
                    transform.position.x > Game::mapWidth + margin || 
                    transform.position.y < -margin ||
                    transform.position.y > Game::mapHeight + margin
                );

                if (isEntityOutsideMap && !entity.HasTag("player")) {
                    entity.Kill();
                }

                // Logger::Log(
                //     "Entity id = " +
                //     std::to_string(entity.GetId()) +
                //     " position is now (" +
                //     std::to_string(transform.position.x) +
                //     ", " +
                //     std::to_string(transform.position.y) +
                //     ")"
                // );
            }

        }

        void OnCollision(CollisionEvent& event) {
            Entity a = event.a;
            Entity b = event.b;

            if (a.BelongsToGroup("obstacles") && b.BelongsToGroup("enemies")) {
                OnEnemyHitsObstacle(b, a);
            }

            if (b.BelongsToGroup("obstacles") && a.BelongsToGroup("enemies")) {
                OnEnemyHitsObstacle(a, b);
            }
        }

        void OnEnemyHitsObstacle(Entity enemy, Entity obstacle) {
            if (enemy.HasComponent<RigidBodyComponent>() && enemy.HasComponent<SpriteComponent>()) {
                auto& rigidBody = enemy.GetComponent<RigidBodyComponent>();
                auto& sprite = enemy.GetComponent<SpriteComponent>();

                if (rigidBody.velocity.x != 0) {
                    rigidBody.velocity.x *= -1;
                    sprite.flip = (sprite.flip == SDL_FLIP_NONE) ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
                }

                if (rigidBody.velocity.y != 0) {
                    rigidBody.velocity.y *= -1;
                    sprite.flip = (sprite.flip == SDL_FLIP_NONE) ? SDL_FLIP_VERTICAL : SDL_FLIP_NONE;
                }
            }
        }
};

#endif