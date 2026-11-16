#pragma once
#include "../world.h"

namespace RecruitSystem {

    static const int daysPerUnit[] = {40, 20, 13, 10, 8, 7, 6, 5};

    int recruitmentDays(int workers);
    std::string recruitBlockReason(const World& world, UnitType unit, int workers);
    bool canAfford(const World& world, UnitType unit);
    bool startRecruitment(World& world, UnitType unit, int workers);
    void cancelRecruitment(World& world, int slot);
    void tick(World& world);
}