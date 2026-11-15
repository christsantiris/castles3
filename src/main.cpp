#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include "core/world.h"
#include "core/systems/map_system.h"
#include "core/systems/game_system.h"
#include "renderer/map_renderer.h"
#include "renderer/panel_renderer.h"
#include "renderer/topbar_renderer.h"
#include "renderer/landing_renderer.h"
#include "input/input_handler.h"
#include "renderer/battle_renderer.h"
#include "core/systems/combat_system.h"
#include "renderer/victory_renderer.h"
#include "renderer/defeat_renderer.h"
#include "renderer/upkeep_renderer.h"
#include "core/systems/hall_of_fame_system.h"
#include "core/systems/date_system.h"
#include "core/systems/ai_system.h"
#include "core/systems/save_load.h"
#include "core/storage_paths.h"
#ifdef __EMSCRIPTEN__
#include <emscripten.h>

EM_ASYNC_JS(void, browser_next_frame, (), {
    await new Promise(function(resolve) {
        requestAnimationFrame(resolve);
    });
});
#endif

int main() {
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();

    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);
    Mix_Music* music = Mix_LoadMUS("music/theme/medieval.ogg");
    if (music)
        Mix_PlayMusic(music, -1);

    SDL_Window* window = SDL_CreateWindow(
        "Castles III: Siege & Conquest",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        1280, 800,
        SDL_WINDOW_SHOWN
    );
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    TTF_Font* font = TTF_OpenFont("fonts/MedievalSharp-Regular.ttf", 14);
    if (!font) {
        SDL_Log("Failed to load font: %s", TTF_GetError());
        return 1;
    }

    World world;
    MapSystem::load(world, "data/map.json");
    GameSystem::initArmies(world);
    // AISystem::initAI(world, world.aiConfig);

    LandingState landingState;
    HallOfFame hof = HallOfFameSystem::load(Storage::path("hall_of_fame.json"));
    Uint32 lastTick = SDL_GetTicks();
    Uint32 lastFrame = lastTick;
    const Uint32 tickInterval = 3750;

    SDL_Event event;
    bool running = true;
    while (running) {
#ifdef __EMSCRIPTEN__
        browser_next_frame();
#endif
        while (SDL_PollEvent(&event)) {
            if (!InputHandler::handle(event, world, landingState, music))
                running = false;
        }
        if (world.ctx.shouldQuit) running = false;

        Uint32 now = SDL_GetTicks();
        float deltaTime = (now - lastFrame) / 1000.0f;
        lastFrame = now;

        if (world.ctx.screen == GameScreen::Playing ||
            world.ctx.screen == GameScreen::Victory) {
            CombatSystem::updateBattle(world, deltaTime);

            if (world.ctx.paused || world.battle.phase != BattlePhase::None) {
                // Do not accumulate campaign time while fighting or reviewing
                // results. Battle animation uses the separate frame clock.
                lastTick = now;
            } else if (now - lastTick >= tickInterval) {
                lastTick = now;
                if (world.ctx.screen == GameScreen::Playing &&
                    world.battle.phase == BattlePhase::None &&
                    !world.ctx.paused) {
                    GameSystem::tick(world);
                }

            }
        }

        if (world.ctx.screen == GameScreen::Victory && !world.ctx.victoryRecorded) {
            world.ctx.victoryRecorded = true;
            HallOfFameSystem::addEntry(hof, world.ctx.playerDynasty,
                                       world.ctx.score, DateSystem::toString(world.date));
            HallOfFameSystem::save(hof, Storage::path("hall_of_fame.json"));
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        if (world.ctx.screen == GameScreen::Landing) {
            LandingRenderer::render(renderer, font, landingState);
        } else if (world.ctx.screen == GameScreen::Playing) {
            MapRenderer::render(renderer, font, world);
            PanelRenderer::render(renderer, font, world, landingState.musicOn);
            TopBarRenderer::render(renderer, font, world);
            BattleRenderer::render(renderer, font, world);
            if (world.ctx.upkeepDue) {
                UpkeepRenderer::render(renderer, font, world);
            }
        } else if (world.ctx.screen == GameScreen::Victory) {
            MapRenderer::render(renderer, font, world);
            PanelRenderer::render(renderer, font, world, landingState.musicOn);
            TopBarRenderer::render(renderer, font, world);
            VictoryRenderer::render(renderer, font, world, hof);
        } else if (world.ctx.screen == GameScreen::Defeat) {
            MapRenderer::render(renderer, font, world);
            PanelRenderer::render(renderer, font, world, landingState.musicOn);
            TopBarRenderer::render(renderer, font, world);
            DefeatRenderer::render(renderer, font, world);
        }

        SDL_RenderPresent(renderer);
#ifndef __EMSCRIPTEN__
        SDL_Delay(16);
#endif
    }

    if (world.ctx.screen == GameScreen::Playing) {
        SaveLoad::save_game(world, Storage::path("savegame.json"));
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_CloseFont(font);
    Mix_FreeMusic(music);
    Mix_CloseAudio();
    TTF_Quit();
    SDL_Quit();
#ifdef __EMSCRIPTEN__
    EM_ASM({ Module.onGameExit(); });
#endif
    return 0;
}
