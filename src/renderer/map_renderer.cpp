#include "map_renderer.h"

namespace MapRenderer {

    // Parchment background color
    static const SDL_Color WATER       = {180, 210, 230, 255};
    static const SDL_Color BORDER      = {100,  80,  40, 255};
    static const SDL_Color LABEL       = {255, 215,   0, 255};
    static const SDL_Color LABEL_SEL   = {255, 255, 255, 255};

    static SDL_Color ownerColor(const std::string& owner) {
        if (owner == "Baldwin II")    return {128,   0, 128, 255};
        if (owner == "Kantakouzenos") return {180,  30,  30, 255};
        if (owner == "Doukas")        return {180,   0, 180, 255};
        if (owner == "Palaiologos")   return {200, 100,   0, 255};
        if (owner == "Phokas")        return {139,  69,  19, 255};
        if (owner == "Komnenos")      return {  0, 139, 139, 255};
        if (owner == "neutral")       return { 34, 139,  34, 255};
        // Player dynasty — bright green
        return {50, 205, 50, 255};
    }

    static void fillPolygon(SDL_Renderer* renderer, const std::vector<Point>& polygon, SDL_Color color) {
        if (polygon.size() < 3) return;

        // Find bounding box
        int minY = polygon[0].y, maxY = polygon[0].y;
        for (auto& pt : polygon) {
            minY = std::min(minY, pt.y);
            maxY = std::max(maxY, pt.y);
        }

        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);

        // Scanline fill
        for (int y = minY; y <= maxY; y++) {
            std::vector<int> intersections;
            int n = polygon.size();
            for (int i = 0, j = n - 1; i < n; j = i++) {
                int yi = polygon[i].y, yj = polygon[j].y;
                int xi = polygon[i].x, xj = polygon[j].x;
                if ((yi <= y && yj > y) || (yj <= y && yi > y)) {
                    int x = xi + (y - yi) * (xj - xi) / (yj - yi);
                    intersections.push_back(x);
                }
            }
            std::sort(intersections.begin(), intersections.end());
            for (int i = 0; i + 1 < (int)intersections.size(); i += 2)
                SDL_RenderDrawLine(renderer, intersections[i], y, intersections[i+1], y);
        }
    }

    static void drawBorder(SDL_Renderer* renderer, const std::vector<Point>& polygon) {
        int n = polygon.size();
        SDL_SetRenderDrawColor(renderer, BORDER.r, BORDER.g, BORDER.b, BORDER.a);
        for (int i = 0; i < n; i++) {
            int j = (i + 1) % n;
            SDL_RenderDrawLine(renderer,
                polygon[i].x, polygon[i].y,
                polygon[j].x, polygon[j].y);
        }
    }

    static void drawLabel(SDL_Renderer* renderer, TTF_Font* font,
                          const std::string& text, const std::vector<Point>& polygon,
                          bool selected) {
        // Centroid
        int cx = 0, cy = 0;
        for (auto& pt : polygon) { cx += pt.x; cy += pt.y; }
        cx /= polygon.size();
        cy /= polygon.size();

        SDL_Color color = selected ? LABEL_SEL : LABEL;
        SDL_Surface* s = TTF_RenderText_Solid(font, text.c_str(), color);
        if (!s) return;
        SDL_Texture* t = SDL_CreateTextureFromSurface(renderer, s);
        SDL_Rect r = {cx - s->w / 2, cy - s->h / 2, s->w, s->h};
        SDL_RenderCopy(renderer, t, nullptr, &r);
        SDL_FreeSurface(s);
        SDL_DestroyTexture(t);
    }

    void render(SDL_Renderer* renderer, TTF_Font* font, const World& world) {
        // Draw water background
        SDL_SetRenderDrawColor(renderer, WATER.r, WATER.g, WATER.b, WATER.a);
        SDL_Rect mapArea = {0, 0, 950, 900};
        SDL_RenderFillRect(renderer, &mapArea);

        // Fill provinces
        for (auto& p : world.provinces) {
            SDL_Color color = p.isSelected
                ? SDL_Color{144, 238, 144, 255}
                : SDL_Color{34, 139, 34, 255};  // neutral green for all unselected
            fillPolygon(renderer, p.polygon, color);
        }

        // Draw borders
        for (auto& p : world.provinces)
            drawBorder(renderer, p.polygon);

        // Draw labels
        for (auto& p : world.provinces)
            drawLabel(renderer, font, p.name, p.polygon, p.isSelected);
    }

}