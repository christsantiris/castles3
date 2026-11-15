#include <catch2/catch_test_macros.hpp>
#include "core/systems/task_slots.h"
#include "core/systems/combat_system.h"
#include "core/systems/recruit_system.h"
#include "core/systems/resource_system.h"
#include "core/systems/diplomacy_system.h"
#include "core/systems/trade_system.h"
#include "core/systems/save_load.h"
#include <filesystem>

static World taskWorld() {
    World w;
    w.ctx.playerDynasty = "Player";
    ProvinceComponent home;
    home.id = 0;
    home.owner = "Player";
    home.resource = "Food";
    home.neighbors = {1};
    ProvinceComponent enemy;
    enemy.id = 1;
    enemy.name = "Target";
    enemy.owner = "Enemy";
    enemy.resource = "Timber";
    w.provinces = {home, enemy};
    w.armies["Player"] = {"Player", 4, 2, 0};
    w.resources.food = 20;
    w.resources.iron = 20;
    w.resources.timber = 20;
    w.resources.gold = 20;
    w.pendingTradeDynasty = "Enemy";
    return w;
}

TEST_CASE("military category shares one slot until score 400 in either start order", "[tasks]") {
    for (int score : {0, 399, 400}) {
        for (bool recruitFirst : {false, true}) {
            auto w = taskWorld();
            w.ctx.score = score;
            bool first = recruitFirst ? RecruitSystem::startRecruitment(w, UnitType::Infantry, 2) : CombatSystem::startMarch(w, 1, 2);
            REQUIRE(first);
            int food = w.resources.food;
            int iron = w.resources.iron;
            bool second = recruitFirst ? CombatSystem::startMarch(w, 1, 2) : RecruitSystem::startRecruitment(w, UnitType::Infantry, 2);
            REQUIRE(second == (score >= 400));
            REQUIRE(w.workerPool.availableMilitaryWorkers == (score >= 400 ? 0 : 2));
            if (!second) {
                REQUIRE(w.resources.food == food);
                REQUIRE(w.resources.iron == iron);
            }
            REQUIRE_FALSE(RecruitSystem::startRecruitment(w, UnitType::Archers, 1));
        }
    }
}

TEST_CASE("zero and excessive workers cannot start any task", "[tasks]") {
    for (int workers : {-1, 0, 5}) {
        auto w = taskWorld();
        REQUIRE_FALSE(ResourceSystem::startTask(w, ResourceType::Food, workers));
        REQUIRE_FALSE(CombatSystem::startMarch(w, 1, workers));
        REQUIRE_FALSE(RecruitSystem::startRecruitment(w, UnitType::Infantry, workers));
        REQUIRE_FALSE(DiplomacySystem::startScout(w, 1, workers));
        REQUIRE_FALSE(DiplomacySystem::startBribe(w, 1, workers, 1));
        w.pendingDiplomaticWorkers = workers;
        REQUIRE_FALSE(TradeSystem::startTrade(w));
        REQUIRE(w.resources.food == 20);
        REQUIRE(w.resources.iron == 20);
        REQUIRE(w.resources.gold == 20);
    }
}

TEST_CASE("two recruits consume both military slots even with spare workers", "[tasks]") {
    auto w = taskWorld();
    w.ctx.score = 400;
    REQUIRE(RecruitSystem::startRecruitment(w, UnitType::Infantry, 1));
    REQUIRE(RecruitSystem::startRecruitment(w, UnitType::Archers, 1));
    REQUIRE(w.workerPool.availableMilitaryWorkers == 2);
    REQUIRE_FALSE(CombatSystem::startMarch(w, 1, 1));
    RecruitSystem::cancelRecruitment(w, 0);
    REQUIRE(CombatSystem::startMarch(w, 1, 1));
}

TEST_CASE("scouting and bribery share diplomatic capacity", "[tasks]") {
    auto w = taskWorld();
    REQUIRE(DiplomacySystem::startBribe(w, 1, 1, 1));
    REQUIRE_FALSE(DiplomacySystem::startScout(w, 1, 1));
    w.ctx.score = 600;
    REQUIRE(DiplomacySystem::startScout(w, 1, 1));
    REQUIRE(TaskSlots::diplomaticCount(w) == 2);
    w.pendingDiplomaticWorkers = 1;
    REQUIRE_FALSE(TradeSystem::startTrade(w));
}

TEST_CASE("players explicitly allocate workers and may use their whole pool", "[tasks]") {
    auto w = taskWorld();
    REQUIRE(w.pendingMilitaryWorkers == 0);
    REQUIRE(w.pendingDiplomaticWorkers == 0);
    REQUIRE(w.pendingWorkers[0] == 0);
    REQUIRE_FALSE(CombatSystem::startMarch(w, 1, w.pendingMilitaryWorkers));
    w.pendingMilitaryWorkers = 4;
    REQUIRE(CombatSystem::startMarch(w, 1, w.pendingMilitaryWorkers));
    REQUIRE(w.pendingMilitaryWorkers == 0);
    REQUIRE(w.workerPool.availableMilitaryWorkers == 0);
    CombatSystem::cancelMarch(w, 0);
    REQUIRE(w.workerPool.availableMilitaryWorkers == 4);
    REQUIRE(RecruitSystem::startRecruitment(w, UnitType::Infantry, 4));
}

TEST_CASE("administrative and diplomatic categories enforce their unlock boundaries", "[tasks]") {
    for (int score : {199, 200}) {
        auto w = taskWorld();
        w.ctx.score = score;
        REQUIRE(ResourceSystem::startTask(w, ResourceType::Food, 1));
        REQUIRE(ResourceSystem::startTask(w, ResourceType::Food, 1) == (score == 200));
        REQUIRE_FALSE(ResourceSystem::startTask(w, ResourceType::Food, 1));
    }
    for (int score : {599, 600}) {
        for (bool tradeFirst : {false, true}) {
            auto w = taskWorld();
            w.ctx.score = score;
            w.pendingDiplomaticWorkers = 1;
            REQUIRE((tradeFirst ? TradeSystem::startTrade(w) : DiplomacySystem::startScout(w, 1, 1)));
            w.pendingDiplomaticWorkers = 1;
            REQUIRE((tradeFirst ? DiplomacySystem::startScout(w, 1, 1) : TradeSystem::startTrade(w)) == (score == 600));
            REQUIRE_FALSE(DiplomacySystem::startBribe(w, 1, 1, 1));
        }
    }
}

TEST_CASE("completion frees the shared slot for a different task type", "[tasks]") {
    auto w = taskWorld();
    REQUIRE(RecruitSystem::startRecruitment(w, UnitType::Infantry, 2));
    w.recruitTasks.slots[0].daysAccumulated = w.recruitTasks.slots[0].daysRequired - 1;
    RecruitSystem::tick(w);
    REQUIRE(TaskSlots::militaryCount(w) == 0);
    REQUIRE(w.workerPool.availableMilitaryWorkers == 4);
    REQUIRE(CombatSystem::startMarch(w, 1, 2));
}

TEST_CASE("save loading preserves earned capacity and repairs old overflowing tasks", "[tasks][save]") {
    for (int score : {0, 400}) {
        auto w = taskWorld();
        w.ctx.score = 400;
        REQUIRE(CombatSystem::startMarch(w, 1, 1));
        REQUIRE(RecruitSystem::startRecruitment(w, UnitType::Infantry, 1));
        w.ctx.score = score;
        auto path = std::filesystem::temp_directory_path() / "castles3-task-limits-test.json";
        SaveLoad::save_game(w, path.string());
        auto restored = taskWorld();
        bool loaded = SaveLoad::load_game(restored, path.string());
        std::filesystem::remove(path);
        REQUIRE(loaded);
        REQUIRE(TaskSlots::militaryCount(restored) == (score == 400 ? 2 : 1));
        REQUIRE(restored.workerPool.availableMilitaryWorkers == (score == 400 ? 2 : 3));
        REQUIRE(restored.resources.iron == (score == 400 ? 17 : 20));
        REQUIRE_FALSE(RecruitSystem::startRecruitment(restored, UnitType::Archers, 1));
    }
}
