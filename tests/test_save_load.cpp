#include <catch2/catch_test_macros.hpp>
#include "core/systems/save_load.h"
#include "core/systems/map_system.h"
#include "core/systems/game_system.h"
#include "core/systems/resource_system.h"
#include "core/systems/recruit_system.h"
#include "core/systems/combat_system.h"
#include "core/systems/diplomacy_system.h"
#include "core/systems/trade_system.h"
#include <filesystem>
#include <fstream>
#include <algorithm>
#include <nlohmann/json.hpp>

static World freshMap() {
    World world;
    auto path = std::filesystem::path(__FILE__).parent_path().parent_path() / "data/map.json";
    MapSystem::load(world, path.string());
    return world;
}

struct SaveFile {
    std::filesystem::path path = std::filesystem::temp_directory_path() / "castles3-save-roundtrip-test.json";
    ~SaveFile() {
        std::filesystem::remove(path);
    }
};

TEST_CASE("save load restores a running campaign with tasks workers armies and battle", "[save]") {
    auto source = freshMap();
    source.ctx.playerDynasty = "Player";
    source.ctx.screen = GameScreen::Playing;
    source.ctx.score = 650;
    source.ctx.dayCounter = 83;
    source.ctx.upkeepCounter = 24;
    source.ctx.upkeepDue = true;
    source.ctx.paused = true;
    source.date = {17, 8, 1313};
    source.resources = {30, 31, 32, 33};
    source.armies["Player"] = {"Player", 8, 5, 2};
    source.armies["Enemy"] = {"Enemy", 4, 3, 1};
    source.findProvince(0)->owner = "Player";
    source.findProvince(17)->owner = "Player";
    source.findProvince(13)->owner = "Enemy";
    source.findProvince(13)->isRevealed = false;
    source.scoutedProvinces.insert(16);
    source.aiStates["Enemy"] = {11, 12};
    source.aiMarches.push_back({true, "Enemy", 17, 9, 2});
    REQUIRE(ResourceSystem::startTask(source, ResourceType::Food, 1));
    REQUIRE(ResourceSystem::startTask(source, ResourceType::Gold, 2));
    REQUIRE(CombatSystem::startMarch(source, 13, 1));
    REQUIRE(RecruitSystem::startRecruitment(source, UnitType::Infantry, 2));
    REQUIRE(DiplomacySystem::startScout(source, 20, 1));
    source.pendingTradeDynasty = "Enemy";
    source.pendingTradeQty = 3;
    source.pendingDiplomaticWorkers = 2;
    REQUIRE(TradeSystem::startTrade(source));
    source.collectionTasks.slots[0].daysAccumulated = 7;
    source.combatTasks.slots[0].daysAccumulated = 2;
    source.recruitTasks.slots[0].daysAccumulated = 3;
    source.scoutTasks.slots[0].daysAccumulated = 4;
    source.tradeTasks.slots[0].daysAccumulated = 5;
    CombatSystem::initBattle(source, 17, "Enemy");
    source.battle.phase = BattlePhase::Paused;
    source.battle.playerHealth -= 3;
    source.battle.roundTimer = 0.4f;
    source.pendingMilitaryWorkers = 1;
    SaveFile file;
    SaveLoad::save_game(source, file.path.string());
    REQUIRE(std::filesystem::file_size(file.path) > 0);
    auto restored = freshMap();
    REQUIRE(SaveLoad::load_game(restored, file.path.string()));
    REQUIRE(restored.ctx.playerDynasty == "Player");
    REQUIRE(restored.ctx.score == 650);
    REQUIRE(restored.ctx.dayCounter == 83);
    REQUIRE(restored.ctx.upkeepCounter == 24);
    REQUIRE(restored.ctx.upkeepDue);
    REQUIRE(restored.ctx.paused);
    REQUIRE(restored.date.day == 17);
    REQUIRE(restored.date.month == 8);
    REQUIRE(restored.date.year == 1313);
    REQUIRE(restored.resources.food == source.resources.food);
    REQUIRE(restored.resources.iron == source.resources.iron);
    REQUIRE(restored.resources.timber == source.resources.timber);
    REQUIRE(restored.resources.gold == source.resources.gold);
    REQUIRE(restored.workerPool.availableStockWorkers == 1);
    REQUIRE(restored.workerPool.availableMilitaryWorkers == 1);
    REQUIRE(restored.workerPool.availableDiplomaticWorkers == 1);
    REQUIRE(restored.armies.at("Player").knights == 2);
    REQUIRE(restored.armies.at("Enemy").infantry == 4);
    REQUIRE(restored.findProvince(13)->owner == "Enemy");
    REQUIRE_FALSE(restored.findProvince(13)->isRevealed);
    REQUIRE(restored.scoutedProvinces.count(16) == 1);
    REQUIRE(restored.collectionTasks.slots[0].daysAccumulated == 7);
    REQUIRE(restored.combatTasks.slots[0].targetProvinceId == 13);
    REQUIRE(restored.recruitTasks.slots[0].daysAccumulated == 3);
    REQUIRE(restored.scoutTasks.slots[0].daysAccumulated == 4);
    REQUIRE(restored.tradeTasks.slots[0].qty == 3);
    REQUIRE(restored.tradeTasks.slots[0].daysAccumulated == 5);
    REQUIRE(restored.aiStates.at("Enemy").recruitCounter == 12);
    REQUIRE(restored.aiMarches[0].targetProvinceId == 17);
    REQUIRE(restored.battle.phase == BattlePhase::Paused);
    REQUIRE(restored.battle.defenderDynasty == "Enemy");
    REQUIRE(restored.battle.playerHealth == source.battle.playerHealth);
    REQUIRE(restored.battle.roundTimer == source.battle.roundTimer);
    REQUIRE(restored.battle.aiUnits.size() == source.battle.aiUnits.size());
    REQUIRE(restored.pendingMilitaryWorkers == 1);
}

TEST_CASE("missing or malformed save is reported as unsuccessful", "[save]") {
    auto world = freshMap();
    SaveFile file;
    std::filesystem::remove(file.path);
    REQUIRE_FALSE(SaveLoad::load_game(world, file.path.string()));
    {
        std::ofstream output(file.path);
        output << "{not JSON";
    }
    REQUIRE_FALSE(SaveLoad::load_game(world, file.path.string()));
}

TEST_CASE("randomized province resources survive a fresh map load by province ID", "[save][resource-map]") {
    auto source = freshMap();
    auto defaults = source;
    GameSystem::randomizeResources(source);
    auto changed = std::find_if(source.provinces.begin(), source.provinces.end(), [&](const auto& p) {
        return p.resource != defaults.findProvince(p.id)->resource;
    });
    // A shuffle may theoretically produce the original arrangement. Keep the
    // regression deterministic while retaining a valid resource permutation.
    if (changed == source.provinces.end()) {
        std::swap(source.findProvince(1)->resource, source.findProvince(2)->resource);
        changed = std::find_if(source.provinces.begin(), source.provinces.end(), [&](const auto& p) {
            return p.resource != defaults.findProvince(p.id)->resource;
        });
    }
    REQUIRE(changed != source.provinces.end());
    int ownedId = changed->id;
    source.ctx.playerDynasty = "Player";
    changed->owner = "Player";
    SaveFile file;
    SaveLoad::save_game(source, file.path.string());
    auto restored = freshMap();
    // Province IDs, rather than vector order, must determine restoration.
    std::reverse(restored.provinces.begin(), restored.provinces.end());
    REQUIRE(SaveLoad::load_game(restored, file.path.string()));
    for (const auto& province : source.provinces) {
        INFO(province.name);
        REQUIRE(restored.findProvince(province.id)->resource == province.resource);
    }
    REQUIRE(restored.findProvince(0)->resource == defaults.findProvince(0)->resource);
    REQUIRE(restored.findProvince(ownedId)->owner == "Player");
    const char* resources[] = {"Food", "Timber", "Iron", "Gold"};
    for (int i = 0; i < 4; i++) {
        if (source.findProvince(ownedId)->resource == resources[i]) {
            auto type = static_cast<ResourceType>(i);
            REQUIRE(restored.countOwnedProvinces("Player", type) == 1);
            REQUIRE(ResourceSystem::startTask(restored, type, 1));
            restored.collectionTasks.slots[0].daysAccumulated = restored.collectionTasks.slots[0].daysRequired - 1;
            int before[] = {restored.resources.food, restored.resources.timber, restored.resources.iron, restored.resources.gold};
            ResourceSystem::tick(restored);
            int after[] = {restored.resources.food, restored.resources.timber, restored.resources.iron, restored.resources.gold};
            REQUIRE(after[i] == before[i] + 1);
        }
    }
}

TEST_CASE("older saves without province resources retain map defaults", "[save][resource-map]") {
    auto source = freshMap();
    source.findProvince(17)->owner = "Player";
    SaveFile file;
    SaveLoad::save_game(source, file.path.string());
    nlohmann::json legacy;
    {
        std::ifstream input(file.path);
        input >> legacy;
    }
    for (auto& province : legacy["provinces"]) {
        province.erase("resource");
    }
    {
        std::ofstream output(file.path);
        output << legacy.dump(2);
    }
    auto restored = freshMap();
    REQUIRE(SaveLoad::load_game(restored, file.path.string()));
    REQUIRE(restored.findProvince(17)->owner == "Player");
    for (const auto& province : source.provinces) {
        REQUIRE(restored.findProvince(province.id)->resource == province.resource);
    }
}
