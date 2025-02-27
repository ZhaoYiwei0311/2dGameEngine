#ifndef RENDERSYSYEM_H
#define RENDERSYSYEM_H

#include "../ECS/ECS.h"
#include "../Components/TransformComponent.h"
#include "../Components/SpriteComponent.h"
#include "../AssetStore/AssetStore.h"
#include <SDL2/SDL.h>
#include <algorithm>

class RenderSystem: public System {
    public:
        RenderSystem() {
            RequireComponent<TransformComponent>();
            RequireComponent<SpriteComponent>();
        }

        void Update(SDL_Renderer* renderer, std::unique_ptr<AssetStore>& assetStore, SDL_Rect& camera) {
            // create a vector with both sprite and transform of all entities
            struct RenderableEntity {
                TransformComponent transformComponent;
                SpriteComponent spriteComponent;
            };
            std::vector<RenderableEntity> renderableEntities;
            for (auto entity: GetSystemEntities()) {
                RenderableEntity renderableEntity;
                renderableEntity.spriteComponent = entity.GetComponent<SpriteComponent>();
                renderableEntity.transformComponent = entity.GetComponent<TransformComponent>();

                // bypass rendering entities if they are outside the camera view
                bool isEntityOutsideCameraView = (
                    renderableEntity.transformComponent.position.x + renderableEntity.transformComponent.scale.x * renderableEntity.spriteComponent.width < camera.x ||
                    renderableEntity.transformComponent.position.x > camera.x + camera.w ||
                    renderableEntity.transformComponent.position.y + renderableEntity.transformComponent.scale.y * renderableEntity.spriteComponent.height < camera.y || 
                    renderableEntity.transformComponent.position.y > camera.y + camera.h
                );
                if (isEntityOutsideCameraView && !renderableEntity.spriteComponent.isFixed) {
                    continue;
                }
                renderableEntities.emplace_back(renderableEntity);
            }

            // sort
            std::sort(renderableEntities.begin(), renderableEntities.end(), [](const RenderableEntity& a, const RenderableEntity& b) {
                return a.spriteComponent.zIndex < b.spriteComponent.zIndex;
            });

            // loop all entities that the system is interested in
            for (auto entity : renderableEntities) {
                // update entity position based on its velocity
                const auto transform = entity.transformComponent;
                const auto sprite = entity.spriteComponent;

                // set the source rectangle of original sprite texture
                SDL_Rect srcRect = sprite.srcRect;

                // set the destination rectangle with the x, y position to be rendered
                SDL_Rect dstRect = {
                    static_cast<int>(transform.position.x - (sprite.isFixed ? 0 : camera.x)),
                    static_cast<int>(transform.position.y - (sprite.isFixed ? 0 : camera.y)),
                    static_cast<int>(sprite.width * transform.scale.x),
                    static_cast<int>(sprite.height * transform.scale.y)
                };

                SDL_RenderCopyEx(
                    renderer,
                    assetStore->GetTexture(sprite.assetId),
                    &srcRect,
                    &dstRect,
                    transform.rotation,
                    NULL,
                    sprite.flip
                );
            }

        }
};

#endif