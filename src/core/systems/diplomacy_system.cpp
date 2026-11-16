#include "diplomacy_system.h"
#include "combat_system.h"
#include "task_slots.h"
#include <cstdlib>
#include <algorithm>

namespace DiplomacySystem {

    static const int daysPerUnit[] = {20, 10, 7, 5, 4, 3, 3, 2};

    int diplomaticDays(int workers) {
        if (workers < 1) {
            return 0;
        }
        int count = workers;
        if (count > 8) {
            count = 8;
        }
        return daysPerUnit[count - 1];
    }

    std::string scoutBlockReason(const World& world, int targetProvinceId, int workers) {
        if (workers < 1) {
            return "Assign a worker";
        }
        if (!TaskSlots::diplomaticAvailable(world)) {
            return "No diplomatic slot";
        }
        if (workers > world.workerPool.availableDiplomaticWorkers) {
            return "Not enough workers";
        }
        if (!canStartScout(world, targetProvinceId)) {
            auto* target = const_cast<World&>(world).findProvince(targetProvinceId);
            if (!target) {
                return "No province";
            }
            if (target->owner == world.ctx.playerDynasty) {
                return "Own province";
            }
            if (world.scoutedProvinces.count(targetProvinceId)) {
                return "Already scouted";
            }
            if (!CombatSystem::isAdjacent(world, targetProvinceId)) {
                return "Not adjacent";
            }
            return "No diplomatic slot";
        }
        return "";
    }

    std::string bribeBlockReason(const World& world, int targetProvinceId, int workers, int gold) {
        if (workers < 1) {
            return "Assign a worker";
        }
        if (!TaskSlots::diplomaticAvailable(world)) {
            return "No diplomatic slot";
        }
        if (workers > world.workerPool.availableDiplomaticWorkers) {
            return "Not enough workers";
        }
        if (gold < 1) {
            return "Assign gold";
        }
        if (gold > world.resources.gold) {
            return "Need " + std::to_string(gold) + " gold";
        }
        if (!canStartBribe(world, targetProvinceId)) {
            auto* target = const_cast<World&>(world).findProvince(targetProvinceId);
            if (!target) {
                return "No province";
            }
            if (target->owner == world.ctx.playerDynasty) {
                return "Own province";
            }
            if (target->name == "Constantinople") {
                return "Constantinople";
            }
            if (!CombatSystem::isAdjacent(world, targetProvinceId)) {
                return "Not adjacent";
            }
            return "No diplomatic slot";
        }
        return "";
    }

    bool canStartBribe(const World& world, int targetProvinceId) {
        if (!TaskSlots::diplomaticAvailable(world)) {
            return false;
        }
        auto* target = const_cast<World&>(world).findProvince(targetProvinceId);
        if (!target) return false;
        if (target->owner == world.ctx.playerDynasty) return false;
        if (target->name == "Constantinople") return false;
        if (!CombatSystem::isAdjacent(world, targetProvinceId)) return false;

        for (int i = 0; i < 2; i++) {
            if (!world.bribeTasks.slots[i].active) return true;
        }
        return false;
    }

    bool startBribe(World& world, int targetProvinceId, int workers, int gold) {
        if (!TaskSlots::canStart(world, BribeTask::category, workers)) {
            return false;
        }
        if (!canStartBribe(world, targetProvinceId)) return false;
        if (workers < 1) return false;
        if (workers > world.workerPool.availableDiplomaticWorkers) return false;
        if (gold < 1) return false;
        if (gold > world.resources.gold) return false;

        int slot = -1;
        for (int i = 0; i < 2; i++) {
            if (!world.bribeTasks.slots[i].active) { slot = i; break; }
        }

        auto& task = world.bribeTasks.slots[slot];
        task.active = true;
        task.targetProvinceId = targetProvinceId;
        task.workersAssigned = workers;
        task.goldAmount = gold;
        task.daysAccumulated = 0;
        task.daysRequired = diplomaticDays(workers);

        world.resources.gold -= gold;
        world.workerPool.availableDiplomaticWorkers -= workers;
        world.pendingDiplomaticWorkers = 0;
        return true;
    }

    void cancelBribe(World& world, int slot) {
        auto& task = world.bribeTasks.slots[slot];
        if (!task.active) return;
        world.workerPool.availableDiplomaticWorkers += task.workersAssigned;
        task = BribeTask{};
    }

    static int bribeChance(int gold, bool isNeutral) {
        if (isNeutral) {
            return std::min(95, 35 + gold * 5);
        } else {
            return std::min(75, 20 + gold * 3);
        }
    }

    bool canStartScout(const World& world, int targetProvinceId) {
        if (!TaskSlots::diplomaticAvailable(world)) {
            return false;
        }
        auto* target = const_cast<World&>(world).findProvince(targetProvinceId);
        if (!target) return false;
        if (target->owner == world.ctx.playerDynasty) return false;
        if (world.scoutedProvinces.count(targetProvinceId)) return false;
        if (!CombatSystem::isAdjacent(world, targetProvinceId)) return false;

        for (int i = 0; i < 2; i++) {
            if (!world.scoutTasks.slots[i].active) return true;
        }
        return false;
    }

    bool startScout(World& world, int targetProvinceId, int workers) {
        if (!TaskSlots::canStart(world, ScoutTask::category, workers)) {
            return false;
        }
        if (!canStartScout(world, targetProvinceId)) return false;
        if (workers < 1) return false;
        if (workers > world.workerPool.availableDiplomaticWorkers) return false;

        int slot = -1;
        for (int i = 0; i < 2; i++) {
            if (!world.scoutTasks.slots[i].active) { slot = i; break; }
        }

        auto& task = world.scoutTasks.slots[slot];
        task.active = true;
        task.targetProvinceId = targetProvinceId;
        task.workersAssigned = workers;
        task.daysAccumulated = 0;
        task.daysRequired = diplomaticDays(workers);

        world.workerPool.availableDiplomaticWorkers -= workers;
        world.pendingDiplomaticWorkers = 0;
        return true;
    }

    void cancelScout(World& world, int slot) {
        auto& task = world.scoutTasks.slots[slot];
        if (!task.active) return;
        world.workerPool.availableDiplomaticWorkers += task.workersAssigned;
        task = ScoutTask{};
    }

    static void tickScout(World& world) {
        for (int i = 0; i < 2; i++) {
            auto& task = world.scoutTasks.slots[i];
            if (!task.active) continue;

            task.daysAccumulated++;
            if (task.daysAccumulated < task.daysRequired) continue;

            auto* target = world.findProvince(task.targetProvinceId);
            world.workerPool.availableDiplomaticWorkers += task.workersAssigned;
            task = ScoutTask{};

            if (!target) continue;

            world.scoutedProvinces.insert(target->id);
            world.ctx.battleMessage = target->name + " has been scouted";
            world.ctx.battleMessageTimer = 5;
        }
    }

    void tickBribe(World& world) {
        for (int i = 0; i < 2; i++) {
            auto& task = world.bribeTasks.slots[i];
            if (!task.active) continue;

            task.daysAccumulated++;
            if (task.daysAccumulated < task.daysRequired) continue;

            auto* target = world.findProvince(task.targetProvinceId);
            int gold = task.goldAmount;
            world.workerPool.availableDiplomaticWorkers += task.workersAssigned;
            task = BribeTask{};

            if (!target) continue;

            bool isNeutral = target->owner == "neutral";
            int chance = bribeChance(gold, isNeutral);
            int roll = rand() % 100;

            if (roll < chance) {
                target->owner = world.ctx.playerDynasty;
                world.scoutedProvinces.insert(target->id);
                world.ctx.score += 50;
                world.ctx.battleMessage = "Bribe accepted! " + target->name + " joins your dynasty";
            } else {
                world.ctx.battleMessage = "Bribe rejected by " + target->name;
            }
            world.ctx.battleMessageTimer = 5;
        }
    }

    void tick(World& world) {
        tickScout(world);
        tickBribe(world);
    }

}
