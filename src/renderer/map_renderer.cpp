#include "map_renderer.h"
#include "../core/map_labels.h"
#include <algorithm>
#include <cmath>
#include <functional>

namespace MapRenderer {

    static const int MAP_Y = 60;

    static const SDL_Color WATER     = {180, 210, 230, 255};
    static const SDL_Color BORDER    = {100,  80,  40, 255};
    static const SDL_Color LABEL     = {255, 215,   0, 255};
    static const SDL_Color LABEL_SEL = {255, 255, 255, 255};

    static const float MAP_SCALE_Y = 740.0f / 880.0f;
    static const float MAP_SCALE_X = 950.0f / 940.0f;

    static Point offsetPoint(const Point& point) {
        return {(int)(point.x * MAP_SCALE_X), (int)(point.y * MAP_SCALE_Y) + MAP_Y};
    }

    static std::vector<Point> offsetPolygon(const std::vector<Point>& polygon) {
        std::vector<Point> result;
        result.reserve(polygon.size());
        for (const auto& pt : polygon) {
            result.push_back(offsetPoint(pt));
        }
        return result;
    }

    static void fillPolygon(SDL_Renderer* renderer, const std::vector<Point>& polygon, SDL_Color color) {
        if (polygon.size() < 3) return;

        int minY = polygon[0].y, maxY = polygon[0].y;
        for (auto& pt : polygon) {
            minY = std::min(minY, pt.y);
            maxY = std::max(maxY, pt.y);
        }

        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);

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

    static void drawProvinceBorder(SDL_Renderer* renderer, const std::vector<Point>& polygon, bool owned) {
        auto points = offsetPolygon(polygon);
        if (!owned) {
            drawBorder(renderer, points);
            return;
        }
        SDL_SetRenderDrawColor(renderer, 255, 215, 0, 255);
        for (size_t i = 0; i < points.size(); i++) {
            size_t j = (i + 1) % points.size();
            SDL_RenderDrawLine(renderer, points[i].x, points[i].y, points[j].x, points[j].y);
            SDL_RenderDrawLine(renderer, points[i].x + 1, points[i].y, points[j].x + 1, points[j].y);
            SDL_RenderDrawLine(renderer, points[i].x, points[i].y + 1, points[j].x, points[j].y + 1);
        }
    }

    static const std::vector<MapLabels::Placement>& labelLayout(TTF_Font* font, const World& world) {
        static size_t cachedKey = 0;
        static std::vector<MapLabels::Placement> cached;
        size_t key = std::hash<TTF_Font*>{}(font);
        std::vector<Point> sizes;
        auto combine = [&](size_t value) { key ^= value + 0x9e3779b9 + (key << 6) + (key >> 2); };
        for (const auto& province : world.provinces) {
            int width = 0, height = 0;
            TTF_SizeText(font, province.name.c_str(), &width, &height);
            sizes.push_back({(int)std::ceil(width / MAP_SCALE_X) + 1, (int)std::ceil(height / MAP_SCALE_Y) + 1});
            combine(std::hash<std::string>{}(province.name));
            combine(width);
            combine(height);
            combine(province.labelPosition.x);
            combine(province.labelPosition.y);
            combine(province.hasLabelPosition);
            auto hashPolygon = [&](const std::vector<Point>& polygon) {
                combine(polygon.size());
                for (const auto& point : polygon) {
                    combine(point.x);
                    combine(point.y);
                }
            };
            hashPolygon(province.polygon);
            combine(province.islands.size());
            for (const auto& island : province.islands) {
                hashPolygon(island);
            }
        }
        if (cachedKey != key || cached.size() != world.provinces.size()) {
            cached = MapLabels::layout(world.provinces, sizes);
            cachedKey = key;
        }
        return cached;
    }

    static void drawLeader(SDL_Renderer* renderer, const MapLabels::Placement& placement) {
        if (!placement.external) {
            return;
        }
        const auto& bounds = placement.bounds;
        Point target = offsetPoint(placement.target);
        Point start = offsetPoint({std::clamp(placement.target.x, bounds.x - 3, bounds.x + bounds.w + 3),
                                   std::clamp(placement.target.y, bounds.y - 3, bounds.y + bounds.h + 3)});
        SDL_SetRenderDrawColor(renderer, 20, 30, 30, 255);
        for (int dx = -1; dx <= 1; dx++) {
            SDL_RenderDrawLine(renderer, start.x + dx, start.y, target.x + dx, target.y);
            SDL_RenderDrawLine(renderer, start.x, start.y + dx, target.x, target.y + dx);
        }
        SDL_Rect dot = {target.x - 2, target.y - 2, 5, 5};
        SDL_RenderFillRect(renderer, &dot);
        SDL_SetRenderDrawColor(renderer, LABEL.r, LABEL.g, LABEL.b, 255);
        SDL_RenderDrawLine(renderer, start.x, start.y, target.x, target.y);
        dot = {target.x - 1, target.y - 1, 3, 3};
        SDL_RenderFillRect(renderer, &dot);
    }

    static void drawLabel(SDL_Renderer* renderer, TTF_Font* font, const ProvinceComponent& province, const MapLabels::Placement& placement) {
        const auto& bounds = placement.bounds;
        Point center = offsetPoint({bounds.x + bounds.w / 2, bounds.y + bounds.h / 2});
        SDL_Color color = province.isSelected ? LABEL_SEL : LABEL;
        SDL_Surface* s = TTF_RenderText_Blended(font, province.name.c_str(), color);
        if (!s) {
            return;
        }
        SDL_Texture* t = SDL_CreateTextureFromSurface(renderer, s);
        SDL_Rect r = {center.x - s->w / 2, center.y - s->h / 2, s->w, s->h};
        if (placement.external) {
            SDL_Rect plate = {r.x - 3, r.y - 2, r.w + 6, r.h + 4};
            SDL_SetRenderDrawColor(renderer, 28, 42, 45, 255);
            SDL_RenderFillRect(renderer, &plate);
        }
        // A dark halo keeps gold and selected white text legible on every owner
        // color, including the pale selected-province fill.
        SDL_SetTextureColorMod(t, 12, 18, 18);
        for (int dy = -1; dy <= 1; dy++) {
            for (int dx = -1; dx <= 1; dx++) {
                if (dx != 0 || dy != 0) {
                    SDL_Rect outline = {r.x + dx, r.y + dy, r.w, r.h};
                    SDL_RenderCopy(renderer, t, nullptr, &outline);
                }
            }
        }
        SDL_SetTextureColorMod(t, 255, 255, 255);
        SDL_RenderCopy(renderer, t, nullptr, &r);
        SDL_FreeSurface(s);
        SDL_DestroyTexture(t);
    }

    void render(SDL_Renderer* renderer, TTF_Font* font, const World& world) {
        SDL_Rect mapArea = {0, MAP_Y, 950, 740};
        SDL_RenderSetClipRect(renderer, &mapArea);

        // Water background
        SDL_SetRenderDrawColor(renderer, WATER.r, WATER.g, WATER.b, WATER.a);
        SDL_RenderFillRect(renderer, &mapArea);

        // Fill provinces
        for (auto& p : world.provinces) {
            SDL_Color color;
            if (p.isSelected) {
                color = {144, 238, 144, 255};
            } else if (p.owner == world.ctx.playerDynasty) {
                color = {34, 139, 34, 255};
            } else if (world.scoutedProvinces.count(p.id)) {
                // Color by owner
                if (p.owner == "Baldwin II")        color = {128,   0, 128, 255};
                else if (p.owner == "Kantakouzenos") color = {180,  30,  30, 255};
                else if (p.owner == "Doukas")        color = {180,   0, 180, 255};
                else if (p.owner == "Palaiologos")   color = {200, 100,   0, 255};
                else if (p.owner == "Phokas")        color = {139,  69,  19, 255};
                else if (p.owner == "Komnenos")      color = {0,   139, 139, 255};
                else                                 color = {34,  139,  34, 255};
            } else {
                color = {34, 139, 34, 255};
            }
            fillPolygon(renderer, offsetPolygon(p.polygon), color);
            for (const auto& island : p.islands) {
                fillPolygon(renderer, offsetPolygon(island), color);
            }
        }

        // Draw borders
        for (auto& p : world.provinces) {
            bool owned = p.owner == world.ctx.playerDynasty;
            drawProvinceBorder(renderer, p.polygon, owned);
            for (const auto& island : p.islands) {
                drawProvinceBorder(renderer, island, owned);
            }
        }

        // Draw labels
        const auto& placements = labelLayout(font, world);
        for (const auto& placement : placements) {
            drawLeader(renderer, placement);
        }
        for (size_t i = 0; i < world.provinces.size(); i++) {
            drawLabel(renderer, font, world.provinces[i], placements[i]);
        }

        SDL_RenderSetClipRect(renderer, nullptr);
    }

}
