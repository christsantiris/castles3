#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "../core/world.h"

namespace PanelRenderer {
    void render(SDL_Renderer* renderer, TTF_Font* font, World& world, bool musicOn);
    static void renderOptsTab(SDL_Renderer* r, TTF_Font* font, World& world, bool musicOn);
}