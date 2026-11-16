#include <catch2/catch_test_macros.hpp>
#include "core/systems/resource_system.h"
#include "core/systems/combat_system.h"
#include "core/systems/recruit_system.h"
#include "core/systems/diplomacy_system.h"
#include "core/systems/trade_system.h"

TEST_CASE("worker timing tables keep their plateaus", "[tasks]") {
    REQUIRE(ResourceSystem::collectionDays(0, 2) == 0);
    REQUIRE(ResourceSystem::collectionDays(1, 0) == 0);
    REQUIRE(ResourceSystem::collectionDays(1, 1) == 40);
    REQUIRE(ResourceSystem::collectionDays(2, 1) == 20);
    REQUIRE(ResourceSystem::collectionDays(6, 2) == 4);
    REQUIRE(ResourceSystem::collectionDays(7, 2) == 4);
    REQUIRE(ResourceSystem::collectionDays(6, 3) == 3);
    REQUIRE(ResourceSystem::collectionDays(8, 3) == 3);
    REQUIRE(ResourceSystem::collectionDays(4, 4) == 3);
    REQUIRE(ResourceSystem::collectionDays(7, 4) == 3);
    REQUIRE(ResourceSystem::collectionDays(8, 4) == 2);
    REQUIRE(ResourceSystem::collectionDays(3, 5) == 3);
    REQUIRE(ResourceSystem::collectionDays(6, 5) == 3);
    REQUIRE(ResourceSystem::collectionDays(7, 5) == 2);
    REQUIRE(ResourceSystem::collectionDays(8, 6) == ResourceSystem::collectionDays(8, 5));
    REQUIRE(ResourceSystem::collectionDays(9, 1) == 5);

    REQUIRE(DiplomacySystem::diplomaticDays(1) == 20);
    REQUIRE(DiplomacySystem::diplomaticDays(6) == 3);
    REQUIRE(DiplomacySystem::diplomaticDays(7) == 3);
    REQUIRE(DiplomacySystem::diplomaticDays(8) == 2);
    REQUIRE(DiplomacySystem::diplomaticDays(9) == 2);

    REQUIRE(TradeSystem::tradeDays(1) == 40);
    REQUIRE(TradeSystem::tradeDays(8) == 5);
    REQUIRE(TradeSystem::tradeDays(9) == 5);
    REQUIRE(RecruitSystem::recruitmentDays(8) == RecruitSystem::recruitmentDays(9));
    for (int workers = 1; workers < 8; workers++) {
        REQUIRE(TradeSystem::tradeDays(workers) > TradeSystem::tradeDays(workers + 1));
        REQUIRE(RecruitSystem::recruitmentDays(workers) > RecruitSystem::recruitmentDays(workers + 1));
        REQUIRE(CombatSystem::marchDays(workers, true) > CombatSystem::marchDays(workers + 1, true));
    }
    REQUIRE(CombatSystem::marchDays(1, true) == 40);
    REQUIRE(CombatSystem::marchDays(1, false) == 80);
    REQUIRE(CombatSystem::marchDays(8, false) == CombatSystem::marchDays(9, false));
}

TEST_CASE("started tasks store the timing-table duration", "[tasks]") {
    World w;
    w.ctx.playerDynasty = "Player";
    w.ctx.score = 600;
    ProvinceComponent home;
    home.id = 0;
    home.owner = "Player";
    home.resource = "Food";
    home.neighbors = {1};
    ProvinceComponent target;
    target.id = 1;
    target.owner = "neutral";
    target.resource = "Timber";
    target.neighbors = {0};
    w.provinces = {home, target};
    w.resources.food = 20;
    w.resources.iron = 20;
    w.resources.timber = 20;
    w.resources.gold = 20;
    w.armies["Player"] = {"Player", 1, 0, 0};

    REQUIRE(ResourceSystem::startTask(w, ResourceType::Food, 2));
    REQUIRE(w.collectionTasks.slots[0].daysRequired == ResourceSystem::collectionDays(2, 1));
    REQUIRE(DiplomacySystem::startScout(w, 1, 2));
    REQUIRE(w.scoutTasks.slots[0].daysRequired == DiplomacySystem::diplomaticDays(2));
    REQUIRE(CombatSystem::startMarch(w, 1, 2));
    REQUIRE(w.combatTasks.slots[0].daysRequired == CombatSystem::marchDays(2, true));
    REQUIRE(RecruitSystem::startRecruitment(w, UnitType::Infantry, 1));
    REQUIRE(w.recruitTasks.slots[0].daysRequired == RecruitSystem::recruitmentDays(1));

    w.pendingTradeDynasty = "neutral";
    w.pendingTradeOfferRes = 0;
    w.pendingTradeRequestRes = 1;
    w.pendingTradeQty = 1;
    w.pendingDiplomaticWorkers = 1;
    REQUIRE(TradeSystem::startTrade(w));
    REQUIRE(w.tradeTasks.slots[0].daysRequired == TradeSystem::tradeDays(1));
}
