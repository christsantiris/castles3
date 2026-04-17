#include "trade_system.h"

namespace TradeSystem {

    static const ResourceType RES_TYPES[] = {
        ResourceType::Food,
        ResourceType::Timber,
        ResourceType::Iron,
        ResourceType::Gold
    };

    static const int daysPerUnit[] = {40, 20, 13, 10, 8, 7, 6, 5};

    const char* resourceName(int index) {
        static const char* names[] = {"Food", "Timber", "Iron", "Gold"};
        return names[index];
    }

    int playerStock(const World& world, int resIndex) {
        switch (resIndex) {
            case 0: return world.resources.food;
            case 1: return world.resources.timber;
            case 2: return world.resources.iron;
            case 3: return world.resources.gold;
            default: return 0;
        }
    }

    bool dynastyProduces(const World& world, const std::string& dynasty, int resIndex) {
        return world.countOwnedProvinces(dynasty, RES_TYPES[resIndex]) > 0;
    }

    bool canStartTrade(const World& world) {
        if (world.tradeTasks.slots[0].active) {
            return false;
        }
        if (world.pendingTradeDynasty.empty()) {
            return false;
        }
        if (world.pendingTradeOfferRes == world.pendingTradeRequestRes) {
            return false;
        }
        if (playerStock(world, world.pendingTradeOfferRes) < world.pendingTradeQty) {
            return false;
        }
        if (!dynastyProduces(world, world.pendingTradeDynasty, world.pendingTradeRequestRes)) {
            return false;
        }
        if (world.workerPool.availableDiplomaticWorkers < 1) {
            return false;
        }
        return true;
    }

    bool startTrade(World& world) {
        if (!canStartTrade(world)) {
            return false;
        }
        int workers = world.pendingDiplomaticWorkers;

        auto& task = world.tradeTasks.slots[0];
        task.active = true;
        task.offerRes = world.pendingTradeOfferRes;
        task.requestRes = world.pendingTradeRequestRes;
        task.qty = world.pendingTradeQty;
        task.dynasty = world.pendingTradeDynasty;
        task.workersAssigned = workers;
        task.daysAccumulated = 0;
        task.daysRequired = daysPerUnit[std::min(workers, 8) - 1];

        world.workerPool.availableDiplomaticWorkers -= workers;

        switch (task.offerRes) {
            case 0: world.resources.food   -= task.qty; break;
            case 1: world.resources.timber -= task.qty; break;
            case 2: world.resources.iron   -= task.qty; break;
            case 3: world.resources.gold   -= task.qty; break;
        }

        world.pendingTradeQty = 1;
        return true;
    }

    void tick(World& world) {
        auto& task = world.tradeTasks.slots[0];
        if (!task.active) {
            return;
        }

        task.daysAccumulated++;
        if (task.daysAccumulated < task.daysRequired) {
            return;
        }

        switch (task.requestRes) {
            case 0: world.resources.food   += task.qty; break;
            case 1: world.resources.timber += task.qty; break;
            case 2: world.resources.iron   += task.qty; break;
            case 3: world.resources.gold   += task.qty; break;
        }

        world.workerPool.availableDiplomaticWorkers += task.workersAssigned;
        world.ctx.battleMessage = "Trade complete";
        world.ctx.battleMessageTimer = 5;
        task = TradeTask{};
    }

}
