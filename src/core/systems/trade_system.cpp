#include "trade_system.h"
#include "task_slots.h"

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

    static void adjustStock(World& world, int resIndex, int delta) {
        switch (resIndex) {
            case 0: world.resources.food += delta; break;
            case 1: world.resources.timber += delta; break;
            case 2: world.resources.iron += delta; break;
            case 3: world.resources.gold += delta; break;
        }
    }

    static int freeSlot(const World& world) {
        for (int i = 0; i < 2; i++) {
            if (!world.tradeTasks.slots[i].active) {
                return i;
            }
        }
        return -1;
    }

    bool dynastyProduces(const World& world, const std::string& dynasty, int resIndex) {
        return world.countOwnedProvinces(dynasty, RES_TYPES[resIndex]) > 0;
    }

    bool canStartTrade(const World& world) {
        if (!TaskSlots::canStart(world, TradeTask::category, world.pendingDiplomaticWorkers)) {
            return false;
        }
        if (freeSlot(world) < 0) {
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
        int slot = freeSlot(world);

        auto& task = world.tradeTasks.slots[slot];
        task.active = true;
        task.offerRes = world.pendingTradeOfferRes;
        task.requestRes = world.pendingTradeRequestRes;
        task.qty = world.pendingTradeQty;
        task.dynasty = world.pendingTradeDynasty;
        task.workersAssigned = workers;
        task.daysAccumulated = 0;
        task.daysRequired = daysPerUnit[std::min(workers, 8) - 1];

        world.workerPool.availableDiplomaticWorkers -= workers;
        adjustStock(world, task.offerRes, -task.qty);

        world.pendingTradeQty = 1;
        world.pendingDiplomaticWorkers = 0;
        return true;
    }

    void cancelTrade(World& world, int slot) {
        if (slot < 0 || slot >= 2) {
            return;
        }
        auto& task = world.tradeTasks.slots[slot];
        if (!task.active) {
            return;
        }
        adjustStock(world, task.offerRes, task.qty);
        world.workerPool.availableDiplomaticWorkers += task.workersAssigned;
        task = TradeTask{};
    }

    void tick(World& world) {
        for (int i = 0; i < 2; i++) {
            auto& task = world.tradeTasks.slots[i];
            if (!task.active) {
                continue;
            }

            task.daysAccumulated++;
            if (task.daysAccumulated < task.daysRequired) {
                continue;
            }

            adjustStock(world, task.requestRes, task.qty);
            world.workerPool.availableDiplomaticWorkers += task.workersAssigned;
            world.ctx.battleMessage = "Trade complete";
            world.ctx.battleMessageTimer = 5;
            task = TradeTask{};
        }
    }

}
