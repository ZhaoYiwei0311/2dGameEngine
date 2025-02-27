#ifndef SPRITECOMPONENT_H
#define SPRITECOMPONENT_H

#include <string>
#include <SDL2/SDL.h>

struct SpriteComponent {
    std::string assetId;
    int width;
    int height;
    int zIndex;
    bool isFixed;
    SDL_RendererFlip flip;
    SDL_Rect srcRect;
    
    // a default constructo is needed
    SpriteComponent(std::string assetId = "", int width = 0, int height = 0, int zIndex = 0, bool isFixed = false, int srcRectX = 0, int srcRectY = 0) {
        this->assetId = assetId;
        this->width = width;
        this->height = height;
        this->flip = SDL_FLIP_NONE;
        this->isFixed = isFixed;
        this->zIndex = zIndex;
        this->srcRect = {srcRectX, srcRectY, width, height};
    }
};


#endif