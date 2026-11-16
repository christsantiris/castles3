#include "resource_system.h"
#include "task_slots.h"

namespace ResourceSystem {

    int collectionDays(int workers, int ownedProvinces) {
        if (workers < 1 || ownedProvinces < 1) {
            return 0;
        }
        int w = workers;
        if (w > 8) {
            w = 8;
        }
        int p = ownedProvinces;
        if (p > 5) {
            p = 5;
        }
        return daysToCollect[w - 1][p - 1];
    }

    std::string collectBlockReason(const World& world, ResourceType res, int workers) {
        if (workers < 1) {
            return "Assign a worker";
        }
        if (!TaskSlots::administrativeAvailable(world)) {
            return "No stock slot";
        }
        if (workers > world.workerPool.availableStockWorkers) {
            return "Not enough workers";
        }
        if (world.countOwnedProvinces(world.ctx.playerDynasty, res) < 1) {
            return "No province";
        }
        if (freeSlot(world.collectionTasks) < 0) {
            return "No stock slot";
        }
        return "";
    }

    int freeSlot(const CollectionTasksComponent& tasks) {
        for (int i = 0; i < 2; i++)
            if (!tasks.slots[i].active) return i;
        return -1;
    }

    bool startTask(World& world, ResourceType res, int workers) {
        if (!TaskSlots::canStart(world, CollectionTask::category, workers)) {
            return false;
        }
        int slot = freeSlot(world.collectionTasks);
        if (slot == -1) return false;

        int owned = world.countOwnedProvinces(world.ctx.playerDynasty, res);
        if (owned == 0) return false;
        if (workers < 1) return false;
        if (workers > world.workerPool.availableStockWorkers) return false;

        auto& task = world.collectionTasks.slots[slot];
        task.active = true;
        task.resource = res;
        task.workersAssigned = workers;
        task.daysRequired = collectionDays(workers, owned);
        task.daysAccumulated = 0;

        world.workerPool.availableStockWorkers -= workers;
        world.pendingWorkers[(int)res] = 0;
        return true;
    }

    void cancelTask(World& world, int slot) {
        auto& task = world.collectionTasks.slots[slot];
        if (!task.active) return;
        world.workerPool.availableStockWorkers += task.workersAssigned;
        task = CollectionTask{};
    }

    void tick(World& world) {
        for (int i = 0; i < 2; i++) {
            auto& task = world.collectionTasks.slots[i];
            if (!task.active) continue;

            task.daysAccumulated++;
            if (task.daysAccumulated < task.daysRequired) continue;

            switch (task.resource) {
                case ResourceType::Food:   world.resources.food++;   break;
                case ResourceType::Timber: world.resources.timber++; break;
                case ResourceType::Iron:   world.resources.iron++;   break;
                case ResourceType::Gold:   world.resources.gold++;   break;
                default: break;
            }
            world.ctx.score += 10;
            world.workerPool.availableStockWorkers += task.workersAssigned;
            task = CollectionTask{};
        }
    }

}
