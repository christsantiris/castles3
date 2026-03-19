#include "ui.h"

void renderInfoPanel(SDL_Renderer* renderer, TTF_Font* font, const char* dateStr, Game& game) {
    SDL_SetRenderDrawColor(renderer, 60, 30, 10, 255);
    SDL_Rect infoPanel = {745, 464, 274, 299};
    SDL_RenderFillRect(renderer, &infoPanel);

    std::string scoreStr = "Score: " + std::to_string(game.score);
    const char* infoLabels[] = {"Castles III", dateStr, scoreStr.c_str()};
    int infoY[] = {500, 600, 640};
    for (int i = 0; i < 3; i++) {
        SDL_Surface* s = TTF_RenderText_Solid(font, infoLabels[i], yellow);
        SDL_Texture* t = SDL_CreateTextureFromSurface(renderer, s);
        SDL_Rect r = {750, infoY[i], s->w, s->h};
        SDL_RenderCopy(renderer, t, NULL, &r);
        SDL_FreeSurface(s);
        SDL_DestroyTexture(t);
    }
}