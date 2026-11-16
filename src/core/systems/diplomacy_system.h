#pragma once
#include "../world.h"

namespace DiplomacySystem {

    int diplomaticDays(int workers);
    std::string scoutBlockReason(const World& world, int targetProvinceId, int workers);
    std::string bribeBlockReason(const World& world, int targetProvinceId, int workers, int gold);
    bool canStartBribe(const World& world, int targetProvinceId);
    bool startBribe(World& world, int targetProvinceId, int workers, int gold);
    void cancelBribe(World& world, int slot);

    bool canStartScout(const World& world, int targetProvinceId);
    bool startScout(World& world, int targetProvinceId, int workers);
    void cancelScout(World& world, int slot);

    void tick(World& world);

}
