#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

int main() {
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();

    SDL_Window* window = SDL_CreateWindow(
        "Castles II: Siege & Conquest",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        1024, 768,
        SDL_WINDOW_SHOWN
    );

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    SDL_Event event;
    bool running = true;
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = false;
        }

        // Top bar - dark gray
        SDL_SetRenderDrawColor(renderer, 60, 60, 60, 255);
        SDL_Rect topBar = {0, 0, 1024, 80};
        SDL_RenderFillRect(renderer, &topBar);

        // Main map area - dark green
        SDL_SetRenderDrawColor(renderer, 34, 85, 34, 255);
        SDL_Rect mapArea = {0, 80, 740, 688};
        SDL_RenderFillRect(renderer, &mapArea);

        // Right sidebar - gray stone color
        SDL_SetRenderDrawColor(renderer, 120, 120, 120, 255);
        SDL_Rect sidebar = {740, 80, 284, 688};
        SDL_RenderFillRect(renderer, &sidebar);

        // Map border - gold color
        SDL_SetRenderDrawColor(renderer, 200, 170, 50, 255);
        SDL_Rect mapBorder = {0, 80, 740, 688};
        SDL_RenderDrawRect(renderer, &mapBorder);

        // Sidebar border - dark gray
        SDL_SetRenderDrawColor(renderer, 40, 40, 40, 255);
        SDL_Rect sidebarBorder = {740, 80, 284, 688};
        SDL_RenderDrawRect(renderer, &sidebarBorder);

        // Tab bar background - dark brown
        SDL_SetRenderDrawColor(renderer, 60, 30, 10, 255);
        SDL_Rect tabBar = {740, 80, 284, 40};
        SDL_RenderFillRect(renderer, &tabBar);

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}