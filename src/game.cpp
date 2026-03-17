#include "game.h"

GameScreen screen = LANDING;
std::string playerDynasty = "";

void Game::init() {
    map.load("data/map.json");
}

void Game::handleEvent(SDL_Event& event) {
    if (event.type == SDL_MOUSEBUTTONDOWN) {
        int x = event.button.x;
        int y = event.button.y;

        if (screen == LANDING) {
            // New Game button
            if (x >= 412 && x <= 612 && y >= 350 && y <= 400) {
                screen = DYNASTY_SELECT;
            }
            // Load Game
            if (x >= 412 && x <= 612 && y >= 430 && y <= 480) {
                
            }
        } else if (screen == DYNASTY_SELECT) {
            for (int i = 0; i < 5; i++) {
                if (x >= 212 && x <= 812 && y >= 200 + (i * 90) && y <= 260 + (i * 90)) {
                    const char* dynasties[] = {
                        "Kantakouzenos",
                        "Doukas",
                        "Palaiologos",
                        "Phokas",
                        "Komnenos"
                    };
                    playerDynasty = dynasties[i];
                    screen = PLAYING;
                }
            }
        } else if (screen == PLAYING) {
            if (y >= 424 && y <= 464) {
                if (x >= 740 && x < 811) activeTab = 0;
                else if (x >= 811 && x < 882) activeTab = 1;
                else if (x >= 882 && x < 953) activeTab = 2;
                else if (x >= 953 && x < 1024) activeTab = 3;
            }
            if (x >= 6 && x <= 728 && y >= 86 && y <= 761) {
                map.handleClick(x, y);
            }
        }
    }
}

void Game::update() {
    
}

void Game::render(SDL_Renderer* renderer, TTF_Font* font) {
    if (screen == LANDING) {
        renderLanding(renderer, font);
    } else if (screen == DYNASTY_SELECT) {
        renderDynastySelect(renderer, font);
    } else if (screen == PLAYING) {
        renderUI(renderer, font, activeTab);
        map.render(renderer, font);

        // Show province info if one is selected
        for (auto& p : map.provinces) {
            if (p.isSelected) {
                renderProvinceInfo(renderer, font, p, playerDynasty);
                break;
            }
        }
    }
}