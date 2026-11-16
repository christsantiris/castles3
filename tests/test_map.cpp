#include <catch2/catch_test_macros.hpp>
#include "core/world.h"
#include "core/systems/combat_system.h"
#include "core/systems/map_system.h"
#include "core/systems/save_load.h"
#include <filesystem>

static ProvinceComponent makeRect(int id, int x, int y, int w, int h) {
    ProvinceComponent p;
    p.id   = id;
    p.name = "Province" + std::to_string(id);
    p.owner    = "neutral";
    p.resource = "Food";
    p.polygon  = {{x, y}, {x+w, y}, {x+w, y+h}, {x, y+h}};
    return p;
}

TEST_CASE("point inside rectangle province", "[map]") {
    auto p = makeRect(0, 0, 0, 100, 100);
    REQUIRE(MapSystem::containsPoint(p, 50, 50) == true);
}

TEST_CASE("point outside rectangle province", "[map]") {
    auto p = makeRect(0, 0, 0, 100, 100);
    REQUIRE(MapSystem::containsPoint(p, 150, 150) == false);
}

TEST_CASE("point clearly outside province", "[map]") {
    auto p = makeRect(0, 0, 0, 100, 100);
    REQUIRE(MapSystem::containsPoint(p, -1, 50) == false);
}

TEST_CASE("click selects a province", "[map]") {
    World w;
    w.provinces.push_back(makeRect(0, 0, 0, 100, 100));
    auto* selected = MapSystem::handleClick(w, 50, 110);
    REQUIRE(selected != nullptr);
    REQUIRE(selected->id == 0);
    REQUIRE(w.provinces[0].isSelected == true);
}

TEST_CASE("click same province deselects it", "[map]") {
    World w;
    w.provinces.push_back(makeRect(0, 0, 0, 100, 100));
    MapSystem::handleClick(w, 50, 110);
    auto* selected = MapSystem::handleClick(w, 50, 110);
    REQUIRE(selected == nullptr);
    REQUIRE(w.provinces[0].isSelected == false);
}

TEST_CASE("clicking one province deselects others", "[map]") {
    World w;
    w.provinces.push_back(makeRect(0, 0,   0, 100, 100));
    w.provinces.push_back(makeRect(1, 200, 0, 100, 100));
    MapSystem::handleClick(w, 50, 110);
    MapSystem::handleClick(w, 250, 110);
    REQUIRE(w.provinces[0].isSelected == false);
    REQUIRE(w.provinces[1].isSelected == true);
}

TEST_CASE("click empty area deselects all", "[map]") {
    World w;
    w.provinces.push_back(makeRect(0, 0, 0, 100, 100));
    MapSystem::handleClick(w, 50, 110);
    MapSystem::handleClick(w, 500, 500);
    REQUIRE(w.provinces[0].isSelected == false);
}

static World loadCoastlineMap() {
    World world;
    auto path = std::filesystem::path(__FILE__).parent_path().parent_path() / "data/map.json";
    MapSystem::load(world, path.string());
    return world;
}

TEST_CASE("coastline map loads all gameplay provinces and label anchors", "[map]") {
    auto world = loadCoastlineMap();
    REQUIRE(world.provinces.size() == 24);
    for (size_t i = 0; i < world.provinces.size(); i++) {
        const auto& province = world.provinces[i];
        REQUIRE(province.id == i);
        REQUIRE(province.polygon.size() >= 3);
        REQUIRE(province.hasLabelPosition);
        REQUIRE(province.labelPosition.x >= 0);
        REQUIRE(province.labelPosition.x < 940);
        REQUIRE(province.labelPosition.y >= 0);
        REQUIRE(province.labelPosition.y < 880);
    }
}

TEST_CASE("real coastlines keep landmarks selectable and open sea empty", "[map]") {
    auto world = loadCoastlineMap();
    // Projected locations of Istanbul, Chios, Samos, central Crete and Cyprus.
    REQUIRE(MapSystem::containsPoint(world.provinces[0], 512, 146));
    REQUIRE(MapSystem::containsPoint(world.provinces[12], 425, 266));
    REQUIRE(MapSystem::containsPoint(world.provinces[12], 449, 295));
    REQUIRE(MapSystem::containsPoint(world.provinces[17], 394, 411));
    REQUIRE(MapSystem::containsPoint(world.provinces[13], 643, 418));
    REQUIRE_FALSE(world.provinces[12].islands.empty());
    for (const auto& province : world.provinces) {
        // Open Mediterranean at 28 E, 34 N.
        REQUIRE_FALSE(MapSystem::containsPoint(province, 485, 462));
    }
    // The screen-space click lands on Samos, a secondary piece of Chios.
    auto* selected = MapSystem::handleClick(world, 454, 309);
    REQUIRE(selected != nullptr);
    REQUIRE(selected->id == 12);
    REQUIRE(MapSystem::handleClick(world, 491, 449) == nullptr);
    REQUIRE_FALSE(world.provinces[12].isSelected);
}

TEST_CASE("Thrace and Anatolia can attack each other without Constantinople", "[map]") {
    auto world = loadCoastlineMap();
    world.ctx.playerDynasty = "Player";
    world.provinces[4].owner = "Player";
    REQUIRE(world.provinces[4].name == "Thrace");
    REQUIRE(world.provinces[7].name == "Anatolia");
    REQUIRE(world.provinces[0].owner != "Player");
    REQUIRE(CombatSystem::isAdjacent(world, 7));

    world.provinces[4].owner = "neutral";
    world.provinces[7].owner = "Player";
    REQUIRE(CombatSystem::isAdjacent(world, 4));
}

TEST_CASE("saved ownership restores onto current static coastline geometry", "[map][save]") {
    auto world = loadCoastlineMap();
    world.provinces[12].owner = "Phokas";
    auto path = std::filesystem::temp_directory_path() / "castles3-coastline-save-test.json";
    SaveLoad::save_game(world, path.string());
    auto restored = loadCoastlineMap();
    bool loaded = SaveLoad::load_game(restored, path.string());
    std::filesystem::remove(path);
    REQUIRE(loaded);
    REQUIRE(restored.provinces[12].owner == "Phokas");
    REQUIRE(restored.provinces[12].hasLabelPosition);
    REQUIRE(MapSystem::containsPoint(restored.provinces[12], 449, 295));
}

TEST_CASE("clicking active tab deactivates it", "[input]") {
    World w;
    w.ctx.activeTab = 0;

    // Simulate tab click on already active tab
    w.ctx.activeTab = (w.ctx.activeTab == 0) ? -1 : 0;

    REQUIRE(w.ctx.activeTab == -1);
}

TEST_CASE("clicking inactive tab activates it", "[input]") {
    World w;
    w.ctx.activeTab = -1;

    // Simulate tab click on inactive tab
    w.ctx.activeTab = (w.ctx.activeTab == 0) ? -1 : 0;

    REQUIRE(w.ctx.activeTab == 0);
}

TEST_CASE("clicking different tab switches active tab", "[input]") {
    World w;
    w.ctx.activeTab = 0;

    // Simulate clicking tab 1 while tab 0 is active
    int tabIndex = 1;
    w.ctx.activeTab = (w.ctx.activeTab == tabIndex) ? -1 : tabIndex;

    REQUIRE(w.ctx.activeTab == 1);
}
