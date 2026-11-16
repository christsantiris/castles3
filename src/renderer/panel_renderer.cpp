#include "panel_renderer.h"
#include "../core/systems/date_system.h"
#include "../core/systems/unlock_system.h"
#include "../core/systems/combat_system.h"
#include "../core/systems/recruit_system.h"
#include "../core/systems/trade_system.h"
#include "../core/systems/task_slots.h"
#include "../core/systems/diplomacy_system.h"
#include "../core/systems/resource_system.h"

namespace PanelRenderer {

    static const int PANEL_X = 950;
    static const int PANEL_Y = 60;
    static const int PANEL_W = 330;
    static const int PANEL_H = 900;
    static const int TAB_H = 40;
    static const int BAR_H = 36;
    static const int BAR_MARGIN = 4;

    static const SDL_Color WOOD_DARK = {60, 35, 10, 255};
    static const SDL_Color WOOD_MID = {90, 55, 20, 255};
    static const SDL_Color GOLD = {255, 215, 0, 255};
    static const SDL_Color WHITE = {255, 255, 255, 255};
    static const SDL_Color DOT = {200, 180, 100, 255};

    static const SDL_Color TAB_STOCK = {0, 140, 0, 255};
    static const SDL_Color TAB_ARMY = {180, 0, 0, 255};
    static const SDL_Color TAB_RELAT = {0, 0, 180, 255};
    static const SDL_Color TAB_OPTS = {100, 100, 100, 255};

    static const SDL_Color BAR_STOCK = {20, 80, 20, 255};
    static const SDL_Color BAR_ARMY = {80, 20, 20, 255};
    static const SDL_Color BAR_DIPL = {20, 20, 80, 255};

    static void drawRect(SDL_Renderer* r, int x, int y, int w, int h, SDL_Color c) {
        SDL_SetRenderDrawColor(r, c.r, c.g, c.b, c.a);
        SDL_Rect rect = {x, y, w, h};
        SDL_RenderFillRect(r, &rect);
    }

    static void drawBorder(SDL_Renderer* r, int x, int y, int w, int h, SDL_Color c) {
        SDL_SetRenderDrawColor(r, c.r, c.g, c.b, c.a);
        SDL_Rect rect = {x, y, w, h};
        SDL_RenderDrawRect(r, &rect);
    }

    static void drawText(SDL_Renderer* r, TTF_Font* font, const std::string& text,
                         int x, int y, SDL_Color color) {
        SDL_Surface* s = TTF_RenderText_Solid(font, text.c_str(), color);
        if (!s) return;
        SDL_Texture* t = SDL_CreateTextureFromSurface(r, s);
        SDL_Rect rect = {x, y, s->w, s->h};
        SDL_RenderCopy(r, t, nullptr, &rect);
        SDL_FreeSurface(s);
        SDL_DestroyTexture(t);
    }

    static std::string workerPaceText(int assigned, int available, int currentDays, int nextDays) {
        if (assigned < 1) {
            if (available < 1 || nextDays < 1) {
                return "";
            }
            return "> " + std::to_string(nextDays) + "d";
        }
        if (currentDays < 1) {
            return "";
        }
        std::string text = std::to_string(currentDays) + "d";
        if (assigned >= available || nextDays < 1) {
            return text;
        }
        if (nextDays == currentDays) {
            return text + " = " + std::to_string(nextDays) + "d";
        }
        return text + " > " + std::to_string(nextDays) + "d";
    }

    static SDL_Color workerPaceColor(int assigned, int available, int currentDays, int nextDays) {
        if (assigned >= 1 && assigned < available && nextDays == currentDays) {
            return SDL_Color{150, 150, 150, 255};
        }
        return GOLD;
    }

    static void drawTextCentered(SDL_Renderer* r, TTF_Font* font, const std::string& text,
                                  int x, int y, int w, SDL_Color color) {
        SDL_Surface* s = TTF_RenderText_Solid(font, text.c_str(), color);
        if (!s) return;
        SDL_Texture* t = SDL_CreateTextureFromSurface(r, s);
        SDL_Rect rect = {x + (w - s->w) / 2, y, s->w, s->h};
        SDL_RenderCopy(r, t, nullptr, &rect);
        SDL_FreeSurface(s);
        SDL_DestroyTexture(t);
    }

    static void renderTaskBars(SDL_Renderer* r, TTF_Font* font, const World& world) {
        int y = PANEL_Y + BAR_MARGIN;
        int x = PANEL_X + BAR_MARGIN;
        int w = PANEL_W - (BAR_MARGIN * 2);

        struct CategoryBar {
            TaskCategory category;
            const char* name;
            bool secondUnlocked;
            SDL_Color fill;
            SDL_Color border;
        };
        const CategoryBar categories[] = {
            {TaskCategory::Administrative, "Stock", UnlockSystem::hasSecondStockSlot(world), BAR_STOCK, {0, 200, 0, 255}},
            {TaskCategory::Military, "Military", UnlockSystem::hasSecondMilitarySlot(world), BAR_ARMY, {200, 0, 0, 255}},
            {TaskCategory::Diplomatic, "Diplomacy", UnlockSystem::hasSecondDiplomacySlot(world), BAR_DIPL, {0, 0, 200, 255}}
        };
        const auto tasks = world.tasks();
        for (const auto& category : categories) {
            std::vector<World::TaskView> active;
            for (const auto& entry : tasks) {
                if (entry.category == category.category && entry.task->active) {
                    active.push_back(entry);
                }
            }
            for (int i = 0; i < 2; i++) {
                if (i == 1 && !category.secondUnlocked) {
                    y += BAR_H + BAR_MARGIN;
                    continue;
                }
                bool occupied = i < (int)active.size();
                std::string label = occupied ? active[i].label : std::string(category.name) + " slot " + std::to_string(i + 1);
                float progress = occupied ? std::max(active[i].task->progress(), 0.01f) : 0.0f;
                drawRect(r, x, y, w, BAR_H, category.fill);
                drawBorder(r, x, y, w, BAR_H, category.border);
                if (occupied) {
                    drawRect(r, x + 2, y + BAR_H - 6, (int)((w - 4) * progress), 4, category.border);
                }
                drawText(r, font, label, x + 6, y + 8, WHITE);
                y += BAR_H + BAR_MARGIN;
            }
        }
    }

    static void renderTabs(SDL_Renderer* r, TTF_Font* font, int activeTab) {
        const char* labels[] = {"STOCK", "ARMY", "RELAT", "OPTS"};
        SDL_Color tabColors[] = {TAB_STOCK, TAB_ARMY, TAB_RELAT, TAB_OPTS};
        int tabW = PANEL_W / 4;
        int tabY = PANEL_Y + (BAR_H + BAR_MARGIN) * 6 + BAR_MARGIN;

        for (int i = 0; i < 4; i++) {
            int tx = PANEL_X + (i * tabW);
            SDL_Color base = tabColors[i];
            SDL_Color fill = (activeTab == i)
                ? base
                : SDL_Color{(Uint8)(base.r / 2), (Uint8)(base.g / 2), (Uint8)(base.b / 2), 255};
            drawRect(r, tx, tabY, tabW, TAB_H, fill);
            drawBorder(r, tx, tabY, tabW, TAB_H, base);
            drawTextCentered(r, font, labels[i], tx, tabY + 10, tabW, GOLD);
        }
    }

    static void renderStockTab(SDL_Renderer* r, TTF_Font* font, World& world) {
        const char* resNames[] = {"Food", "Timber", "Iron", "Gold"};
        ResourceType resTypes[] = {
            ResourceType::Food, ResourceType::Timber,
            ResourceType::Iron, ResourceType::Gold
        };
        int stockValues[] = {
            world.resources.food, world.resources.timber,
            world.resources.iron, world.resources.gold
        };

        int infoY = PANEL_Y + (BAR_H + BAR_MARGIN) * 6 + BAR_MARGIN + TAB_H;
        int y = infoY + 10;
        int x = PANEL_X + 8;
        int w = PANEL_W - 16;
        int rowH = 50;

        for (int i = 0; i < 4; i++) {
            bool canCollect = world.countOwnedProvinces(world.ctx.playerDynasty, resTypes[i]) > 0;
            SDL_Color rowColor = canCollect ? WOOD_MID : SDL_Color{40, 40, 40, 255};
            SDL_Color labelColor = canCollect ? GOLD : SDL_Color{80, 80, 80, 255};
            SDL_Color textColor = canCollect ? WHITE : SDL_Color{60, 60, 60, 255};

            drawRect(r, x, y, w, rowH, rowColor);
            drawBorder(r, x, y, w, rowH, canCollect ? GOLD : SDL_Color{60, 60, 60, 255});

            std::string label = std::string(resNames[i]) + ": " + std::to_string(stockValues[i]);
            drawText(r, font, label, x + 6, y + 4, labelColor);
            int owned = world.countOwnedProvinces(world.ctx.playerDynasty, resTypes[i]);
            int assigned = world.pendingWorkers[i];
            int available = world.workerPool.availableStockWorkers;
            int nextWorkers = assigned < 1 ? 1 : assigned + 1;
            int currentDays = ResourceSystem::collectionDays(assigned, owned);
            int nextDays = ResourceSystem::collectionDays(nextWorkers, owned);
            std::string pace = workerPaceText(assigned, available, currentDays, nextDays);
            if (!pace.empty()) {
                drawText(r, font, pace, x + 200, y + 4, workerPaceColor(assigned, available, currentDays, nextDays));
            }

            std::string wLabel = "W:" + std::to_string(world.pendingWorkers[i]);
            drawText(r, font, wLabel, x + 6, y + 26, textColor);

            drawRect(r, x + 60, y + 24, 20, 20, {120, 0, 0, 255});
            drawBorder(r, x + 60, y + 24, 20, 20, GOLD);
            drawTextCentered(r, font, "-", x + 60, y + 26, 20, WHITE);

            drawRect(r, x + 86, y + 24, 20, 20, {0, 120, 0, 255});
            drawBorder(r, x + 86, y + 24, 20, 20, GOLD);
            drawTextCentered(r, font, "+", x + 86, y + 26, 20, WHITE);

            bool isActive = false;
            int activeSlot = -1;
            for (int s = 0; s < 2; s++) {
                if (world.collectionTasks.slots[s].active &&
                    world.collectionTasks.slots[s].resource == resTypes[i]) {
                    isActive = true;
                    activeSlot = s;
                    break;
                }
            }

            canCollect = canCollect && (isActive || (TaskSlots::administrativeAvailable(world)
                         && world.pendingWorkers[i] >= 1
                         && world.pendingWorkers[i] <= world.workerPool.availableStockWorkers));
            SDL_Color btnColor = isActive ? SDL_Color{120, 0, 0, 255}
                                        : canCollect ? SDL_Color{0, 80, 0, 255}
                                        : SDL_Color{40, 40, 40, 255};
            SDL_Color btnBorder = canCollect ? GOLD : SDL_Color{60, 60, 60, 255};
            SDL_Color btnText = canCollect || isActive ? GOLD : SDL_Color{60, 60, 60, 255};
            drawRect(r, x + 160, y + 24, 80, 20, btnColor);
            drawBorder(r, x + 160, y + 24, 80, 20, btnBorder);
            drawTextCentered(r, font, isActive ? "Cancel" : "Collect", x + 160, y + 26, 80, btnText);


            y += rowH + 4;
        }
    }

    static void renderArmyTab(SDL_Renderer* r, TTF_Font* font, World& world) {
        int infoY = PANEL_Y + (BAR_H + BAR_MARGIN) * 6 + BAR_MARGIN + TAB_H;
        int y = infoY + 10;
        int x = PANEL_X + 8;
        int w = PANEL_W - 16;
        int rowH = 60;

        std::string armyStr = "Army: ";
        auto it = world.armies.find(world.ctx.playerDynasty);
        if (it != world.armies.end()) {
            armyStr += "INF:" + std::to_string(it->second.infantry) +
                       " ARC:" + std::to_string(it->second.archers) +
                       " KNT:" + std::to_string(it->second.knights);
        }
        drawText(r, font, armyStr, x, y, GOLD);
        y += 24;

        const char* unitNames[] = {"Infantry", "Archers", "Knights"};
        UnitType unitTypes[] = {UnitType::Infantry, UnitType::Archers, UnitType::Knights};
        const char* costs[] = {"Cost: 3 Iron", "Cost: 3 Timber", "Cost: 3 Iron + 3 Timber"};

        for (int i = 0; i < 3; i++) {
            if (i == 2 && !UnlockSystem::hasKnights(world)) {
                drawRect(r, x, y, w, rowH, WOOD_MID);
                drawBorder(r, x, y, w, rowH, GOLD);
                drawText(r, font, "Knights: Locked", x + 6, y + 8, {150, 150, 150, 255});
                y += rowH + 4;
                continue;
            }

            drawRect(r, x, y, w, rowH, WOOD_MID);
            drawBorder(r, x, y, w, rowH, GOLD);

            drawText(r, font, unitNames[i], x + 6, y + 4, GOLD);
            drawText(r, font, costs[i], x + 6, y + 24, {150, 150, 150, 255});

            bool isActive = false;
            int activeSlot = -1;
            for (int s = 0; s < 2; s++) {
                if (world.recruitTasks.slots[s].active &&
                    world.recruitTasks.slots[s].unitType == unitTypes[i]) {
                    isActive = true;
                    activeSlot = s;
                    break;
                }
            }

            // if (isActive) {
            //     auto& task = world.recruitTasks.slots[activeSlot];
            //     SDL_SetRenderDrawColor(r, 30, 15, 5, 255);
            //     SDL_Rect pbg = {x, y + rowH - 8, w, 6};
            //     SDL_RenderFillRect(r, &pbg);
            //     SDL_SetRenderDrawColor(r, 0, 200, 0, 255);
            //     SDL_Rect pfill = {x, y + rowH - 8, (int)(w * task.progress()), 6};
            //     SDL_RenderFillRect(r, &pfill);
            // }

            bool canStart = !isActive && RecruitSystem::canAfford(world, unitTypes[i])
                            && world.pendingMilitaryWorkers >= 1
                            && world.pendingMilitaryWorkers <= world.workerPool.availableMilitaryWorkers
                            && TaskSlots::militaryAvailable(world);

            SDL_Color btnColor = isActive ? SDL_Color{120, 0, 0, 255}
                               : canStart ? SDL_Color{0, 80, 0, 255}
                               : SDL_Color{50, 50, 50, 255};
            drawRect(r, x + 160, y + 18, 80, 22, btnColor);
            drawBorder(r, x + 160, y + 18, 80, 22, GOLD);
            drawTextCentered(r, font,
                isActive ? "Cancel" : "Recruit",
                x + 160, y + 20, 80,
                canStart || isActive ? GOLD : SDL_Color{100, 100, 100, 255});

            y += rowH + 4;
        }

        std::string wLabel = "Workers: " + std::to_string(world.pendingMilitaryWorkers);
        drawText(r, font, wLabel, x, y + 8, WHITE);
        int assigned = world.pendingMilitaryWorkers;
        int available = world.workerPool.availableMilitaryWorkers;
        int nextWorkers = assigned < 1 ? 1 : assigned + 1;
        int currentDays = RecruitSystem::recruitmentDays(assigned);
        int nextDays = RecruitSystem::recruitmentDays(nextWorkers);
        std::string pace = workerPaceText(assigned, available, currentDays, nextDays);
        if (!pace.empty()) {
            drawText(r, font, pace, x + 178, y + 8, workerPaceColor(assigned, available, currentDays, nextDays));
        }

        drawRect(r, x + 120, y + 6, 22, 22, {120, 0, 0, 255});
        drawBorder(r, x + 120, y + 6, 22, 22, GOLD);
        drawTextCentered(r, font, "-", x + 120, y + 8, 22, WHITE);

        drawRect(r, x + 148, y + 6, 22, 22, {0, 120, 0, 255});
        drawBorder(r, x + 148, y + 6, 22, 22, GOLD);
        drawTextCentered(r, font, "+", x + 148, y + 8, 22, WHITE);
    }

    static void renderRelatTab(SDL_Renderer* r, TTF_Font* font, const World& world);

    static void renderInfoArea(SDL_Renderer* r, TTF_Font* font, World& world, bool musicOn) {
        int infoY = PANEL_Y + (BAR_H + BAR_MARGIN) * 6 + BAR_MARGIN + TAB_H;
        int infoH = PANEL_H - infoY;

        drawRect(r, PANEL_X, infoY, PANEL_W, infoH, WOOD_DARK);
        drawBorder(r, PANEL_X, infoY, PANEL_W, infoH, GOLD);

        const ProvinceComponent* selected = nullptr;
        for (auto& p : world.provinces)
            if (p.isSelected) { selected = &p; break; }

        if (selected) {
            drawText(r, font, selected->name, PANEL_X + 10, infoY + 15, GOLD);
            bool isOwned = selected->owner == world.ctx.playerDynasty;
            bool isScouted = world.scoutedProvinces.count(selected->id) > 0;
            if (isOwned || isScouted) {
                drawText(r, font, "Owner: " + selected->owner,
                        PANEL_X + 10, infoY + 45, WHITE);
                drawText(r, font, "Resource: " + selected->resource,
                        PANEL_X + 10, infoY + 75, WHITE);
                auto it = world.armies.find(selected->owner);
                if (it != world.armies.end()) {
                    std::string armyStr = "Army: INF:" + std::to_string(it->second.infantry) +
                                         " ARC:" + std::to_string(it->second.archers) +
                                         " KNT:" + std::to_string(it->second.knights);
                    drawText(r, font, armyStr, PANEL_X + 10, infoY + 105, WHITE);
                }
            }

            bool isEnemy = selected->owner != world.ctx.playerDynasty;
            bool isAdj = CombatSystem::isAdjacent(world, selected->id);

            if (isEnemy && isAdj && world.battle.phase == BattlePhase::None) {
                bool marching = false;
                for (int s = 0; s < 2; s++)
                    if (world.combatTasks.slots[s].active &&
                        world.combatTasks.slots[s].targetProvinceId == selected->id)
                        marching = true;

                if (!marching) {
                    std::string wLabel = "Military: " + std::to_string(world.pendingMilitaryWorkers);
                    drawText(r, font, wLabel, PANEL_X + 10, infoY + 135, WHITE);
                    bool neutral = selected->owner == "neutral";
                    int marchAssigned = world.pendingMilitaryWorkers;
                    int marchAvailable = world.workerPool.availableMilitaryWorkers;
                    int marchNext = marchAssigned < 1 ? 1 : marchAssigned + 1;
                    int marchCurrent = CombatSystem::marchDays(marchAssigned, neutral);
                    int marchNextDays = CombatSystem::marchDays(marchNext, neutral);
                    std::string marchPace = workerPaceText(marchAssigned, marchAvailable, marchCurrent, marchNextDays);
                    if (!marchPace.empty()) {
                        drawText(r, font, marchPace, PANEL_X + 222, infoY + 135,
                                 workerPaceColor(marchAssigned, marchAvailable, marchCurrent, marchNextDays));
                    }

                    drawRect(r, PANEL_X + 170, infoY + 133, 20, 20, {120, 0, 0, 255});
                    drawBorder(r, PANEL_X + 170, infoY + 133, 20, 20, GOLD);
                    drawTextCentered(r, font, "-", PANEL_X + 170, infoY + 135, 20, WHITE);

                    drawRect(r, PANEL_X + 196, infoY + 133, 20, 20, {0, 120, 0, 255});
                    drawBorder(r, PANEL_X + 196, infoY + 133, 20, 20, GOLD);
                    drawTextCentered(r, font, "+", PANEL_X + 196, infoY + 135, 20, WHITE);

                    bool canMarch = CombatSystem::canStartMarch(world, selected->id)
                                    && world.pendingMilitaryWorkers >= 1
                                    && world.pendingMilitaryWorkers <= world.workerPool.availableMilitaryWorkers;
                    drawRect(r, PANEL_X + 10, infoY + 160, 200, 36, canMarch ? SDL_Color{0, 100, 0, 255} : SDL_Color{40, 40, 40, 255});
                    drawBorder(r, PANEL_X + 10, infoY + 160, 200, 36, GOLD);
                    drawTextCentered(r, font, "ATTACK", PANEL_X + 10, infoY + 170, 200, canMarch ? GOLD : SDL_Color{100, 100, 100, 255});
                } else {
                    drawText(r, font, "Marching...", PANEL_X + 10, infoY + 140, GOLD);
                }

                // Scout section
                bool scouting = false;
                for (int s = 0; s < 2; s++)
                    if (world.scoutTasks.slots[s].active &&
                        world.scoutTasks.slots[s].targetProvinceId == selected->id)
                        scouting = true;

                if (scouting) {
                    drawText(r, font, "Scouting...", PANEL_X + 10, infoY + 208, {100, 150, 255, 255});
                } else if (!isScouted) {
                    std::string scoutWLabel = "Workers: " + std::to_string(world.pendingDiplomaticWorkers);
                    drawText(r, font, scoutWLabel, PANEL_X + 10, infoY + 208, WHITE);
                    int scoutAssigned = world.pendingDiplomaticWorkers;
                    int scoutAvailable = world.workerPool.availableDiplomaticWorkers;
                    int scoutNext = scoutAssigned < 1 ? 1 : scoutAssigned + 1;
                    int scoutCurrent = DiplomacySystem::diplomaticDays(scoutAssigned);
                    int scoutNextDays = DiplomacySystem::diplomaticDays(scoutNext);
                    std::string scoutPace = workerPaceText(scoutAssigned, scoutAvailable, scoutCurrent, scoutNextDays);
                    if (!scoutPace.empty()) {
                        drawText(r, font, scoutPace, PANEL_X + 222, infoY + 208,
                                 workerPaceColor(scoutAssigned, scoutAvailable, scoutCurrent, scoutNextDays));
                    }

                    drawRect(r, PANEL_X + 170, infoY + 206, 20, 20, {120, 0, 0, 255});
                    drawBorder(r, PANEL_X + 170, infoY + 206, 20, 20, GOLD);
                    drawTextCentered(r, font, "-", PANEL_X + 170, infoY + 208, 20, WHITE);

                    drawRect(r, PANEL_X + 196, infoY + 206, 20, 20, {0, 120, 0, 255});
                    drawBorder(r, PANEL_X + 196, infoY + 206, 20, 20, GOLD);
                    drawTextCentered(r, font, "+", PANEL_X + 196, infoY + 208, 20, WHITE);

                    bool canScout = DiplomacySystem::canStartScout(world, selected->id)
                                    && world.pendingDiplomaticWorkers >= 1
                                    && world.pendingDiplomaticWorkers <= world.workerPool.availableDiplomaticWorkers;
                    drawRect(r, PANEL_X + 10, infoY + 233, 200, 36, canScout ? SDL_Color{0, 0, 120, 255} : SDL_Color{40, 40, 40, 255});
                    drawBorder(r, PANEL_X + 10, infoY + 233, 200, 36, GOLD);
                    drawTextCentered(r, font, "SCOUT", PANEL_X + 10, infoY + 243, 200, canScout ? GOLD : SDL_Color{100, 100, 100, 255});
                }

                // Bribe section
                bool bribing = false;
                for (int s = 0; s < 2; s++)
                    if (world.bribeTasks.slots[s].active &&
                        world.bribeTasks.slots[s].targetProvinceId == selected->id)
                        bribing = true;

                if (bribing) {
                    drawText(r, font, "Bribing...", PANEL_X + 10, infoY + 283, {100, 150, 255, 255});
                } else if (selected->name != "Constantinople") {
                    std::string brWLabel = "Workers: " + std::to_string(world.pendingDiplomaticWorkers);
                    drawText(r, font, brWLabel, PANEL_X + 10, infoY + 278, WHITE);
                    int bribeAssigned = world.pendingDiplomaticWorkers;
                    int bribeAvailable = world.workerPool.availableDiplomaticWorkers;
                    int bribeNext = bribeAssigned < 1 ? 1 : bribeAssigned + 1;
                    int bribeCurrent = DiplomacySystem::diplomaticDays(bribeAssigned);
                    int bribeNextDays = DiplomacySystem::diplomaticDays(bribeNext);
                    std::string bribePace = workerPaceText(bribeAssigned, bribeAvailable, bribeCurrent, bribeNextDays);
                    if (!bribePace.empty()) {
                        drawText(r, font, bribePace, PANEL_X + 222, infoY + 278,
                                 workerPaceColor(bribeAssigned, bribeAvailable, bribeCurrent, bribeNextDays));
                    }

                    drawRect(r, PANEL_X + 170, infoY + 276, 20, 20, {120, 0, 0, 255});
                    drawBorder(r, PANEL_X + 170, infoY + 276, 20, 20, GOLD);
                    drawTextCentered(r, font, "-", PANEL_X + 170, infoY + 278, 20, WHITE);

                    drawRect(r, PANEL_X + 196, infoY + 276, 20, 20, {0, 120, 0, 255});
                    drawBorder(r, PANEL_X + 196, infoY + 276, 20, 20, GOLD);
                    drawTextCentered(r, font, "+", PANEL_X + 196, infoY + 278, 20, WHITE);

                    std::string goldLabel = "Gold: " + std::to_string(world.pendingBribeGold);
                    drawText(r, font, goldLabel, PANEL_X + 10, infoY + 305, WHITE);

                    drawRect(r, PANEL_X + 170, infoY + 303, 20, 20, {120, 0, 0, 255});
                    drawBorder(r, PANEL_X + 170, infoY + 303, 20, 20, GOLD);
                    drawTextCentered(r, font, "-", PANEL_X + 170, infoY + 305, 20, WHITE);

                    drawRect(r, PANEL_X + 196, infoY + 303, 20, 20, {0, 120, 0, 255});
                    drawBorder(r, PANEL_X + 196, infoY + 303, 20, 20, GOLD);
                    drawTextCentered(r, font, "+", PANEL_X + 196, infoY + 305, 20, WHITE);

                    bool canBribe = world.resources.gold >= world.pendingBribeGold
                                    && world.pendingDiplomaticWorkers >= 1
                                    && world.pendingDiplomaticWorkers <= world.workerPool.availableDiplomaticWorkers
                                    && DiplomacySystem::canStartBribe(world, selected->id);
                    SDL_Color bribeBtn = canBribe ? SDL_Color{0, 0, 120, 255}
                                                  : SDL_Color{40, 40, 40, 255};
                    drawRect(r, PANEL_X + 10, infoY + 330, 200, 36, bribeBtn);
                    drawBorder(r, PANEL_X + 10, infoY + 330, 200, 36, GOLD);
                    drawTextCentered(r, font, "BRIBE", PANEL_X + 10, infoY + 340, 200,
                                     canBribe ? GOLD : SDL_Color{80, 80, 80, 255});
                }
            }
        } else if (world.ctx.activeTab == 0) {
            renderStockTab(r, font, world);
            return;
        } else if (world.ctx.activeTab == 1) {
            renderArmyTab(r, font, world);
            return;
        } else if (world.ctx.activeTab == 2) {
            renderRelatTab(r, font, world);
            return;
        } else if (world.ctx.activeTab == 3) {
            renderOptsTab(r, font, world, musicOn);
            return;
        } else {
            int dotX = PANEL_X + 10;

            drawRect(r, PANEL_X + 4, infoY + 20, PANEL_W - 8, 36, WOOD_MID);
            drawBorder(r, PANEL_X + 4, infoY + 20, PANEL_W - 8, 36, GOLD);
            SDL_SetRenderDrawColor(r, DOT.r, DOT.g, DOT.b, DOT.a);
            SDL_RenderDrawPoint(r, dotX + 4, infoY + 38);
            SDL_RenderDrawPoint(r, dotX + 5, infoY + 38);
            SDL_RenderDrawPoint(r, dotX + 4, infoY + 39);
            SDL_RenderDrawPoint(r, dotX + 5, infoY + 39);
            drawTextCentered(r, font, DateSystem::toString(world.date),
                             PANEL_X, infoY + 28, PANEL_W, WHITE);

            drawRect(r, PANEL_X + 4, infoY + 64, PANEL_W - 8, 36, WOOD_MID);
            drawBorder(r, PANEL_X + 4, infoY + 64, PANEL_W - 8, 36, GOLD);
            SDL_SetRenderDrawColor(r, DOT.r, DOT.g, DOT.b, DOT.a);
            SDL_RenderDrawPoint(r, dotX + 4, infoY + 82);
            SDL_RenderDrawPoint(r, dotX + 5, infoY + 82);
            SDL_RenderDrawPoint(r, dotX + 4, infoY + 83);
            SDL_RenderDrawPoint(r, dotX + 5, infoY + 83);
            std::string scoreStr = "Score: " + std::to_string(world.ctx.score);
            drawTextCentered(r, font, scoreStr, PANEL_X, infoY + 72, PANEL_W, WHITE);
        }
    }

    void render(SDL_Renderer* renderer, TTF_Font* font, World& world, bool musicOn) {
        drawRect(renderer, PANEL_X, PANEL_Y, PANEL_W, PANEL_H - PANEL_Y, WOOD_DARK);
        renderTaskBars(renderer, font, world);
        renderTabs(renderer, font, world.ctx.activeTab);
        renderInfoArea(renderer, font, world, musicOn);
    }

    static void renderOptsTab(SDL_Renderer* r, TTF_Font* font, World& world, bool musicOn) {
        int infoY = PANEL_Y + (BAR_H + BAR_MARGIN) * 6 + BAR_MARGIN + TAB_H;
        int y = infoY + 10;
        int x = PANEL_X + 8;
        int w = PANEL_W - 16;
        int rowH = 50;

        const char* labels[] = {
            world.ctx.paused ? "Resume" : "Pause",
            musicOn ? "Music: On" : "Music: Off",
            "Save Game",
            "Load Game",
            "Quit"
        };

        SDL_Color btnColors[] = {
            world.ctx.paused ? SDL_Color{0, 100, 0, 255} : SDL_Color{100, 80, 0, 255},
            {0, 80, 80, 255},
            {0, 80, 50, 255},
            {0, 60, 90, 255},
            {120, 0, 0, 255}
        };

        for (int i = 0; i < 5; i++) {
            drawRect(r, x, y, w, rowH, btnColors[i]);
            drawBorder(r, x, y, w, rowH, GOLD);
            drawTextCentered(r, font, labels[i], x, y + 16, w, GOLD);
            y += rowH + 8;
        }
    }

    static void renderRelatTab(SDL_Renderer* r, TTF_Font* font, const World& world) {
        static const std::vector<std::string> ALL_DYNASTIES = {
            "Kantakouzenos", "Doukas", "Palaiologos", "Phokas", "Komnenos", "Baldwin II"
        };

        int infoY = PANEL_Y + (BAR_H + BAR_MARGIN) * 6 + BAR_MARGIN + TAB_H;
        int x = PANEL_X + 8;
        int w = PANEL_W - 16;
        int dy = infoY + 8;

        for (const auto& d : ALL_DYNASTIES) {
            if (d == world.ctx.playerDynasty) {
                continue;
            }
            bool defeated = world.isDefeated(d);
            bool selected = (d == world.pendingTradeDynasty);
            SDL_Color bg = defeated  ? SDL_Color{30, 30, 30, 255}
                         : selected  ? SDL_Color{0, 60, 120, 255}
                         : WOOD_MID;
            SDL_Color border = defeated ? SDL_Color{60, 60, 60, 255} : GOLD;
            SDL_Color textCol = defeated ? SDL_Color{80, 80, 80, 255} : WHITE;
            drawRect(r, x, dy, w, 28, bg);
            drawBorder(r, x, dy, w, 28, border);
            drawText(r, font, d, x + 6, dy + 7, textCol);
            dy += 32;
        }

        int tradeY = dy + 8;

        for (int i = 0; i < 2; i++) {
            const auto& task = world.tradeTasks.slots[i];
            if (!task.active) {
                continue;
            }
            std::string line = task.dynasty + ": " + std::to_string(task.qty) + " " +
                TradeSystem::resourceName(task.offerRes) + " for " +
                TradeSystem::resourceName(task.requestRes);
            drawText(r, font, line, x + 6, tradeY + 7, GOLD);
            drawRect(r, x + w - 22, tradeY, 22, 26, {120, 0, 0, 255});
            drawBorder(r, x + w - 22, tradeY, 22, 26, GOLD);
            drawTextCentered(r, font, "X", x + w - 22, tradeY + 6, 22, WHITE);
            tradeY += 32;
        }

        // Offer row
        drawText(r, font, "Offer:", x, tradeY + 5, WHITE);
        drawRect(r, x + 110, tradeY, 22, 26, {120, 0, 0, 255});
        drawBorder(r, x + 110, tradeY, 22, 26, GOLD);
        drawTextCentered(r, font, "<", x + 110, tradeY + 6, 22, WHITE);
        drawTextCentered(r, font, TradeSystem::resourceName(world.pendingTradeOfferRes),
                         x + 134, tradeY + 5, 64, GOLD);
        drawRect(r, x + 200, tradeY, 22, 26, {0, 120, 0, 255});
        drawBorder(r, x + 200, tradeY, 22, 26, GOLD);
        drawTextCentered(r, font, ">", x + 200, tradeY + 6, 22, WHITE);
        std::string offerStock = "(" + std::to_string(
            TradeSystem::playerStock(world, world.pendingTradeOfferRes)) + ")";
        drawText(r, font, offerStock, x + 228, tradeY + 5, WHITE);

        tradeY += 36;

        // Request row
        bool produces = !world.pendingTradeDynasty.empty() &&
                        TradeSystem::dynastyProduces(world, world.pendingTradeDynasty,
                                                     world.pendingTradeRequestRes);
        drawText(r, font, "Want:", x, tradeY + 5, WHITE);
        drawRect(r, x + 110, tradeY, 22, 26, {120, 0, 0, 255});
        drawBorder(r, x + 110, tradeY, 22, 26, GOLD);
        drawTextCentered(r, font, "<", x + 110, tradeY + 6, 22, WHITE);
        SDL_Color wantCol = produces ? GOLD : SDL_Color{80, 80, 80, 255};
        drawTextCentered(r, font, TradeSystem::resourceName(world.pendingTradeRequestRes),
                         x + 134, tradeY + 5, 64, wantCol);
        drawRect(r, x + 200, tradeY, 22, 26, {0, 120, 0, 255});
        drawBorder(r, x + 200, tradeY, 22, 26, GOLD);
        drawTextCentered(r, font, ">", x + 200, tradeY + 6, 22, WHITE);

        tradeY += 36;

        // Quantity row
        drawText(r, font, "Qty:", x, tradeY + 4, WHITE);
        drawRect(r, x + 110, tradeY, 22, 22, {120, 0, 0, 255});
        drawBorder(r, x + 110, tradeY, 22, 22, GOLD);
        drawTextCentered(r, font, "-", x + 110, tradeY + 4, 22, WHITE);
        drawTextCentered(r, font, std::to_string(world.pendingTradeQty),
                         x + 134, tradeY + 4, 44, WHITE);
        drawRect(r, x + 180, tradeY, 22, 22, {0, 120, 0, 255});
        drawBorder(r, x + 180, tradeY, 22, 22, GOLD);
        drawTextCentered(r, font, "+", x + 180, tradeY + 4, 22, WHITE);

        tradeY += 36;

        // Diplomat worker picker
        std::string wLabel = "Diplomats: " + std::to_string(world.pendingDiplomaticWorkers);
        drawText(r, font, wLabel, x, tradeY + 4, WHITE);
        int tradeAssigned = world.pendingDiplomaticWorkers;
        int tradeAvailable = world.workerPool.availableDiplomaticWorkers;
        int tradeNext = tradeAssigned < 1 ? 1 : tradeAssigned + 1;
        int tradeCurrent = TradeSystem::tradeDays(tradeAssigned);
        int tradeNextDays = TradeSystem::tradeDays(tradeNext);
        std::string tradePace = workerPaceText(tradeAssigned, tradeAvailable, tradeCurrent, tradeNextDays);
        if (!tradePace.empty()) {
            drawText(r, font, tradePace, x + 226, tradeY + 4,
                     workerPaceColor(tradeAssigned, tradeAvailable, tradeCurrent, tradeNextDays));
        }
        drawRect(r, x + 170, tradeY, 22, 22, {120, 0, 0, 255});
        drawBorder(r, x + 170, tradeY, 22, 22, GOLD);
        drawTextCentered(r, font, "-", x + 170, tradeY + 4, 22, WHITE);
        drawRect(r, x + 198, tradeY, 22, 22, {0, 120, 0, 255});
        drawBorder(r, x + 198, tradeY, 22, 22, GOLD);
        drawTextCentered(r, font, "+", x + 198, tradeY + 4, 22, WHITE);

        tradeY += 36;

        // Trade button
        bool ok = TradeSystem::canStartTrade(world);
        SDL_Color tradeBtn = ok ? SDL_Color{0, 100, 0, 255} : SDL_Color{50, 50, 50, 255};
        SDL_Color tradeTxt = ok ? GOLD : SDL_Color{80, 80, 80, 255};
        drawRect(r, x + 87, tradeY, 140, 32, tradeBtn);
        drawBorder(r, x + 87, tradeY, 140, 32, GOLD);
        drawTextCentered(r, font, "TRADE", x + 87, tradeY + 9, 140, tradeTxt);
    }
}