#pragma once
#include <string>
enum BuildingType
{
	NONE,
	DEPOT,
	TREE,
	ROCK,
	GOLD,
	WORKSHOP,
	FORGE,
	ANVIL,
	DUMMY,
};
static std::string getBuildingIcon(BuildingType type)
{
	switch (type)
	{
	case NONE:
		break;
	case DEPOT:
		break;
	case TREE:
		break;
	case ROCK:
		break;
	case GOLD:
		break;
	case WORKSHOP:
		return "Workstation.bmp";
		break;
	case FORGE:
		return "Forge.bmp";
		break;
	case ANVIL:
		return "Anvil.bmp";
		break;
	case DUMMY:
		return "TrainingDummy.bmp";
		break;
	default:
		return "";
		break;
	}
}

static std::string getBuildingName(BuildingType type)
{
	switch (type)
	{
	case NONE:
		break;
	case DEPOT:
		break;
	case TREE:
		break;
	case ROCK:
		break;
	case GOLD:
		break;
	case WORKSHOP:
		return "Workstation";
		break;
	case FORGE:
		return "Forge";
		break;
	case ANVIL:
		return "Anvil";
		break;
	case DUMMY:
		return "TrainingDummy";
		break;
	default:
		return "";
		break;
	}
}