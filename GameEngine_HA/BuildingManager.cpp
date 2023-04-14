#include "BuildingManager.h"

void BuildingManager::addBuilding(Building building)
{
	mBuildings.push_back(building);
}

bool BuildingManager::craftBuilding(Building building, std::vector<Item> items)
{
	// Using a map so first -> id(wood,stone) and second -> amt required
	std::map<itemId, int> craftingRecipe;

	switch (building.mType)
	{
	case WORKSHOP:
			craftingRecipe.emplace(wood, 5);
		break;
	default:
		break;
	}

	// Loop through map as there may be different types of items required
	for (std::pair<itemId, int> item : craftingRecipe)
	{
		int amtRequired = item.second;
		int amtGiven = 0;

		for (Item givenItem : items)
		{
			if (givenItem.id == item.first)
			{
				amtGiven++;
			}
		}
		// If any required item is not met return false
		if (amtGiven < amtRequired)
		{
			return false;
		}
	}

	// Recipe was met so return true
	addBuilding(building);
	return true;
}

std::vector<Building> BuildingManager::getBuildings()
{
	return mBuildings;
}

int BuildingManager::getModifierForType(BuildingType type) const
{
	int modifier = 0;
	for (Building building : mBuildings)
	{
		if (building.mType == type)
		{
			switch (type)
			{
			case FORGE:
				modifier += 2;
				break;
			case ANVIL:
				modifier += 2;
				break;
			case DUMMY:
				modifier++;
				break;
			default:
				break;
			}
		}
	}
	return modifier;
}