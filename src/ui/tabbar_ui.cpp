#include "ui.h"

void renderTabBar(SDL_Renderer* renderer, TTF_Font* font, int activeTab) {
    SDL_SetRenderDrawColor(renderer, 0, activeTab == 0 ? 200 : 120, 0, 255);
    SDL_Rect stockTab = {740, 424, 71, 40};
    SDL_RenderFillRect(renderer, &stockTab);

    SDL_SetRenderDrawColor(renderer, activeTab == 1 ? 220 : 150, 0, 0, 255);
    SDL_Rect armyTab = {811, 424, 71, 40};
    SDL_RenderFillRect(renderer, &armyTab);

    SDL_SetRenderDrawColor(renderer, 0, 0, activeTab == 2 ? 220 : 150, 255);
    SDL_Rect relatTab = {882, 424, 71, 40};
    SDL_RenderFillRect(renderer, &relatTab);

    SDL_SetRenderDrawColor(renderer, activeTab == 3 ? 160 : 101, activeTab == 3 ? 100 : 67, activeTab == 3 ? 50 : 33, 255);
    SDL_Rect optsTab = {953, 424, 71, 40};
    SDL_RenderFillRect(renderer, &optsTab);

    const char* tabLabels[] = {"STOCK", "ARMY", "RELAT", "OPTS"};
    int tabX[] = {750, 820, 890, 960};
    for (int i = 0; i < 4; i++) {
        SDL_Surface* s = TTF_RenderText_Solid(font, tabLabels[i], yellow);
        SDL_Texture* t = SDL_CreateTextureFromSurface(renderer, s);
        SDL_Rect r = {tabX[i], 430, s->w, s->h};
        SDL_RenderCopy(renderer, t, NULL, &r);
        SDL_FreeSurface(s);
        SDL_DestroyTexture(t);
    }
}