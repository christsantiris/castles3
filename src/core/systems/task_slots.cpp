#include "task_slots.h"
#include "unlock_system.h"
#include "resource_system.h"
#include "combat_system.h"
#include "recruit_system.h"
#include "diplomacy_system.h"
#include "trade_system.h"

namespace TaskSlots {
    int activeCount(const World& world, TaskCategory category) {
        int count = 0;
        for (const auto& entry : world.tasks()) {
            if (entry.category == category && entry.task->active) {
                count++;
            }
        }
        return count;
    }

    int administrativeCount(const World& world) {
        return activeCount(world, TaskCategory::Administrative);
    }

    int militaryCount(const World& world) {
        return activeCount(world, TaskCategory::Military);
    }

    int diplomaticCount(const World& world) {
        return activeCount(world, TaskCategory::Diplomatic);
    }

    bool canStart(const World& world, TaskCategory category, int workers) {
        if (workers < 1) {
            return false;
        }
        switch (category) {
            case TaskCategory::Administrative:
                return administrativeAvailable(world) && workers <= world.workerPool.availableStockWorkers;
            case TaskCategory::Military:
                return militaryAvailable(world) && workers <= world.workerPool.availableMilitaryWorkers;
            case TaskCategory::Diplomatic:
                return diplomaticAvailable(world) && workers <= world.workerPool.availableDiplomaticWorkers;
        }
        return false;
    }

    bool administrativeAvailable(const World& world) {
        return administrativeCount(world) < (UnlockSystem::hasSecondStockSlot(world) ? 2 : 1);
    }

    bool militaryAvailable(const World& world) {
        return militaryCount(world) < (UnlockSystem::hasSecondMilitarySlot(world) ? 2 : 1);
    }

    bool diplomaticAvailable(const World& world) {
        return diplomaticCount(world) < (UnlockSystem::hasSecondDiplomacySlot(world) ? 2 : 1);
    }

    int slotCapacity(const World& world, TaskCategory category) {
        switch (category) {
            case TaskCategory::Administrative:
                return UnlockSystem::hasSecondStockSlot(world) ? 2 : 1;
            case TaskCategory::Military:
                return UnlockSystem::hasSecondMilitarySlot(world) ? 2 : 1;
            case TaskCategory::Diplomatic:
                return UnlockSystem::hasSecondDiplomacySlot(world) ? 2 : 1;
        }
        return 1;
    }

    void normalizeLoadedTasks(World& world) {
        // Repair saves made with the old independent-array bug. Keep tasks in
        // panel order and cancel overflow using the normal worker/refund rules.
        auto trim = [](auto& tasks, int& remaining, auto cancel) {
            for (int i = 0; i < 2; i++) {
                if (tasks[i].active) {
                    if (tasks[i].workersAssigned < 1) {
                        cancel(i);
                    } else if (remaining > 0) {
                        remaining--;
                    } else {
                        cancel(i);
                    }
                }
            }
        };
        int administrative = UnlockSystem::hasSecondStockSlot(world) ? 2 : 1;
        trim(world.collectionTasks.slots, administrative, [&](int i) { ResourceSystem::cancelTask(world, i); });
        int military = UnlockSystem::hasSecondMilitarySlot(world) ? 2 : 1;
        trim(world.combatTasks.slots, military, [&](int i) { CombatSystem::cancelMarch(world, i); });
        trim(world.recruitTasks.slots, military, [&](int i) { RecruitSystem::cancelRecruitment(world, i); });
        int diplomatic = UnlockSystem::hasSecondDiplomacySlot(world) ? 2 : 1;
        trim(world.bribeTasks.slots, diplomatic, [&](int i) { DiplomacySystem::cancelBribe(world, i); });
        trim(world.scoutTasks.slots, diplomatic, [&](int i) { DiplomacySystem::cancelScout(world, i); });
        trim(world.tradeTasks.slots, diplomatic, [&](int i) { TradeSystem::cancelTrade(world, i); });
    }
}
