#include "ui.h"

void renderTaskRows(SDL_Renderer* renderer, TTF_Font* font, Game& game) {
    // Task slot row 1
    SDL_SetRenderDrawColor(renderer, 80, 45, 10, 255);
    SDL_Rect taskRow = {745, 90, 274, 45};
    SDL_RenderFillRect(renderer, &taskRow);

    SDL_SetRenderDrawColor(renderer, 30, 15, 5, 255);
    SDL_Rect progressBg = {745, 118, 274, 8};
    SDL_RenderFillRect(renderer, &progressBg);

    if (game.task.active) {
        const char* resNames[] = {"Food", "Timber", "Iron", "Gold"};

        SDL_Surface* ls = TTF_RenderText_Solid(font, resNames[game.task.res], yellow);
        SDL_Texture* lt = SDL_CreateTextureFromSurface(renderer, ls);
        SDL_Rect lr = {820, 97, ls->w, ls->h};
        SDL_RenderCopy(renderer, lt, NULL, &lr);
        SDL_FreeSurface(ls);
        SDL_DestroyTexture(lt);

        SDL_SetRenderDrawColor(renderer, 0, 200, 0, 255);
        SDL_Rect progressFill = {745, 118, (int)(274 * game.task.progress()), 8};
        SDL_RenderFillRect(renderer, &progressFill);
    }

    // Task slot row 2
    if (game.hasSecondStockSlot()) {
        SDL_SetRenderDrawColor(renderer, 80, 45, 10, 255);
        SDL_Rect taskRow2 = {745, 137, 274, 45};
        SDL_RenderFillRect(renderer, &taskRow2);

        SDL_SetRenderDrawColor(renderer, 30, 15, 5, 255);
        SDL_Rect progressBg2 = {745, 165, 274, 8};
        SDL_RenderFillRect(renderer, &progressBg2);

        if (game.task2.active) {
            const char* resNames[] = {"Food", "Timber", "Iron", "Gold"};

            SDL_Surface* ls = TTF_RenderText_Solid(font, resNames[game.task2.res], yellow);
            SDL_Texture* lt = SDL_CreateTextureFromSurface(renderer, ls);
            SDL_Rect lr = {820, 144, ls->w, ls->h};
            SDL_RenderCopy(renderer, lt, NULL, &lr);
            SDL_FreeSurface(ls);
            SDL_DestroyTexture(lt);

            SDL_SetRenderDrawColor(renderer, 0, 200, 0, 255);
            SDL_Rect progressFill2 = {745, 165, (int)(274 * game.task2.progress()), 8};
            SDL_RenderFillRect(renderer, &progressFill2);
        }
    }
}