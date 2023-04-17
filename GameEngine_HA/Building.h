#pragma once
#include "BuildingType.h"
#include "Inventory.h"
#include <string>
#include <glm/glm.hpp>
class Building
{
public:
    Building(int id, std::string name, std::string icon, BuildingType type);
    Building(int id, std::string name, std::string icon, BuildingType type, int health);

    int mId;
    std::string mName;
    std::string mIcon;
    int mHealth;
    BuildingType mType;
    Inventory* mInventory;
    glm::vec3 mPosition;
};
