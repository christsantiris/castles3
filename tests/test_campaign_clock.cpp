#include <catch2/catch_test_macros.hpp>
#include "core/systems/game_system.h"
#include "core/systems/combat_system.h"
#include "core/systems/ai_system.h"

TEST_CASE("every battle phase and manual pause freeze campaign systems", "[clock]") {
    for (auto phase : {BattlePhase::Preparing, BattlePhase::Running, BattlePhase::Paused, BattlePhase::Resolved, BattlePhase::None}) {
        World w;
        w.battle.phase = phase;
        w.ctx.paused = phase == BattlePhase::None;
        w.collectionTasks.slots[0].active = true;
        w.collectionTasks.slots[0].daysRequired = 10;
        w.recruitTasks.slots[0].active = true;
        w.recruitTasks.slots[0].daysRequired = 10;
        w.tradeTasks.slots[0].active = true;
        w.tradeTasks.slots[0].daysRequired = 10;
        GameSystem::tick(w);
        REQUIRE(w.date.day == 2);
        REQUIRE(w.ctx.dayCounter == 0);
        REQUIRE(w.ctx.upkeepCounter == 0);
        REQUIRE(w.ctx.score == 0);
        REQUIRE(w.collectionTasks.slots[0].daysAccumulated == 0);
        REQUIRE(w.recruitTasks.slots[0].daysAccumulated == 0);
        REQUIRE(w.tradeTasks.slots[0].daysAccumulated == 0);
    }
}

TEST_CASE("arriving marches wait for battle results to be dismissed", "[clock]") {
    World w;
    w.ctx.playerDynasty = "Phokas";
    w.ctx.score = 400;
    GameSystem::initArmies(w);
    ProvinceComponent home;
    home.id = 0;
    home.owner = "Phokas";
    home.neighbors = {1, 2};
    ProvinceComponent first;
    first.id = 1;
    first.name = "First";
    first.owner = "neutral";
    ProvinceComponent second = first;
    second.id = 2;
    second.name = "Second";
    w.provinces = {home, first, second};
    REQUIRE(CombatSystem::startMarch(w, 1, 1));
    REQUIRE(CombatSystem::startMarch(w, 2, 1));
    w.combatTasks.slots[0].daysRequired = 1;
    w.combatTasks.slots[1].daysRequired = 1;
    GameSystem::tick(w);
    REQUIRE(w.battle.targetProvinceId == 1);
    REQUIRE(w.battle.phase == BattlePhase::Preparing);
    REQUIRE(w.combatTasks.slots[1].active);
    REQUIRE(w.combatTasks.slots[1].daysAccumulated == 0);
    REQUIRE(w.date.day == 3);
    w.battle.phase = BattlePhase::Resolved;
    GameSystem::tick(w);
    CombatSystem::tickMarch(w);
    REQUIRE(w.battle.targetProvinceId == 1);
    REQUIRE(w.date.day == 3);
    w.battle = BattleState{};
    GameSystem::tick(w);
    REQUIRE(w.battle.targetProvinceId == 2);
    REQUIRE(w.battle.phase == BattlePhase::Preparing);
    REQUIRE(w.date.day == 4);
}

TEST_CASE("AI march cannot replace a battle awaiting acknowledgment", "[clock]") {
    World w;
    AIMarchTask march;
    march.active = true;
    march.daysRequired = 1;
    w.aiMarches.push_back(march);
    w.battle.phase = BattlePhase::Resolved;
    w.battle.targetProvinceId = 42;
    AISystem::tickMarches(w);
    REQUIRE(w.battle.targetProvinceId == 42);
    REQUIRE(w.aiMarches[0].active);
    REQUIRE(w.aiMarches[0].daysAccumulated == 0);
}
