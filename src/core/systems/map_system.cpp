#include "map_system.h"
#include <nlohmann/json.hpp>
#include <fstream>

using json = nlohmann::json;

static const int MAP_Y = 60;

namespace MapSystem {

    void load(World& world, const std::string& path) {
        std::ifstream file(path);
        json data = json::parse(file);

        for (auto& p : data["provinces"]) {
            ProvinceComponent province;
            province.id       = p["id"];
            province.owner    = p["owner"];
            province.resource = p["resource"];
#ifdef DEBUG
            province.name = std::string(p["name"]) + " " + std::to_string((int)p["id"]);
#else
            province.name     = p["name"];
#endif

            for (auto& point : p["polygon"]) {
                Point pt;
                pt.x = point["x"];
                pt.y = point["y"];
                province.polygon.push_back(pt);
            }

            if (p.contains("islands")) {
                for (const auto& island : p["islands"]) {
                    std::vector<Point> points;
                    for (const auto& point : island) {
                        points.push_back({point["x"], point["y"]});
                    }
                    province.islands.push_back(points);
                }
            }
            if (p.contains("label")) {
                province.labelPosition = {p["label"]["x"], p["label"]["y"]};
                province.hasLabelPosition = true;
            }

            for (auto& neighbor : p["neighbors"])
                province.neighbors.push_back(neighbor);

            world.provinces.push_back(province);
        }
    }

    static bool polygonContainsPoint(const std::vector<Point>& polygon, int px, int py) {
        int n = polygon.size();
        bool inside = false;
        for (int i = 0, j = n - 1; i < n; j = i++) {
            int xi = polygon[i].x, yi = polygon[i].y;
            int xj = polygon[j].x, yj = polygon[j].y;
            bool intersect = ((yi > py) != (yj > py)) &&
                             (px < (double)(xj - xi) * (py - yi) / (yj - yi) + xi);
            if (intersect) {
                inside = !inside;
            }
        }
        return inside;
    }

    bool containsPoint(const ProvinceComponent& province, int px, int py) {
        if (polygonContainsPoint(province.polygon, px, py)) {
            return true;
        }
        for (const auto& island : province.islands) {
            if (polygonContainsPoint(island, px, py)) {
                return true;
            }
        }
        return false;
    }

    ProvinceComponent* handleClick(World& world, int x, int y) {
        if (x < 0 || x >= 950 || y < MAP_Y || y >= MAP_Y + 740) {
            return nullptr;
        }
        float scaleX = 940.0f / 950.0f;
        float scaleY = 880.0f / 740.0f;
        int adjustedX = (int)(x * scaleX);
        int adjustedY = (int)((y - 60) * scaleY);

        for (auto& p : world.provinces) {
            if (!containsPoint(p, adjustedX, adjustedY)) continue;
            bool wasSelected = p.isSelected;
            deselectAll(world);
            if (!wasSelected) {
                p.isSelected = true;
                return &p;
            }
            return nullptr;
        }
        deselectAll(world);
        return nullptr;
    }

    void deselectAll(World& world) {
        for (auto& p : world.provinces)
            p.isSelected = false;
    }

}
