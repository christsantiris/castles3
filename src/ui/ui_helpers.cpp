#include "ui.h"

SDL_Color green  = {0, 200, 0, 255};
SDL_Color red    = {220, 0, 0, 255};
SDL_Color blue   = {0, 0, 220, 255};
SDL_Color yellow = {255, 215, 0, 255};
SDL_Color white  = {255, 255, 255, 255};

TopBarIcon topBarIcons[] = {
    {{0, 120, 0, 255},   4,  4, TASK,     CASTLE},   // stock
    {{150, 0, 0, 255},   4,  4, TASK,     SWORD},    // army
    {{0, 0, 150, 255},   4,  4, TASK,     SCROLL},   // relations
    {{150, 0, 0, 255},   0,  4, UNIT,     SWORD},    // infantry
    {{150, 0, 0, 255},   0,  4, UNIT,     ARROW},    // archers
    {{150, 0, 0, 255},   0,  4, UNIT,     SHIELD},   // knights
    {{0, 120, 0, 255},   0,  0, RESOURCE, FOOD},
    {{0, 120, 0, 255},   0,  0, RESOURCE, TIMBER},
    {{0, 120, 0, 255},   0,  0, RESOURCE, IRON},
    {{0, 120, 0, 255},   0,  0, RESOURCE, GOLD},
};

void drawShape(SDL_Renderer* renderer, IconShape shape, int x, int y) {
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    switch (shape) {
        case CASTLE:
            SDL_RenderDrawRect(renderer, new SDL_Rect{x, y+5, 20, 20});
            SDL_RenderDrawRect(renderer, new SDL_Rect{x+3, y, 5, 7});
            SDL_RenderDrawRect(renderer, new SDL_Rect{x+12, y, 5, 7});
            break;
        case SWORD:
            SDL_RenderDrawLine(renderer, x+10, y, x+10, y+20);
            SDL_RenderDrawLine(renderer, x+5, y+8, x+15, y+8);
            break;
        case SHIELD:
            SDL_RenderDrawRect(renderer, new SDL_Rect{x+3, y, 14, 18});
            break;
        case HELMET:
            SDL_RenderDrawLine(renderer, x+3, y+10, x+17, y+10);
            SDL_RenderDrawLine(renderer, x+3, y+10, x+5, y+20);
            SDL_RenderDrawLine(renderer, x+17, y+10, x+15, y+20);
            break;
        case SCROLL:
            SDL_RenderDrawRect(renderer, new SDL_Rect{x+3, y+3, 14, 18});
            SDL_RenderDrawLine(renderer, x+3, y+3, x+3, y+21);
            break;
        case ARROW:
            SDL_RenderDrawLine(renderer, x+10, y, x+10, y+20);
            SDL_RenderDrawLine(renderer, x+10, y, x+5, y+8);
            SDL_RenderDrawLine(renderer, x+10, y, x+15, y+8);
            break;
        case CIRCLE:
            SDL_RenderDrawRect(renderer, new SDL_Rect{x+3, y+3, 14, 14});
            break;
        case TRIANGLE:
            SDL_RenderDrawLine(renderer, x+10, y, x+3, y+20);
            SDL_RenderDrawLine(renderer, x+10, y, x+17, y+20);
            SDL_RenderDrawLine(renderer, x+3, y+20, x+17, y+20);
            break;
        case SQUARE:
            SDL_RenderDrawRect(renderer, new SDL_Rect{x+3, y+3, 14, 14});
            break;
        case DIAMOND:
            SDL_RenderDrawLine(renderer, x+10, y, x+17, y+10);
            SDL_RenderDrawLine(renderer, x+17, y+10, x+10, y+20);
            SDL_RenderDrawLine(renderer, x+10, y+20, x+3, y+10);
            SDL_RenderDrawLine(renderer, x+3, y+10, x+10, y);
            break;
        case FOOD:
            SDL_RenderDrawLine(renderer, x+10, y+2, x+10, y+22);
            SDL_RenderDrawLine(renderer, x+2, y+12, x+18, y+12);
            break;
        case TIMBER:
            SDL_RenderDrawLine(renderer, x+10, y+2, x+3, y+14);
            SDL_RenderDrawLine(renderer, x+10, y+2, x+17, y+14);
            SDL_RenderDrawLine(renderer, x+3, y+14, x+17, y+14);
            SDL_RenderDrawLine(renderer, x+9, y+14, x+9, y+22);
            SDL_RenderDrawLine(renderer, x+11, y+14, x+11, y+22);
            break;
        case IRON:
            SDL_RenderDrawRect(renderer, new SDL_Rect{x+3, y+2, 14, 8});
            SDL_RenderDrawRect(renderer, new SDL_Rect{x+6, y+10, 8, 4});
            SDL_RenderDrawRect(renderer, new SDL_Rect{x+4, y+14, 12, 6});
            break;
        case GOLD:
            SDL_RenderDrawRect(renderer, new SDL_Rect{x+4, y+4, 12, 4});
            SDL_RenderDrawRect(renderer, new SDL_Rect{x+4, y+10, 12, 4});
            SDL_RenderDrawRect(renderer, new SDL_Rect{x+4, y+16, 12, 4});
            break;
    }
}