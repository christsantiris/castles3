#include "ui.h"

void renderCombatRow(SDL_Renderer* renderer, TTF_Font* font, Game& game) {
    SDL_SetRenderDrawColor(renderer, game.combat.active ? 120 : 80, game.combat.active ? 0 : 45, 10, 255);
    SDL_Rect combatRow = {745, 184, 274, 45};
    SDL_RenderFillRect(renderer, &combatRow);

    SDL_SetRenderDrawColor(renderer, 30, 15, 5, 255);
    SDL_Rect combatProgressBg = {745, 212, 274, 8};
    SDL_RenderFillRect(renderer, &combatProgressBg);

    if (game.combat.active) {
        std::string label = "Attack: " + game.map.provinces[game.combat.targetProvince].name;
        SDL_Surface* ls = TTF_RenderText_Solid(font, label.c_str(), yellow);
        SDL_Texture* lt = SDL_CreateTextureFromSurface(renderer, ls);
        SDL_Rect lr = {750, 191, ls->w, ls->h};
        SDL_RenderCopy(renderer, lt, NULL, &lr);
        SDL_FreeSurface(ls);
        SDL_DestroyTexture(lt);

        SDL_SetRenderDrawColor(renderer, 220, 0, 0, 255);
        SDL_Rect progressFill = {745, 212, (int)(274 * game.combat.progress()), 8};
        SDL_RenderFillRect(renderer, &progressFill);

        SDL_SetRenderDrawColor(renderer, 120, 0, 0, 255);
        SDL_Rect cancelBtn = {745, 194, 60, 22};
        SDL_RenderFillRect(renderer, &cancelBtn);
        SDL_SetRenderDrawColor(renderer, 255, 215, 0, 255);
        SDL_RenderDrawRect(renderer, &cancelBtn);
        SDL_Surface* cs = TTF_RenderText_Solid(font, "Cancel", yellow);
        SDL_Texture* ct = SDL_CreateTextureFromSurface(renderer, cs);
        SDL_Rect cr = {748, 196, cs->w, cs->h};
        SDL_RenderCopy(renderer, ct, NULL, &cr);
        SDL_FreeSurface(cs);
        SDL_DestroyTexture(ct);
    }
}