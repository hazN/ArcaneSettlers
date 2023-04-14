#include "Building.h"

Building::Building(std::string name, std::string icon, BuildingType type)
{
	mInventory = new Inventory();
	mName = name;
	mIcon = icon;
	mType = type;
	mHealth = 100;
}

Building::Building(std::string name, std::string icon, BuildingType type, int health)
{
	mInventory = new Inventory();
	mName = name;
	mIcon = icon;
	mType = type;
	mHealth = health;
}
