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

        if (y >= 424 && y <= 464) {
            if (x >= 740 && x < 811) activeTab = 0;
            else if (x >= 811 && x < 882) activeTab = 1;
            else if (x >= 882 && x < 953) activeTab = 2;
            else if (x >= 953 && x < 1024) activeTab = 3;
        }

        // Only handle map clicks within the map area
        if (x >= 6 && x <= 728 && y >= 86 && y <= 761) {
            map.handleClick(x, y);
        }
    }
}

void Game::update() {
    
}

void Game::render(SDL_Renderer* renderer, TTF_Font* font) {
    if (screen == LANDING) {
        renderLanding(renderer, font);
    } else if (screen == DYNASTY_SELECT) {
        // coming next
    } else if (screen == PLAYING) {
        renderUI(renderer, font, activeTab);
        map.render(renderer, font);
    }
}