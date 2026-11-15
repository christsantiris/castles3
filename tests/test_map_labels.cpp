#include <catch2/catch_test_macros.hpp>
#include "core/map_labels.h"
#include "core/systems/map_system.h"
#include <filesystem>

TEST_CASE("province labels fit wholly inside or use unobstructed sea callouts", "[map][labels]") {
    World world;
    auto path = std::filesystem::path(__FILE__).parent_path().parent_path() / "data/map.json";
    MapSystem::load(world, path.string());
    std::vector<Point> sizes;
    // Conservative dimensions in map coordinates, including the text halo.
    for (const auto& province : world.provinces) {
        sizes.push_back({(int)province.name.size() * 7, 20});
    }
    auto placements = MapLabels::layout(world.provinces, sizes);
    REQUIRE(placements.size() == 24);
    int callouts = 0;
    for (size_t i = 0; i < placements.size(); i++) {
        const auto& p = placements[i];
        INFO(world.provinces[i].name);
        REQUIRE(p.bounds.x >= 0);
        REQUIRE(p.bounds.y >= 0);
        REQUIRE(p.bounds.x + p.bounds.w <= 940);
        REQUIRE(p.bounds.y + p.bounds.h <= 880);
        if (p.external) {
            callouts++;
            REQUIRE(MapSystem::containsPoint(world.provinces[i], p.target.x, p.target.y));
        }
        bool fits = true;
        for (int y = p.bounds.y; y < p.bounds.y + p.bounds.h; y++) {
            for (int x = p.bounds.x; x < p.bounds.x + p.bounds.w; x++) {
                if (p.external) {
                    for (const auto& province : world.provinces) {
                        fits = fits && !MapSystem::containsPoint(province, x, y);
                    }
                } else {
                    fits = fits && MapSystem::containsPoint(world.provinces[i], x, y);
                }
            }
        }
        REQUIRE(fits);
        for (size_t j = 0; j < i; j++) {
            const auto& a = p.bounds;
            const auto& b = placements[j].bounds;
            bool overlap = a.x < b.x + b.w && b.x < a.x + a.w && a.y < b.y + b.h && b.y < a.y + a.h;
            REQUIRE_FALSE(overlap);
        }
    }
    REQUIRE(callouts > 0);
}

TEST_CASE("Cilicia and Syria no longer include disconnected mainland scraps", "[map]") {
    World world;
    auto path = std::filesystem::path(__FILE__).parent_path().parent_path() / "data/map.json";
    MapSystem::load(world, path.string());
    REQUIRE(world.provinces[11].islands.empty());
    REQUIRE(world.provinces[14].islands.empty());
    REQUIRE(world.provinces[20].islands.empty());
}
