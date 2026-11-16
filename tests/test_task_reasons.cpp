#include <catch2/catch_test_macros.hpp>
#include "core/systems/task_slots.h"
#include "core/systems/resource_system.h"
#include "core/systems/combat_system.h"
#include "core/systems/recruit_system.h"
#include "core/systems/diplomacy_system.h"
#include "core/systems/trade_system.h"

static World reasonWorld() {
    World w;
    w.ctx.playerDynasty = "Player";
    ProvinceComponent home;
    home.id = 0;
    home.name = "Home";
    home.owner = "Player";
    home.resource = "Food";
    home.neighbors = {1};
    ProvinceComponent target;
    target.id = 1;
    target.name = "Target";
    target.owner = "Enemy";
    target.resource = "Timber";
    target.neighbors = {0};
    w.provinces = {home, target};
    w.armies["Player"] = {"Player", 4, 2, 0};
    w.armies["Enemy"] = {"Enemy", 1, 0, 0};
    w.resources.food = 20;
    w.resources.timber = 20;
    w.resources.iron = 20;
    w.resources.gold = 20;
    return w;
}

TEST_CASE("slot capacity follows the score unlocks", "[tasks]") {
    auto w = reasonWorld();
    REQUIRE(TaskSlots::slotCapacity(w, TaskCategory::Administrative) == 1);
    REQUIRE(TaskSlots::slotCapacity(w, TaskCategory::Military) == 1);
    REQUIRE(TaskSlots::slotCapacity(w, TaskCategory::Diplomatic) == 1);
    w.ctx.score = 200;
    REQUIRE(TaskSlots::slotCapacity(w, TaskCategory::Administrative) == 2);
    w.ctx.score = 400;
    REQUIRE(TaskSlots::slotCapacity(w, TaskCategory::Military) == 2);
    w.ctx.score = 600;
    REQUIRE(TaskSlots::slotCapacity(w, TaskCategory::Diplomatic) == 2);
}

TEST_CASE("blocked tasks name the first failed requirement", "[tasks]") {
    auto w = reasonWorld();
    REQUIRE(ResourceSystem::collectBlockReason(w, ResourceType::Food, 0) == "Assign a worker");
    REQUIRE(ResourceSystem::collectBlockReason(w, ResourceType::Timber, 1) == "No province");
    REQUIRE(ResourceSystem::collectBlockReason(w, ResourceType::Food, 1).empty());
    REQUIRE(ResourceSystem::startTask(w, ResourceType::Food, 1));
    REQUIRE(ResourceSystem::collectBlockReason(w, ResourceType::Food, 1) == "No stock slot");
    w.pendingWorkers[0] = 5;
    w.ctx.score = 200;
    REQUIRE(ResourceSystem::collectBlockReason(w, ResourceType::Food, 5) == "Not enough workers");

    auto march = reasonWorld();
    march.ctx.marchFoodFlat = 0;
    REQUIRE(CombatSystem::marchFoodCost(march) == 3);
    march.ctx.marchFoodFlat = 7;
    REQUIRE(CombatSystem::marchFoodCost(march) == 7);
    march.ctx.marchFoodFlat = 0;
    REQUIRE(CombatSystem::marchBlockReason(march, 1, 0) == "Assign a worker");
    march.resources.food = 0;
    REQUIRE(CombatSystem::marchBlockReason(march, 1, 1) == "Need 3 food");
    march.resources.food = 20;
    REQUIRE(CombatSystem::marchBlockReason(march, 1, 1).empty());
    REQUIRE(CombatSystem::marchBlockReason(march, 0, 1) == "Own province");

    auto recruit = reasonWorld();
    REQUIRE(RecruitSystem::recruitBlockReason(recruit, UnitType::Infantry, 0) == "Assign a worker");
    recruit.resources.iron = 0;
    REQUIRE(RecruitSystem::recruitBlockReason(recruit, UnitType::Infantry, 1) == "Need 3 iron");
    recruit.resources.iron = 20;
    recruit.resources.timber = 0;
    REQUIRE(RecruitSystem::recruitBlockReason(recruit, UnitType::Archers, 1) == "Need 3 timber");
    recruit.resources.iron = 0;
    REQUIRE(RecruitSystem::recruitBlockReason(recruit, UnitType::Knights, 1) == "Need iron+timber");
    recruit.resources.iron = 20;
    recruit.resources.timber = 20;
    REQUIRE(RecruitSystem::recruitBlockReason(recruit, UnitType::Knights, 1) == "Knights locked");
    recruit.armies["Player"].infantry = 15;
    REQUIRE(RecruitSystem::recruitBlockReason(recruit, UnitType::Infantry, 1) == "Unit cap");

    auto diplomatic = reasonWorld();
    REQUIRE(DiplomacySystem::scoutBlockReason(diplomatic, 1, 0) == "Assign a worker");
    REQUIRE(DiplomacySystem::scoutBlockReason(diplomatic, 1, 1).empty());
    REQUIRE(DiplomacySystem::startScout(diplomatic, 1, 1));
    REQUIRE(DiplomacySystem::bribeBlockReason(diplomatic, 1, 1, 5) == "No diplomatic slot");
    diplomatic.pendingBribeGold = 0;
    diplomatic.ctx.score = 600;
    REQUIRE(DiplomacySystem::bribeBlockReason(diplomatic, 1, 1, 0) == "Assign gold");
    REQUIRE(DiplomacySystem::bribeBlockReason(diplomatic, 1, 1, 25) == "Need 25 gold");

    auto trade = reasonWorld();
    trade.pendingDiplomaticWorkers = 0;
    REQUIRE(TradeSystem::tradeBlockReason(trade) == "Assign a worker");
    trade.pendingDiplomaticWorkers = 1;
    REQUIRE(TradeSystem::tradeBlockReason(trade) == "Select dynasty");
    trade.pendingTradeDynasty = "Enemy";
    trade.pendingTradeOfferRes = 1;
    trade.pendingTradeRequestRes = 1;
    REQUIRE(TradeSystem::tradeBlockReason(trade) == "Same resource");
    trade.pendingTradeRequestRes = 0;
    REQUIRE(TradeSystem::tradeBlockReason(trade) == "Not produced");
    trade.pendingTradeRequestRes = 1;
    trade.pendingTradeOfferRes = 3;
    trade.pendingTradeQty = 4;
    trade.resources.gold = 1;
    REQUIRE(TradeSystem::tradeBlockReason(trade) == "Need 4 Gold");
    trade.resources.gold = 20;
    REQUIRE(TradeSystem::tradeBlockReason(trade).empty());
}
