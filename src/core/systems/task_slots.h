#pragma once
#include "../world.h"

// One task per category initially; score unlocks a second, shared by ALL task
// types in that category. Worker availability is a separate requirement.
namespace TaskSlots {
    int activeCount(const World& world, TaskCategory category);
    bool canStart(const World& world, TaskCategory category, int workers);
    int administrativeCount(const World& world);
    int militaryCount(const World& world);
    int diplomaticCount(const World& world);
    bool administrativeAvailable(const World& world);
    bool militaryAvailable(const World& world);
    bool diplomaticAvailable(const World& world);
    int slotCapacity(const World& world, TaskCategory category);
    void normalizeLoadedTasks(World& world);
}
