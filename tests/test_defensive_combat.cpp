#include <catch2/catch_test_macros.hpp>
#include "core/systems/ai_system.h"
#include "core/systems/combat_system.h"
#include "core/systems/save_load.h"
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>

static World defensiveWorld() {
    World w;
    w.ctx.playerDynasty = "Player";
    w.ctx.screen = GameScreen::Playing;
    w.armies["Player"] = {"Player", 4, 6, 2};
    w.armies["Enemy"] = {"Enemy", 8, 2, 3};
    w.armies["neutral"] = {"neutral", 1, 1, 0};
    ProvinceComponent home;
    home.id = 0;
    home.name = "Home";
    home.owner = "Player";
    ProvinceComponent enemy = home;
    enemy.id = 1;
    enemy.name = "Enemy land";
    enemy.owner = "Enemy";
    w.provinces = {home, enemy};
    AIMarchTask march;
    march.active = true;
    march.dynasty = "Enemy";
    march.targetProvinceId = 0;
    march.daysRequired = 1;
    w.aiMarches.push_back(march);
    AISystem::tickMarches(w);
    return w;
}

static void playRound(World& w, unsigned seed = 37) {
    w.battle.phase = BattlePhase::Running;
    std::srand(seed);
    CombatSystem::updateBattle(w, 1.0f);
}

TEST_CASE("AI arrival uses attacker strength and player defense including knights", "[combat][defense]") {
    auto w = defensiveWorld();
    REQUIRE(w.battle.phase == BattlePhase::Preparing);
    REQUIRE(w.battle.defenderDynasty == "Enemy");
    REQUIRE(w.battle.playerMaxHealth == CombatSystem::defenseStrength(w.armies.at("Player")) * 10);
    REQUIRE(w.battle.aiMaxHealth == CombatSystem::attackStrength(w.armies.at("Enemy")) * 10);
    REQUIRE(w.battle.playerUnits.size() == 12);
    REQUIRE(w.battle.aiUnits.size() == 13);
    REQUIRE(w.battle.aiUnits.back().type == 2);
}

TEST_CASE("defense round damage depends on the attacking AI army", "[combat][defense]") {
    auto weak = defensiveWorld();
    auto strong = weak;
    weak.armies["Enemy"].infantry = 1;
    strong.armies["Enemy"].infantry = 100;
    playRound(weak);
    playRound(strong);
    REQUIRE(strong.battle.playerHealth < weak.battle.playerHealth);
}

TEST_CASE("defending archers use defense strength rather than attack strength", "[combat][defense]") {
    auto w = defensiveWorld();
    w.armies["Player"] = {"Player", 0, 10, 0};
    w.armies["Enemy"] = {"Enemy", 10, 0, 0};
    std::srand(19);
    int playerRoll = 30 + std::rand() % 10;
    int enemyRoll = 30 + std::rand() % 10;
    int oldPlayer = w.battle.playerHealth;
    int oldEnemy = w.battle.aiHealth;
    playRound(w, 19);
    REQUIRE(w.battle.playerHealth == oldPlayer - std::max(1, enemyRoll - playerRoll / 2));
    REQUIRE(w.battle.aiHealth == oldEnemy - std::max(1, playerRoll - enemyRoll / 2));
}

TEST_CASE("successful defense applies casualties to enemy and retains province", "[combat][defense]") {
    auto w = defensiveWorld();
    w.battle.aiHealth = 1;
    w.battle.playerHealth = w.battle.playerMaxHealth = 10000;
    playRound(w);
    REQUIRE(w.battle.phase == BattlePhase::Resolved);
    REQUIRE(w.battle.playerWon);
    REQUIRE(w.armies["Player"].infantry == 4);
    REQUIRE(w.armies["Player"].archers == 6);
    REQUIRE(w.armies["Enemy"].infantry == 4);
    REQUIRE(w.armies["Enemy"].archers == 1);
    REQUIRE(w.armies["Enemy"].knights == 1);
    REQUIRE(w.provinces[0].owner == "Player");
    REQUIRE(w.ctx.battleMessage.find("defended") != std::string::npos);
    REQUIRE(w.ctx.score == 150);
    auto lines = CombatSystem::battleResultLines(w.battle);
    REQUIRE(lines.size() == 6);
    REQUIRE(lines[0] == "Victory");
    REQUIRE(lines[1] == "Home defended");
    REQUIRE(lines[2] == "Reward: 150 score");
    REQUIRE(lines[3] == "Enemy lost INF 4 ARC 1 KNT 2");
    REQUIRE(lines[4] == "Your army INF 4 ARC 6 KNT 2");
    REQUIRE(lines[5] == "Enemy army INF 4 ARC 1 KNT 1");
}

TEST_CASE("failed defense applies casualties to player and transfers province to attacker", "[combat][defense]") {
    auto w = defensiveWorld();
    w.battle.playerHealth = 1;
    w.battle.aiHealth = w.battle.aiMaxHealth = 10000;
    playRound(w);
    REQUIRE_FALSE(w.battle.playerWon);
    REQUIRE(w.battle.phase == BattlePhase::Resolved);
    REQUIRE(w.provinces[0].owner == "Enemy");
    REQUIRE(w.armies["Player"].infantry == 2);
    REQUIRE(w.armies["Enemy"].infantry == 8);
    REQUIRE(w.ctx.screen == GameScreen::Defeat);
    REQUIRE(w.ctx.score == 0);
    auto lines = CombatSystem::battleResultLines(w.battle);
    REQUIRE(lines[0] == "Defeated");
    REQUIRE(lines[1] == "Home lost");
    REQUIRE(lines[2] == "No reward");
    REQUIRE(lines[3] == "You lost INF 2 ARC 3 KNT 1");
    REQUIRE(lines[4] == "Your army INF 2 ARC 3 KNT 1");
    REQUIRE(lines[5] == "Enemy army INF 8 ARC 2 KNT 3");
}

TEST_CASE("new offensive battle clears the prior defensive role", "[combat][defense]") {
    auto w = defensiveWorld();
    CombatSystem::initBattle(w, 1);
    REQUIRE(w.battle.defenderDynasty.empty());
    REQUIRE(w.battle.targetProvinceName == "Enemy land");
    REQUIRE(w.battle.playerMaxHealth == CombatSystem::attackStrength(w.armies["Player"]) * 10);
    REQUIRE(w.battle.aiMaxHealth == CombatSystem::defenseStrength(w.armies["Enemy"]) * 10);
    w.battle.aiHealth = 1;
    w.battle.playerHealth = w.battle.playerMaxHealth = 10000;
    playRound(w);
    REQUIRE(w.provinces[1].owner == "Player");
    REQUIRE(w.armies["Enemy"].infantry == 4);
}

TEST_CASE("saved defensive battles retain their actual opponent and combat roles", "[combat][defense][save]") {
    auto original = defensiveWorld();
    original.battle.phase = BattlePhase::Paused;
    auto path = std::filesystem::temp_directory_path() / "castles3-defense-test.json";
    SaveLoad::save_game(original, path.string());
    auto restored = defensiveWorld();
    bool loaded = SaveLoad::load_game(restored, path.string());
    std::filesystem::remove(path);
    REQUIRE(loaded);
    REQUIRE(restored.battle.phase == BattlePhase::Paused);
    REQUIRE(restored.battle.defenderDynasty == "Enemy");
    REQUIRE(restored.battle.aiUnits.size() == original.battle.aiUnits.size());
    playRound(original);
    playRound(restored);
    REQUIRE(restored.battle.playerHealth == original.battle.playerHealth);
    REQUIRE(restored.battle.aiHealth == original.battle.aiHealth);
}

TEST_CASE("a resolved battle keeps its result through save and load", "[combat][defense][save]") {
    auto w = defensiveWorld();
    w.battle.aiHealth = 1;
    w.battle.playerHealth = w.battle.playerMaxHealth = 10000;
    playRound(w);
    auto expected = CombatSystem::battleResultLines(w.battle);
    auto path = std::filesystem::temp_directory_path() / "castles3-battle-result-test.json";
    SaveLoad::save_game(w, path.string());
    auto restored = defensiveWorld();
    REQUIRE(SaveLoad::load_game(restored, path.string()));
    REQUIRE(CombatSystem::battleResultLines(restored.battle) == expected);

    std::ifstream in(path);
    nlohmann::json saved;
    in >> saved;
    in.close();
    saved["battle"].erase("ownershipText");
    saved["battle"].erase("scoreReward");
    saved["battle"].erase("casualtySide");
    saved["battle"].erase("lostInfantry");
    saved["battle"].erase("lostArchers");
    saved["battle"].erase("lostKnights");
    saved["battle"].erase("remainPlayerInfantry");
    saved["battle"].erase("remainPlayerArchers");
    saved["battle"].erase("remainPlayerKnights");
    saved["battle"].erase("remainEnemyInfantry");
    saved["battle"].erase("remainEnemyArchers");
    saved["battle"].erase("remainEnemyKnights");
    saved["battle"].erase("enemyName");
    std::ofstream out(path);
    out << saved.dump();
    out.close();
    auto legacy = defensiveWorld();
    REQUIRE(SaveLoad::load_game(legacy, path.string()));
    std::filesystem::remove(path);
    REQUIRE(CombatSystem::battleResultLines(legacy.battle).empty());
    REQUIRE(legacy.battle.statusText == "Victory!");
}
