#include "Colonist.h"
#include "quaternion_utils.h"
#include "globalThings.h"
#include <iostream>
#include "TerrainManager.h"
Colonist::Colonist()
{
	mStats = new ColonistStats();
	mInventory = new Inventory();
}

Colonist::Colonist(ColonistStats stats)
{
	mStats = new ColonistStats();
	mStats->hp = stats.hp;
	mStats->hunger = stats.hunger;
	mStats->combat = stats.combat;
	mStats->chopping = stats.chopping;
	mStats->mining = stats.mining;
	mInventory = new Inventory();
}

Colonist::~Colonist()
{
}

void Colonist::Update(float deltaTime) {
	// Hunger decrease
	if (rand() < 0.01f * RAND_MAX) {
		mStats->hunger -= 0.000139f;
	}

	ActionType action = mDecisionTable.getNextAction(*this);

	switch (action) {
	case ActionType::Eat:
	{
		currentAction = "Going to eat...";
		// Check if the depot is in range
		if (mTarget == nullptr)
		{
			for (std::pair<int, GameObject*> go : goMap)
			{
				if (go.second->buildingType == DEPOT)
				{
					mTarget = go.second;
				}
			}
			action = ActionType::Move;
		}
		else if (mTarget->buildingType != DEPOT)
		{
			for (std::pair<int, GameObject*> go : goMap)
			{
				if (go.second->buildingType == DEPOT)
				{
					mTarget = go.second;
				}
			}
			action = ActionType::Move;
		}
		else if (glm::length(*mTarget->position - mGOColonist->mesh->position) <= 3.8f)
		{
			ExecuteCommand();
			break;
		}
	}
	case ActionType::DropOffLoot:
	{
		currentAction = "Dropping off Loot...";
		// Check if the depot is in range
		if (mTarget == nullptr)
		{
			for (std::pair<int, GameObject*> go : goMap)
			{
				if (go.second->buildingType == DEPOT)
				{
					mTarget = go.second;
				}
			}
			action = ActionType::Move;
		}
		else if (mTarget->buildingType != DEPOT)
		{
			for (std::pair<int, GameObject*> go : goMap)
			{
				if (go.second->buildingType == DEPOT)
				{
					mTarget = go.second;
				}
			}
			action = ActionType::Move;
		}
		else if (glm::length(*mTarget->position - mGOColonist->mesh->position) <= 4.f)
		{
			ExecuteCommand();
			break;
		}
		else
		{
			if (mFlowfield.empty())
			{

			}
			Move();
		}
		break;
	}
	case ActionType::Move: {
		currentAction = "Moving...";
		// Set animation
		if (this->mGOColonist->animCharacter->GetCurrentAnimationID() != 10)
			this->mGOColonist->animCharacter->SetAnimation(10);


		float distance = glm::length(*mTarget->position - mGOColonist->mesh->position);

		// Check if its reached its target
		if (mTarget->buildingType != NULL)
		{
			if (distance <= 2.6 || (mTarget->mesh->meshName == "Gold" && distance <= 5.f))
			{
				ExecuteCommand();
				break;
			}
			else
			{
				// Move the colonist 
				Move();
			}
		}
		else if (distance < 0.2f)
		{
			if (mCurrentCommand == CommandType::Move)
			{
				mCurrentCommand = CommandType::None;
			}
			ExecuteCommand();
		}
		else 
		{
			// Move the colonist 
			Move();
		}
		break;
	}
	case ActionType::HarvestTree:
	case ActionType::HarvestRock:
	case ActionType::AttackIntruder:
	{
		if (this->mGOColonist->animCharacter->GetCurrentAnimationID() != 11)
			this->mGOColonist->animCharacter->SetAnimation(11);
		ExecuteCommand();
		break;
	}
	default:
	{
		if (this->mGOColonist->animCharacter->GetCurrentAnimationID() != 1)
			this->mGOColonist->animCharacter->SetAnimation(1);
		mCurrentCommand = CommandType::None;
	}
	break;
	}
}

void Colonist::SetCommand(CommandType command, GameObject* target)
{
	mCurrentCommand = command;
	mTarget = target;
}

void Colonist::ExecuteCommand()
{
	ActionType action = mDecisionTable.getNextAction(*this);
	switch (action) {
	case ActionType::HarvestTree:
	{
		currentAction = "Chopping wood...";
		HarvestTree();
	}
	break;
	case ActionType::HarvestRock:
	{
		currentAction = "Mining node...";
		MineNode();
	}
	break;
	case ActionType::AttackIntruder:
		break;
	case ActionType::DropOffLoot:
	{
		currentAction = "Dropping off loot...";
		DropOffLoot();
	}
	case ActionType::Eat:
	{
		currentAction = "Eating...";
		Eat();
		break;
	}
	default:
		currentAction = "Idle...";
		break;
	}
}

void Colonist::UpdateDecisionTable()
{
}

bool Colonist::isHungry() {
	return mStats->hunger < 30;
}

bool Colonist::getIsIntruderInRange()
{
	return false;
}

ColonistStats Colonist::getStats()
{
	return *mStats;
}

void Colonist::Move()
{
	if (mFlowfield.empty())
	{
		return;
	}

	// Get current position and convert to grid coord
	glm::vec3 currentPos = mGOColonist->mesh->position;
	glm::vec2 currentGridPos = TerrainManager::worldToGridCoords(currentPos);
	currentGridPos = glm::round(currentGridPos);

	// Get the direction from the flowfield
	glm::vec2 flowDirection = mFlowfield[currentGridPos.y][currentGridPos.x];
	glm::vec3 moveDirection = glm::vec3(flowDirection.x, 0.5f, flowDirection.y);

	// Move the colonist
	float speed = 1.0f;
	float deltaTime = 0.1f;
	glm::vec3 dir = moveDirection * speed * deltaTime;
	mCharacterController->Move(dir);
	glm::vec3 lookDir = glm::normalize(moveDirection);
	glm::quat targetDir = q_utils::LookAt(lookDir, glm::vec3(0, 1, 0));
	//mGOColonist->mesh->qRotation = q_utils::RotateTowards(mGOColonist->mesh->qRotation, targetDir, 3.14f * 0.005f);

	float distance = glm::length(*mTarget->position - mGOColonist->mesh->position);
	if (distance < 1.1f)
	{
		if (mCurrentCommand == CommandType::Move)
		{
			mCurrentCommand = CommandType::None;
		}
	}
}

void Colonist::HarvestTree()
{
	// Check if tree has wood
	if (mTarget->inventory->getItemCount(itemId::wood) > 0) {
		duration = (clock() - deltaTime) / (double)CLOCKS_PER_SEC;
		if (duration > 0.5f)
		{
			// Formula to calculate efficiency, based off Smite's dmg
			const float X = 100.0f;
			float efficiencyMultiplier = std::max(1.0f, (float)mStats->chopping / 2.0f);
			float woodToHarvest = 1 * efficiencyMultiplier;
			// May not get back the amount we tried to harvest, ie: chopping for 3 when tree only has 2 wood left
			int actualHarvestedWood = mTarget->inventory->removeItem(itemId::wood, (int)glm::floor(woodToHarvest));

			Item wood;
			wood.icon = "Wood.bmp";
			wood.id = itemId::wood;
			wood.name = "Wood";
			wood.weight = 2;
			mInventory->addItem(wood, actualHarvestedWood);

			deltaTime = clock();
			// Run a level up check
			if (mStats->chopping < 20) {
				float levelUpChance = 0.001f * ((20 - mStats->chopping) * (20 - mStats->chopping));
				if (rand() < levelUpChance * RAND_MAX) {
					mStats->chopping += 1;
				}
			}
		}
	}
	if (mInventory->getCurrentWeight() >= mInventory->getMaxWeight() || (mTarget->inventory->getItemCount(itemId::wood) <= 0))
	{
		mCurrentCommand = CommandType::None;
		if (mTarget->inventory->getItemCount(itemId::wood) <= 0)
		{
			mTarget->mesh->bIsVisible = false;
			world->RemoveBody(mTarget->rigidBody);
			goMap.erase(mTarget->id);
			delete mTarget;
		}
		mTarget = nullptr;
		return;
	}
}

void Colonist::MineNode() {
	// Check if the target has stone or ores
	bool hasStone = mTarget->inventory->getItemCount(itemId::stone) > 0;
	bool hasOres = mTarget->inventory->getItemCount(itemId::ores) > 0;

	if (hasStone || hasOres)
	{
		duration = (clock() - deltaTime) / (double)CLOCKS_PER_SEC;
		if (duration > 0.5f)
		{
			// Formula to calculate efficiency, based off Smite's dmg
			const float X = 100.0f;
			float efficiencyMultiplier = std::max(1.0f, (float)mStats->mining / 2.0f);
			float itemsToMine = 1 * efficiencyMultiplier;

			// Create the stone/ore
			Item minedItem;
			if (hasStone)
			{
				minedItem.id = stone;
				minedItem.icon = "Stone.bmp";
				minedItem.name = "Stone";
				minedItem.weight = 2;
			}
			else if (hasOres)
			{
				minedItem.id = ores;
				minedItem.icon = "Minerals.bmp";
				minedItem.name = "Ore";
				minedItem.weight = 4;
			}

			// May not get back the amount we tried to harvest, ie: mining for 3 stone when the node only has 2 stone left
			int actualMinedItems = mTarget->inventory->removeItem(minedItem.id, (int)glm::floor(itemsToMine));
			mInventory->addItem(minedItem, actualMinedItems);

			deltaTime = clock();

			// Run a level up check for mining skill
			if (mStats->mining < 20) {
				float levelUpChance = 0.001f * ((20 - mStats->mining) * (20 - mStats->mining));
				if (rand() < levelUpChance * RAND_MAX) {
					mStats->mining += 1;
				}
			}
		}
	}
	if (mInventory->getCurrentWeight() >= mInventory->getMaxWeight() || (!hasStone && !hasOres))
	{
		mCurrentCommand = CommandType::None;
		if (!hasStone && !hasOres)
		{
			mTarget->mesh->bIsVisible = false;
			world->RemoveBody(mTarget->rigidBody);
			goMap.erase(mTarget->id);
			delete mTarget;
		}
		mTarget = nullptr;
		return;
	}
}

void Colonist::DropOffLoot()
{
	// Check if the depot is in range
	if (mTarget == nullptr)
	{
		for (std::pair<int, GameObject*> go : goMap)
		{
			if (go.second->buildingType == DEPOT)
			{
				mTarget = go.second;
				return;
			}
		}
	}
	else if (mTarget->buildingType != DEPOT)
	{
		for (std::pair<int, GameObject*> go : goMap)
		{
			if (go.second->buildingType == DEPOT)
			{
				mTarget = go.second;
				return;
			}
		}
	}
	else if (glm::length(*mTarget->position - mGOColonist->mesh->position) <= 4.f)
	{
		currentAction = "Unloading inventory...";
		for (Item item : mInventory->getAllItems())
		{
			mInventory->removeItem(item.id);
			mTarget->inventory->addItem(item);
			Sleep(250);
		}
		mCurrentCommand = CommandType::None;
	}
}

void Colonist::Eat()
{
	if (mTarget->inventory->getItemCount(itemId::food) > 0) {
		mTarget->inventory->removeItem(itemId::food, 1);
		mStats->hunger = 100;
		mCurrentCommand = CommandType::None;
	}
	else {
		std::cout << "One of your colonists is hungry and there is no food!" << std::endl;
	}
}

DWORD WINAPI UpdateColonistThread(LPVOID pVOIDColonistData)
{
	ColonistThreadData* pColonistData = (ColonistThreadData*)(pVOIDColonistData);

	while (!pColonistData->bExitThread) {
		if (!pColonistData->bSuspendThread) {
			pColonistData->pColonist->Update(1.0f / 60.0f);
		}
		Sleep(pColonistData->suspendTime_ms);
	}

	return 0;
}