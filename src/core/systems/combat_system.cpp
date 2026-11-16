#include "combat_system.h"
#include "task_slots.h"
#include <cstdlib>
#include <cmath>

namespace CombatSystem {

    int marchDays(int workers, bool neutral) {
        if (workers < 1) {
            return 0;
        }
        int count = workers;
        if (count > 8) {
            count = 8;
        }
        int days = daysPerUnit[count - 1];
        if (!neutral) {
            days *= 2;
        }
        return days;
    }

    bool isAdjacent(const World& world, int targetProvinceId) {
        for (auto& p : world.provinces) {
            if (p.owner != world.ctx.playerDynasty) continue;
            for (int neighborId : p.neighbors)
                if (neighborId == targetProvinceId) return true;
        }
        return false;
    }

    bool canStartMarch(const World& world, int targetProvinceId) {
        if (!TaskSlots::militaryAvailable(world)) {
            return false;
        }
        auto* target = const_cast<World&>(world).findProvince(targetProvinceId);
        if (!target) return false;
        if (target->owner == world.ctx.playerDynasty) return false;
        if (!isAdjacent(world, targetProvinceId)) return false;
        if (world.battle.phase != BattlePhase::None) return false;

        // Check free combat slot
        for (int i = 0; i < 2; i++)
            if (!world.combatTasks.slots[i].active) return true;
        return false;
    }

    bool startMarch(World& world, int targetProvinceId, int workers) {
        if (!TaskSlots::canStart(world, CombatTask::category, workers)) {
            return false;
        }
        if (!canStartMarch(world, targetProvinceId)) return false;
        if (workers < 1) return false;
        if (workers > world.workerPool.availableMilitaryWorkers) return false;

        const ArmyComponent& army = world.armies[world.ctx.playerDynasty];
        int armySize = army.infantry + army.archers + army.knights;
        int foodCost = (world.ctx.marchFoodFlat > 0)
            ? world.ctx.marchFoodFlat
            : (armySize + 1) / 2;
        if (world.resources.food < foodCost) {
            world.ctx.battleMessage = "Not enough food to march!";
            world.ctx.battleMessageTimer = 5;
            return false;
        }
        world.resources.food -= foodCost;

        auto* target = world.findProvince(targetProvinceId);
        bool isNeutral = target->owner == "neutral";

        int slot = -1;
        for (int i = 0; i < 2; i++)
            if (!world.combatTasks.slots[i].active) { slot = i; break; }

        auto& task = world.combatTasks.slots[slot];
        task.active = true;
        task.targetProvinceId = targetProvinceId;
        task.workersAssigned = workers;
        task.daysAccumulated = 0;
        task.daysRequired = marchDays(workers, isNeutral);

        world.workerPool.availableMilitaryWorkers -= workers;
        world.pendingMilitaryWorkers = 0;
        return true;
    }

    void cancelMarch(World& world, int slot) {
        auto& task = world.combatTasks.slots[slot];
        if (!task.active) return;
        world.workerPool.availableMilitaryWorkers += task.workersAssigned;
        task = CombatTask{};
    }

    void tickMarch(World& world) {
        if (world.battle.phase != BattlePhase::None) {
            return;
        }
        for (int i = 0; i < 2; i++) {
            auto& task = world.combatTasks.slots[i];
            if (!task.active) continue;

            task.daysAccumulated++;
            if (task.daysAccumulated < task.daysRequired) continue;

            // March complete — trigger battle
            int targetId = task.targetProvinceId;
            world.workerPool.availableMilitaryWorkers += task.workersAssigned;
            task = CombatTask{};
            initBattle(world, targetId);
            if (world.battle.phase != BattlePhase::None) {
                return;
            }
        }
    }

    int attackStrength(const ArmyComponent& army) {
        return (army.infantry * 3) + (army.archers * 1) + (army.knights * 4);
    }

    int defenseStrength(const ArmyComponent& army) {
        return (army.infantry * 1) + (army.archers * 3) + (army.knights * 4);
    }

    static void spawnUnits(std::vector<BattleUnit>& units,
                           const ArmyComponent& army, bool isPlayer) {
        units.clear();
        float startX = isPlayer ? 100.0f : 750.0f;
        float y = 200.0f;

        for (int i = 0; i < army.infantry; i++) {
            BattleUnit u;
            u.x = startX;
            u.y = y + (i * 40.0f);
            u.type = 0;
            u.health = 10;
            units.push_back(u);
        }
        for (int i = 0; i < army.archers; i++) {
            BattleUnit u;
            u.x = startX;
            u.y = y + (army.infantry * 40.0f) + (i * 40.0f);
            u.type = 1;
            u.health = 10;
            units.push_back(u);
        }
        for (int i = 0; i < army.knights; i++) {
            BattleUnit u;
            u.x = startX;
            u.y = y + ((army.infantry + army.archers) * 40.0f) + (i * 40.0f);
            u.type = 2;
            u.health = 10;
            units.push_back(u);
        }
    }

    void initBattle(World& world, int targetProvinceId, const std::string& attackingDynasty) {
        auto* target = world.findProvince(targetProvinceId);
        if (!target) return;

        ArmyComponent& playerArmy = world.armies[world.ctx.playerDynasty];
        bool defending = !attackingDynasty.empty();
        const std::string opponent = defending ? attackingDynasty : target->owner;
        ArmyComponent& enemyArmy = world.armies.count(opponent)
            ? world.armies[opponent]
            : world.armies["neutral"];

        world.battle = BattleState{};
        world.battle.phase = BattlePhase::Preparing;
        world.battle.targetProvinceId = targetProvinceId;
        world.battle.targetProvinceName = target->name;
        world.battle.defenderDynasty = defending ? opponent : "";

        int playerStrength = defending ? defenseStrength(playerArmy) : attackStrength(playerArmy);
        int enemyStrength = defending ? attackStrength(enemyArmy) : defenseStrength(enemyArmy);

        world.battle.playerMaxHealth = playerStrength * 10;
        world.battle.aiMaxHealth = enemyStrength * 10;
        world.battle.playerHealth = world.battle.playerMaxHealth;
        world.battle.aiHealth = world.battle.aiMaxHealth;
        world.battle.roundTimer = 0.0f;
        world.battle.playerWon = false;
        world.battle.statusText = std::string(defending ? "Defense: " : "Attack: ") + std::to_string(playerStrength) +
                                  (defending ? " vs Attack: " : " vs Defense: ") + std::to_string(enemyStrength);

        spawnUnits(world.battle.playerUnits, playerArmy, true);
        spawnUnits(world.battle.aiUnits, enemyArmy, false);
    }

    void updateBattle(World& world, float deltaTime) {
        if (world.battle.phase != BattlePhase::Running) return;

        for (auto& u : world.battle.playerUnits) {
            if (!u.alive) continue;
            if (u.x < 400.0f) u.x += 60.0f * deltaTime;
        }
        for (auto& u : world.battle.aiUnits) {
            if (!u.alive) continue;
            if (u.x > 550.0f) u.x -= 60.0f * deltaTime;
        }

        world.battle.roundTimer += deltaTime;
        if (world.battle.roundTimer < world.battle.roundInterval) return;
        world.battle.roundTimer = 0.0f;

        auto* target = world.findProvince(world.battle.targetProvinceId);
        if (!target) {
            return;
        }
        bool defending = !world.battle.defenderDynasty.empty();
        const std::string opponent = defending ? world.battle.defenderDynasty : target->owner;
        ArmyComponent& playerArmy = world.armies[world.ctx.playerDynasty];
        ArmyComponent& enemyArmy = world.armies.count(opponent)
            ? world.armies[opponent]
            : world.armies["neutral"];

        int playerRoll = (defending ? defenseStrength(playerArmy) : attackStrength(playerArmy)) + (rand() % 10);
        int enemyRoll = (defending ? attackStrength(enemyArmy) : defenseStrength(enemyArmy)) + (rand() % 10);

        int damage = std::max(1, playerRoll - enemyRoll / 2);
        int defDamage = std::max(1, enemyRoll - playerRoll / 2);

        world.battle.aiHealth -= damage;
        world.battle.playerHealth -= defDamage;

        auto killUnits = [](std::vector<BattleUnit>& units, int maxHealth, int currentHealth) {
            int total = units.size();
            if (total == 0) return;
            int shouldBeAlive = (int)std::ceil((float)currentHealth / maxHealth * total);
            shouldBeAlive = std::max(0, shouldBeAlive);
            int alive = 0;
            for (auto& u : units) if (u.alive) alive++;
            while (alive > shouldBeAlive) {
                for (auto& u : units) {
                    if (u.alive) { u.alive = false; alive--; break; }
                }
            }
        };

        killUnits(world.battle.aiUnits, world.battle.aiMaxHealth, world.battle.aiHealth);
        killUnits(world.battle.playerUnits, world.battle.playerMaxHealth, world.battle.playerHealth);

        if (world.battle.aiHealth <= 0 || world.battle.playerHealth <= 0) {
            world.battle.playerWon = world.battle.aiHealth <= 0;
            world.battle.phase = BattlePhase::Resolved;

            ArmyComponent playerBefore = playerArmy;
            ArmyComponent enemyBefore = enemyArmy;
            auto applyLosses = [](ArmyComponent& army) {
                army.infantry = army.infantry / 2;
                army.archers = army.archers / 2;
                army.knights = army.knights / 2;
            };

            if (world.battle.playerWon) {
                if (opponent != "neutral" && world.armies.count(opponent)) {
                    applyLosses(enemyArmy);
                }
            } else {
                applyLosses(world.armies[world.ctx.playerDynasty]);
            }

            if (world.battle.playerWon && opponent != "neutral" && world.armies.count(opponent)) {
                world.battle.casualtySide = opponent;
                world.battle.lostInfantry = enemyBefore.infantry - enemyArmy.infantry;
                world.battle.lostArchers = enemyBefore.archers - enemyArmy.archers;
                world.battle.lostKnights = enemyBefore.knights - enemyArmy.knights;
            } else if (!world.battle.playerWon) {
                world.battle.casualtySide = "You";
                world.battle.lostInfantry = playerBefore.infantry - playerArmy.infantry;
                world.battle.lostArchers = playerBefore.archers - playerArmy.archers;
                world.battle.lostKnights = playerBefore.knights - playerArmy.knights;
            }
            world.battle.remainPlayerInfantry = playerArmy.infantry;
            world.battle.remainPlayerArchers = playerArmy.archers;
            world.battle.remainPlayerKnights = playerArmy.knights;
            world.battle.remainEnemyInfantry = enemyArmy.infantry;
            world.battle.remainEnemyArchers = enemyArmy.archers;
            world.battle.remainEnemyKnights = enemyArmy.knights;
            world.battle.enemyName = opponent;

            if (world.battle.playerWon) {
                if (!defending) {
                    target->owner = world.ctx.playerDynasty;
                }
                world.battle.ownershipText = target->name + (defending ? " defended" : " conquered");
                world.ctx.score += 150;
                world.battle.scoreReward = 150;
                world.ctx.battleMessage = "Victory! " + world.battle.ownershipText;
                world.scoutedProvinces.insert(world.battle.targetProvinceId);
                if (!defending && target->name == "Constantinople") {
                    world.ctx.screen = GameScreen::Victory;
                }
            } else {
                if (!world.battle.defenderDynasty.empty()) {
                    target->owner = world.battle.defenderDynasty;
                    world.battle.ownershipText = target->name + " lost";
                    world.ctx.battleMessage = "Defeated! " + world.battle.ownershipText + "!";
                    if (world.isDefeated(world.ctx.playerDynasty)) {
                        world.ctx.screen = GameScreen::Defeat;
                    }
                } else {
                    world.battle.ownershipText = target->name + " held";
                    world.ctx.battleMessage = "Defeated! " + world.battle.ownershipText;
                }
                world.scoutedProvinces.insert(world.battle.targetProvinceId);
            }
            world.ctx.battleMessageTimer = 5;
            world.battle.statusText = world.battle.playerWon ? "Victory!" : "Defeated!";
        }
    }

    std::vector<std::string> battleResultLines(const BattleState& battle) {
        std::vector<std::string> lines;
        if (battle.phase != BattlePhase::Resolved || battle.ownershipText.empty()) {
            return lines;
        }
        lines.push_back(battle.playerWon ? "Victory" : "Defeated");
        lines.push_back(battle.ownershipText);
        if (battle.scoreReward > 0) {
            lines.push_back("Reward: " + std::to_string(battle.scoreReward) + " score");
        } else {
            lines.push_back("No reward");
        }
        if (battle.lostInfantry == 0 && battle.lostArchers == 0 && battle.lostKnights == 0) {
            lines.push_back("No casualties");
        } else {
            lines.push_back(battle.casualtySide + " lost INF " + std::to_string(battle.lostInfantry) +
                            " ARC " + std::to_string(battle.lostArchers) +
                            " KNT " + std::to_string(battle.lostKnights));
        }
        lines.push_back("Your army INF " + std::to_string(battle.remainPlayerInfantry) +
                        " ARC " + std::to_string(battle.remainPlayerArchers) +
                        " KNT " + std::to_string(battle.remainPlayerKnights));
        lines.push_back(battle.enemyName + " army INF " + std::to_string(battle.remainEnemyInfantry) +
                        " ARC " + std::to_string(battle.remainEnemyArchers) +
                        " KNT " + std::to_string(battle.remainEnemyKnights));
        return lines;
    }

    void retreat(World& world) {
        world.battle = BattleState{};
        world.ctx.battleMessage = "Retreated!";
        world.ctx.battleMessageTimer = 3;
    }

    void pauseBattle(World& world) {
        if (world.battle.phase == BattlePhase::Running)
            world.battle.phase = BattlePhase::Paused;
    }

    void resumeBattle(World& world) {
        if (world.battle.phase == BattlePhase::Paused)
            world.battle.phase = BattlePhase::Running;
    }

}
