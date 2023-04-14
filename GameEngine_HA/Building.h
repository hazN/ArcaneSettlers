#pragma once

#include "BuildingType.h"
#include "Inventory.h"
#include <string>

class Building
{
public:
    Building(std::string name, std::string icon, BuildingType type);
    Building(std::string name, std::string icon, BuildingType type, int health);

    std::string mName;
    std::string mIcon;
    int mHealth;
    BuildingType mType;
    Inventory* mInventory;
};
