#pragma once

#include "Building.h"
#include <vector>
#include <map>

class BuildingManager
{
public:
    BuildingManager();
    ~BuildingManager();
    void addBuilding(Building building);
    bool craftBuilding(Building building, std::vector<Item> items);
    bool canCraft(BuildingType building);
    std::vector<Building> getBuildings();
    int getModifierForType(BuildingType type) const;

private:
    std::vector<Building> mBuildings;
};
