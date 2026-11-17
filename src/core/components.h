#pragma once
#include <string>
#include <vector>

// ── Entity ID ─────────────────────────────────────────────────────────────────
using EntityID = int;

// ── Province ──────────────────────────────────────────────────────────────────
struct Point {
    int x, y;
};

struct ProvinceComponent {
    int         id;
    std::string name;
    std::string owner;
    std::string resource;
    std::vector<Point> polygon;
    // Static geography loaded from map.json, including when restoring a save.
    std::vector<std::vector<Point>> islands;
    Point labelPosition = {0, 0};
    bool hasLabelPosition = false;
    std::vector<int>   neighbors;
    bool        isSelected = false;
    bool        isRevealed = true;
};

// ── Army ──────────────────────────────────────────────────────────────────────
struct ArmyComponent {
    std::string owner;
    int infantry = 0;
    int archers  = 0;
    int knights  = 0;
};

// ── Resources ─────────────────────────────────────────────────────────────────
enum class ResourceType { Food, Timber, Iron, Gold, None };

struct ResourceStockComponent {
    int food   = 5;
    int timber = 5;
    int iron   = 5;
    int gold   = 5;
#ifdef DEBUG
    gold       = 15;
#endif
};

// ── Worker Pools ──────────────────────────────────────────────────────────────
struct WorkerPoolComponent {
    int totalStockWorkers          = 4;
    int availableStockWorkers      = 4;
    int totalMilitaryWorkers       = 4;
    int availableMilitaryWorkers   = 4;
    int totalDiplomaticWorkers     = 4;
    int availableDiplomaticWorkers = 4;
};

// ── Task Base ─────────────────────────────────────────────────────────────────
enum class TaskCategory { Administrative, Military, Diplomatic };

struct TaskBase {
    bool active          = false;
    int  workersAssigned = 0;
    int  daysRequired    = 0;
    int  daysAccumulated = 0;

    float progress() const {
        if (daysRequired == 0) return 0.0f;
        return (float)daysAccumulated / daysRequired;
    }
};

// ── Collection Tasks ──────────────────────────────────────────────────────────
struct CollectionTask : TaskBase {
    static constexpr const char* label = "Collecting...";
    static constexpr TaskCategory category = TaskCategory::Administrative;
    ResourceType resource = ResourceType::None;
};

struct CollectionTasksComponent {
    CollectionTask slots[2];
};

// ── Combat Tasks ──────────────────────────────────────────────────────────────
struct CombatTask : TaskBase {
    static constexpr const char* label = "Marching...";
    static constexpr TaskCategory category = TaskCategory::Military;
    int targetProvinceId = -1;
};

struct CombatTasksComponent {
    CombatTask slots[2];
};

// ── Recruit Tasks ─────────────────────────────────────────────────────────────
enum class UnitType { Infantry, Archers, Knights };

struct RecruitTask : TaskBase {
    static constexpr const char* label = "Recruiting...";
    static constexpr TaskCategory category = TaskCategory::Military;
    UnitType unitType = UnitType::Infantry;
};

struct RecruitTasksComponent {
    RecruitTask slots[2];
};

// ── Bribe Tasks ───────────────────────────────────────────────────────────────
struct BribeTask : TaskBase {
    static constexpr const char* label = "Bribing...";
    static constexpr TaskCategory category = TaskCategory::Diplomatic;
    int targetProvinceId = -1;
    int goldAmount = 5;
};

struct BribeTasksComponent {
    BribeTask slots[2];
};

// ── Scout Tasks ───────────────────────────────────────────────────────────────
struct ScoutTask : TaskBase {
    static constexpr const char* label = "Scouting...";
    static constexpr TaskCategory category = TaskCategory::Diplomatic;
    int targetProvinceId = -1;
};

struct ScoutTasksComponent {
    ScoutTask slots[2];
};

// ── Trade Tasks ───────────────────────────────────────────────────────────────
struct TradeTask : TaskBase {
    static constexpr const char* label = "Trading...";
    static constexpr TaskCategory category = TaskCategory::Diplomatic;
    int         offerRes   = 0;
    int         requestRes = 0;
    int         qty        = 0;
    std::string dynasty    = "";
};

struct TradeTasksComponent {
    TradeTask slots[2];
};

// ── Date ──────────────────────────────────────────────────────────────────────
struct DateComponent {
    int day   = 2;
    int month = 5;
    int year  = 1312;
};

// ── Game Context ──────────────────────────────────────────────────────────────
enum class GameScreen { Landing, DynastySelect, Playing, Victory, Defeat };

struct GameContext {
    GameScreen  screen             = GameScreen::Landing;
    std::string playerDynasty      = "";
    int         score              = 0;
    int         dayCounter         = 0;
    int         activeTab          = -1;
    std::string battleMessage      = "";
    int         battleMessageTimer = 0;
    bool        victoryRecorded    = false;
    bool        shouldQuit         = false;
    int         upkeepInterval     = 90;
    int         upkeepCounter      = 0;
    bool        upkeepDue          = false;
    bool        paused             = false;
    int         marchFoodFlat      = 1;
};

enum class BattlePhase { None, Preparing, Running, Paused, Resolved };

struct BattleUnit {
    float x = 0.0f;
    float y = 0.0f;
    int health = 10;
    bool alive = true;
    int type = 0; // 0=infantry, 1=archer, 2=knight
};

struct BattleState {
    BattlePhase phase = BattlePhase::None;
    int targetProvinceId = -1;
    std::string targetProvinceName = "";
    // Legacy save field: attacking AI dynasty when the player defends;
    // empty when the player attacks. Retained for save compatibility.
    std::string defenderDynasty = "";
    std::vector<BattleUnit> playerUnits;
    std::vector<BattleUnit> aiUnits;
    int playerHealth = 100;
    int aiHealth = 100;
    int playerMaxHealth = 100;
    int aiMaxHealth = 100;
    float roundTimer = 0.0f;
    float roundInterval = 1.0f;
    float transitionTimer = 0.0f;
    bool playerWon = false;
    std::string statusText = "";
    // Filled when the battle resolves, so the result screen matches the
    // casualties, ownership change, reward, and armies actually applied.
    std::string ownershipText = "";
    int scoreReward = 0;
    std::string casualtySide = "";
    int lostInfantry = 0;
    int lostArchers = 0;
    int lostKnights = 0;
    int remainPlayerInfantry = 0;
    int remainPlayerArchers = 0;
    int remainPlayerKnights = 0;
    int remainEnemyInfantry = 0;
    int remainEnemyArchers = 0;
    int remainEnemyKnights = 0;
    std::string enemyName = "";
};

struct HallOfFameEntry {
    std::string dynasty;
    int score;
    std::string date;
};

struct HallOfFame {
    std::vector<HallOfFameEntry> entries;
};

struct AIConfig {
    int tickInterval    = 15;
    int recruitInterval = 30;
    int stockWorkers    = 1;
    int militaryWorkers = 1;
    int dipWorkers      = 1;
    int startInfantry   = 2;
    int startArchers    = 1;
    int startKnights    = 0;
};

struct AIState {
    int dayCounter     = 0;
    int recruitCounter = 0;
};

struct MilitarySlot {
    std::string label    = "";
    float progress       = 0.0f;
    bool active          = false;
};

struct AIMarchTask {
    bool        active          = false;
    std::string dynasty         = "";
    int         targetProvinceId = -1;
    int         daysAccumulated = 0;
    int         daysRequired    = 0;
};
