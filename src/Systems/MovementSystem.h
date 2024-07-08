#ifndef MOVEMENTSYSYEM_H
#define MOVEMENTSYSYEM_H

#include "../ECS/ECS.h"
#include "../Components/TransformComponent.h"
#include "../Components/RigidBodyComponent.h"

class MovementSystem: public System {
    public:
        MovementSystem() {
            RequireComponent<TransformComponent>();
            RequireComponent<RigidBodyComponent>();

        }

        void Update(double deltaTime) {
            // loop all entities that the system is interested in
            for (auto entity : GetSystemEntities()) {
                // update entity position based on its velocity
                auto& transform = entity.GetComponent<TransformComponent>();
                const auto rigidBody = entity.GetComponent<RigidBodyComponent>();

                transform.position.x += rigidBody.velocity.x * deltaTime;
                transform.position.y += rigidBody.velocity.y * deltaTime;

                Logger::Log(
                    "Entity id = " +
                    std::to_string(entity.GetId()) +
                    " position is now (" +
                    std::to_string(transform.position.x) +
                    ", " +
                    std::to_string(transform.position.y) +
                    ")"
                );
            }

        }
};

#endif