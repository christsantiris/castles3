#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "../core/world.h"

enum class Difficulty { Easy, Hard };

struct LandingState {
    int dynastyIndex = 0;
    bool musicOn = true;
    bool randomResources = false;
    bool interactiveBattles = true;
    Difficulty difficulty = Difficulty::Easy;
    std::string selectedDynasty() const {
        const char* dynasties[] = {
            "Kantakouzenos", "Doukas", "Palaiologos", "Phokas", "Komnenos"
        };
        return dynasties[dynastyIndex];
    }
};

namespace LandingRenderer {
    void render(SDL_Renderer* renderer, TTF_Font* font, const LandingState& state);
}