#pragma once
#include "../world.h"
#include <string>

namespace TradeSystem {
    const char* resourceName(int index);
    int playerStock(const World& world, int resIndex);
    bool dynastyProduces(const World& world, const std::string& dynasty, int resIndex);
    int tradeDays(int workers);
    bool canStartTrade(const World& world);
    bool startTrade(World& world);
    void cancelTrade(World& world, int slot);
    void tick(World& world);
}
