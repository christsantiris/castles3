#include "world.h"

ProvinceComponent* World::findProvince(int id) {
    for (auto& p : provinces)
        if (p.id == id) return &p;
    return nullptr;
}

int World::countOwnedProvinces(const std::string& dynasty, ResourceType res) const {
    const char* resNames[] = {"Food", "Timber", "Iron", "Gold", "None"};
    std::string resName = resNames[(int)res];
    int count = 0;
    for (auto& p : provinces)
        if (p.owner == dynasty && p.resource == resName)
            count++;
    return count;
}

int World::countOwnedProvinces(const std::string& dynasty) const {
    int count = 0;
    for (auto& p : provinces)
        if (p.owner == dynasty)
            count++;
    return count;
}

bool World::isDefeated(const std::string& dynasty) const {
    return countOwnedProvinces(dynasty) == 0;
}
// Register each task store once here. Category policy never enumerates task
// types: new stores declare category metadata and join this common view.
std::vector<World::TaskView> World::tasks() const {
    std::vector<TaskView> result;
    auto registerStore = [&](const auto& tasks) {
        for (const auto& task : tasks) {
            result.push_back({&task, task.category, task.label});
        }
    };
    registerStore(collectionTasks.slots);
    registerStore(combatTasks.slots);
    registerStore(recruitTasks.slots);
    registerStore(bribeTasks.slots);
    registerStore(scoutTasks.slots);
    registerStore(tradeTasks.slots);
    return result;
}
