#pragma once
#include <SDL2/SDL.h>
#include "../core/world.h"
#include "../renderer/landing_renderer.h"
#include <SDL2/SDL_mixer.h>

namespace InputHandler {
    bool handle(SDL_Event& event, World& world, LandingState& landingState, Mix_Music* music);
}