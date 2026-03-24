#include "game_system.h"
#include "date_system.h"
#include <algorithm>
#include <random>

namespace GameSystem {

    void initArmies(World& world) {
        world.armies["Kantakouzenos"] = {"Kantakouzenos", 4, 2, 0};
        world.armies["Doukas"]        = {"Doukas",        4, 2, 0};
        world.armies["Palaiologos"]   = {"Palaiologos",   4, 2, 0};
        world.armies["Phokas"]        = {"Phokas",        4, 2, 0};
        world.armies["Komnenos"]      = {"Komnenos",      4, 2, 0};
        world.armies["Baldwin II"]    = {"Baldwin II",    3, 4, 2};
        world.armies["neutral"]       = {"neutral",       2, 1, 0};
    }

    void tick(World& world) {
        DateSystem::advance(world.date);
        world.ctx.score++;

        if (world.ctx.battleMessageTimer > 0) {
            world.ctx.battleMessageTimer--;
            if (world.ctx.battleMessageTimer == 0)
                world.ctx.battleMessage = "";
        }
    }

    bool isVictory(const World& world) {
        for (auto& p : world.provinces)
            if (p.name.find("Constantinople") != std::string::npos
                && p.owner == world.ctx.playerDynasty)
                return true;
        return false;
    }

    bool isDefeat(const World& world) {
        return world.isDefeated(world.ctx.playerDynasty);
    }

    void assignStartingProvinces(World& world) {
        const std::vector<std::string> dynasties = {
            "Kantakouzenos", "Doukas", "Palaiologos", "Phokas", "Komnenos"
        };

        // Collect eligible province ids (not Constantinople)
        std::vector<int> eligible;
        for (auto& p : world.provinces)
            if (p.name.find("Constantinople") == std::string::npos)
                eligible.push_back(p.id);

        // Shuffle
        std::random_device rd;
        std::mt19937 g(rd());
        std::shuffle(eligible.begin(), eligible.end(), g);

        // Assign one province per dynasty
        for (int i = 0; i < (int)dynasties.size(); i++)
        world.provinces[eligible[i]].owner = dynasties[i];
    }
}