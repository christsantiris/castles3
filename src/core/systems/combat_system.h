#pragma once
#include "../world.h"
#include <string>
#include <vector>

namespace CombatSystem {

    static const int daysPerUnit[] = {40, 20, 13, 10, 8, 7, 6, 5};

    int marchDays(int workers, bool neutral);
    int marchFoodCost(const World& world);
    std::string marchBlockReason(const World& world, int targetProvinceId, int workers);
    bool isAdjacent(const World& world, int targetProvinceId);
    bool canStartMarch(const World& world, int targetProvinceId);
    bool startMarch(World& world, int targetProvinceId, int workers);
    void cancelMarch(World& world, int slot);
    void tickMarch(World& world);
    void initBattle(World& world, int targetProvinceId, const std::string& attackingDynasty = "");
    void updateBattle(World& world, float deltaTime);
    void retreat(World& world);
    void pauseBattle(World& world);
    void resumeBattle(World& world);
    std::vector<std::string> battleResultLines(const BattleState& battle);
    int attackStrength(const ArmyComponent& army);
    int defenseStrength(const ArmyComponent& army);
}
