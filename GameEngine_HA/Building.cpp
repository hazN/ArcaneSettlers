#include "Building.h"

Building::Building(int id, std::string name, std::string icon, BuildingType type)
{
	mId = id;
	mInventory = new Inventory();
	mName = name;
	mIcon = icon;
	mType = type;
	mHealth = 100;
}

Building::Building(int id, std::string name, std::string icon, BuildingType type, int health)
{
	mId = id;
	mInventory = new Inventory();
	mName = name;
	mIcon = icon;
	mType = type;
	mHealth = health;
}