#include "landing_renderer.h"
#include <string>

namespace LandingRenderer {

    static const SDL_Color GOLD = {255, 215, 0, 255};
    static const SDL_Color YELLOW = {255, 255, 0, 255};
    static const SDL_Color WHITE = {255, 255, 255, 255};
    static const SDL_Color DARK = {20, 10, 5, 255};

    static void drawWoodBackground(SDL_Renderer* r) {
        // Base dark brown
        SDL_SetRenderDrawColor(r, 60, 30, 10, 255);
        SDL_Rect bg = {0, 0, 1280, 800};
        SDL_RenderFillRect(r, &bg);

        // Horizontal grain lines
        for (int y = 0; y < 800; y += 6) {
            int shade = (y % 18 == 0) ? 20 : (y % 12 == 0) ? 10 : 5;
            SDL_SetRenderDrawColor(r, 60 + shade, 30 + shade/2, 10, 180);
            SDL_RenderDrawLine(r, 0, y, 1280, y);
        }

        // Darker vertical grain streaks
        for (int x = 0; x < 1280; x += 80) {
            int offset = (x * 7) % 40;
            SDL_SetRenderDrawColor(r, 40, 20, 5, 60);
            SDL_RenderDrawLine(r, x + offset, 0, x + offset - 20, 800);
        }
    }

    static void drawText(SDL_Renderer* r, TTF_Font* font, const std::string& text,
                         int x, int y, SDL_Color color) {
        SDL_Surface* s = TTF_RenderText_Solid(font, text.c_str(), color);
        if (!s) return;
        SDL_Texture* t = SDL_CreateTextureFromSurface(r, s);
        SDL_Rect rect = {x, y, s->w, s->h};
        SDL_RenderCopy(r, t, nullptr, &rect);
        SDL_FreeSurface(s);
        SDL_DestroyTexture(t);
    }

    static void drawTextCentered(SDL_Renderer* r, TTF_Font* font, const std::string& text,
                                  int y, SDL_Color color) {
        SDL_Surface* s = TTF_RenderText_Solid(font, text.c_str(), color);
        if (!s) return;
        SDL_Texture* t = SDL_CreateTextureFromSurface(r, s);
        SDL_Rect rect = {640 - s->w / 2, y, s->w, s->h};
        SDL_RenderCopy(r, t, nullptr, &rect);
        SDL_FreeSurface(s);
        SDL_DestroyTexture(t);
    }

    static void drawMenuItem(SDL_Renderer* r, TTF_Font* font,
                              const std::string& label, const std::string& value,
                              int y) {
        std::string line = "-> " + label + ": " + value;
        drawText(r, font, line, 420, y, GOLD);
    }

    static void drawButton(SDL_Renderer* r, TTF_Font* font,
                           const std::string& label, int x, int y) {
        SDL_Surface* s = TTF_RenderText_Solid(font, label.c_str(), YELLOW);
        if (!s) return;
        SDL_Texture* t = SDL_CreateTextureFromSurface(r, s);
        SDL_Rect rect = {x, y, s->w, s->h};
        SDL_RenderCopy(r, t, nullptr, &rect);
        SDL_FreeSurface(s);
        SDL_DestroyTexture(t);
    }

    void render(SDL_Renderer* renderer, TTF_Font* font, const LandingState& state) {
        drawWoodBackground(renderer);

        // Title
        drawTextCentered(renderer, font, "CASTLES III", 80, GOLD);
        drawTextCentered(renderer, font, "SIEGE & CONQUEST", 130, GOLD);

        // Menu items
        int menuY = 280;
        int lineH = 50;

        drawMenuItem(renderer, font, "Play as", state.selectedDynasty(), menuY);
        menuY += lineH;
        drawMenuItem(renderer, font, "Difficulty", "Easy", menuY);
        menuY += lineH;
        drawMenuItem(renderer, font, "Music", state.musicOn ? "On" : "Off", menuY);
        menuY += lineH;
        drawMenuItem(renderer, font, "Interactive Battles", state.interactiveBattles ? "On" : "Off", menuY);
        menuY += lineH;
        drawMenuItem(renderer, font, "Resources", state.randomResources ? "Random" : "Balanced", menuY);
        menuY += lineH;
        drawMenuItem(renderer, font, "Starting Locations", "Random", menuY);

        // Buttons
        int btnY = menuY + 70;
        drawButton(renderer, font, "PLAY", 460, btnY);
        drawButton(renderer, font, "LOAD", 580, btnY);
        drawButton(renderer, font, "EXIT", 700, btnY);

        // Copyright
        drawTextCentered(renderer, font,
            "Castles III: Siege & Conquest. All rights reserved.",
            750, GOLD);
    }

}