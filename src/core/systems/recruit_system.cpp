#include "recruit_system.h"
#include "unlock_system.h"
#include "task_slots.h"

namespace RecruitSystem {

    int recruitmentDays(int workers) {
        if (workers < 1) {
            return 0;
        }
        int count = workers;
        if (count > 8) {
            count = 8;
        }
        return daysPerUnit[count - 1];
    }

    bool canAfford(const World& world, UnitType unit) {
        switch (unit) {
            case UnitType::Infantry: return world.resources.iron   >= 3;
            case UnitType::Archers:  return world.resources.timber >= 3;
            case UnitType::Knights:  return world.resources.iron   >= 3
                                         && world.resources.timber >= 3;
        }
        return false;
    }

    bool startRecruitment(World& world, UnitType unit, int workers) {
        if (!TaskSlots::canStart(world, RecruitTask::category, workers)) {
            return false;
        }
        if (!canAfford(world, unit)) return false;
        if (workers < 1) return false;
        if (workers > world.workerPool.availableMilitaryWorkers) return false;
        if (unit == UnitType::Knights && !UnlockSystem::hasKnights(world)) return false;

        // Check free slot
        int slot = -1;
        for (int i = 0; i < 2; i++)
            if (!world.recruitTasks.slots[i].active) { slot = i; break; }
        if (slot == -1) return false;

        // Check unit cap
        auto it = world.armies.find(world.ctx.playerDynasty);
        if (it != world.armies.end()) {
            if (unit == UnitType::Infantry && it->second.infantry >= 15) return false;
            if (unit == UnitType::Archers  && it->second.archers  >= 15) return false;
            if (unit == UnitType::Knights  && it->second.knights  >= 10) return false;
        }

        // Deduct resources
        switch (unit) {
            case UnitType::Infantry: world.resources.iron   -= 3; break;
            case UnitType::Archers:  world.resources.timber -= 3; break;
            case UnitType::Knights:  world.resources.iron   -= 3;
                                     world.resources.timber -= 3; break;
        }

        auto& task = world.recruitTasks.slots[slot];
        task.active = true;
        task.unitType = unit;
        task.workersAssigned = workers;
        task.daysRequired = recruitmentDays(workers);
        task.daysAccumulated = 0;

        world.workerPool.availableMilitaryWorkers -= workers;
        world.pendingMilitaryWorkers = 0;
        return true;
    }

    void cancelRecruitment(World& world, int slot) {
        auto& task = world.recruitTasks.slots[slot];
        if (!task.active) return;

        // Refund resources
        switch (task.unitType) {
            case UnitType::Infantry: world.resources.iron   += 3; break;
            case UnitType::Archers:  world.resources.timber += 3; break;
            case UnitType::Knights:  world.resources.iron   += 3;
                                     world.resources.timber += 3; break;
        }

        world.workerPool.availableMilitaryWorkers += task.workersAssigned;
        task = RecruitTask{};
    }

    void tick(World& world) {
        for (int i = 0; i < 2; i++) {
            auto& task = world.recruitTasks.slots[i];
            if (!task.active) continue;

            task.daysAccumulated++;
            if (task.daysAccumulated < task.daysRequired) continue;

            auto& army = world.armies[world.ctx.playerDynasty];
            switch (task.unitType) {
                case UnitType::Infantry: army.infantry++; break;
                case UnitType::Archers:  army.archers++;  break;
                case UnitType::Knights:  army.knights++;  break;
            }

            world.workerPool.availableMilitaryWorkers += task.workersAssigned;
            task = RecruitTask{};
        }
    }

}
