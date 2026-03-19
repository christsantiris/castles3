#include "ui.h"

void renderTopBar(SDL_Renderer* renderer, TTF_Font* font, Game& game) {
    SDL_SetRenderDrawColor(renderer, 60, 60, 60, 255);
    SDL_Rect topBar = {5, 5, 1014, 70};
    SDL_RenderFillRect(renderer, &topBar);

    int iconX[] = {5, 65, 135, 195, 265, 325, 395, 455, 515, 585, 645, 705, 765};

    for (int i = 0; i < 10; i++) {
        if (i == 0) {
            topBarIcons[i].used  = game.availableWorkers;
            topBarIcons[i].total = game.totalWorkers;
        }
        if (i == 1) {
            topBarIcons[i].used  = game.availableMilitary;
            topBarIcons[i].total = game.totalMilitary;
        }
        if (i >= 6 && i <= 9) {
            topBarIcons[i].total = game.resources[i - 6];
        }
        if (topBarIcons[i].type == TASK) {
            SDL_SetRenderDrawColor(renderer, topBarIcons[i].bgColor.r, topBarIcons[i].bgColor.g, topBarIcons[i].bgColor.b, 255);
            SDL_Rect iconBg = {iconX[i], 5, 35, 65};
            SDL_RenderFillRect(renderer, &iconBg);

            SDL_SetRenderDrawColor(renderer, topBarIcons[i].bgColor.r * 0.6, topBarIcons[i].bgColor.g * 0.6, topBarIcons[i].bgColor.b * 0.6, 255);
            SDL_Rect numberBg = {iconX[i] + 35, 5, 20, 65};
            SDL_RenderFillRect(renderer, &numberBg);

            drawShape(renderer, topBarIcons[i].shape, iconX[i], 10);

            SDL_Surface* us = TTF_RenderText_Solid(font, std::to_string(topBarIcons[i].used).c_str(), yellow);
            SDL_Texture* ut = SDL_CreateTextureFromSurface(renderer, us);
            SDL_Rect ur = {iconX[i] + 37, 10, us->w, us->h};
            SDL_RenderCopy(renderer, ut, NULL, &ur);
            SDL_FreeSurface(us);
            SDL_DestroyTexture(ut);

            SDL_Surface* ts = TTF_RenderText_Solid(font, std::to_string(topBarIcons[i].total).c_str(), yellow);
            SDL_Texture* tt = SDL_CreateTextureFromSurface(renderer, ts);
            SDL_Rect tr = {iconX[i] + 37, 40, ts->w, ts->h};
            SDL_RenderCopy(renderer, tt, NULL, &tr);
            SDL_FreeSurface(ts);
            SDL_DestroyTexture(tt);
        } else {
            SDL_SetRenderDrawColor(renderer, topBarIcons[i].bgColor.r, topBarIcons[i].bgColor.g, topBarIcons[i].bgColor.b, 255);
            SDL_Rect iconBg = {iconX[i], 5, 55, 65};
            SDL_RenderFillRect(renderer, &iconBg);

            drawShape(renderer, topBarIcons[i].shape, iconX[i] + 10, 15);

            SDL_Surface* ts = TTF_RenderText_Solid(font, std::to_string(topBarIcons[i].total).c_str(), yellow);
            SDL_Texture* tt = SDL_CreateTextureFromSurface(renderer, ts);
            SDL_Rect tr = {iconX[i] + 35, 40, ts->w, ts->h};
            SDL_RenderCopy(renderer, tt, NULL, &tr);
            SDL_FreeSurface(ts);
            SDL_DestroyTexture(tt);
        }
    }
}