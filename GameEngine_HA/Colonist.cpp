#include "Colonist.h"
#include "quaternion_utils.h"
#include "globalThings.h"
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
	ActionType action = mDecisionTable.getNextAction(*this);

	switch (action) {
	case ActionType::DropOffLoot:
	{
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
		else if (glm::length(*mTarget->position - mGOColonist->mesh->position) <= 3.5f)
		{
			ExecuteCommand();
			break;
		}
	}
	case ActionType::Move: {
		// Set animation
		if (this->mGOColonist->animCharacter->GetCurrentAnimationID() != 10)
			this->mGOColonist->animCharacter->SetAnimation(10);
		// Move the colonist towards the target
		glm::vec3 direction = glm::normalize(*mTarget->position - mGOColonist->mesh->position);
		float distance = glm::length(*mTarget->position - mGOColonist->mesh->position);
		float speed = 1.0f;

		if (mTarget->buildingType != NULL)
		{
			if (distance <= 2.6)
			{
				ExecuteCommand();
				break;
			}
		}
		if (distance >= 0.2f)
		{
			glm::vec3 dir = direction * speed * deltaTime;
			mCharacterController->Move(dir);
			glm::vec3 lookDir = glm::normalize(glm::vec3(mTarget->position->x, mGOColonist->mesh->position.y + 100.f, mTarget->position->z) - mGOColonist->mesh->position);
			glm::quat targetDir = q_utils::LookAt(lookDir, glm::vec3(0, 1, 0));
			mGOColonist->mesh->qRotation = q_utils::RotateTowards(mGOColonist->mesh->qRotation, targetDir, 3.14f * 0.005f);
		}
		else
		{
			if (mCurrentCommand == CommandType::Move)
			{
				mCurrentCommand = CommandType::None;
			}
			ExecuteCommand();
		}
		break;
	}
	case ActionType::HarvestTree:
	case ActionType::HarvestRock:
	case ActionType::AttackIntruder:
	{
		if (this->mGOColonist->animCharacter->GetCurrentAnimationID() != 11)
			this->mGOColonist->animCharacter->SetAnimation(11);
		// Check if the target is in range
		if (glm::length(*mTarget->position - mGOColonist->mesh->position) <= 3.5f)
		{
			ExecuteCommand();
		}
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
		HarvestTree();
	}
	break;
	case ActionType::HarvestRock:
	{
		MineNode();
	}
	break;
	case ActionType::AttackIntruder:
		break;
	case ActionType::DropOffLoot:
	{
		DropOffLoot();
	}
	default:
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

void Colonist::HarvestTree()
{
	if (mInventory->getCurrentWeight() >= mInventory->getMaxWeight() || (mTarget->inventory->getItemCount(itemId::wood) <= 0))
	{
		mCurrentCommand = CommandType::None;
		mTarget = nullptr;
		return;
	}
	// Check if tree has wood
	if (mTarget->inventory->getItemCount(itemId::wood) > 0) {
		duration = (clock() - deltaTime) / (double)CLOCKS_PER_SEC;
		if (duration > 0.25f)
		{
			// Formula to calculate efficiency, based off Smite's dmg
			const float X = 100.0f;
			float efficiencyMultiplier = std::max(1.0f, (float)mStats->chopping / 2.0f);
			float woodToHarvest = 1 * efficiencyMultiplier;
			// May not get back the amount we tried to harvest, ie: chopping for 3 when tree only has 2 wood left
			int actualHarvestedWood = mTarget->inventory->removeItem(itemId::wood, (int)glm::floor(woodToHarvest));

			Item wood;
			wood.icon = "assets/icons/Wood.png";
			wood.id = itemId::wood;
			wood.name = "Wood";
			wood.weight = 2;
			mInventory->addItem(wood, actualHarvestedWood);

			deltaTime = clock();
			// Run a level up check
			if (mStats->chopping < 20) {
				float levelUpChance = 0.01f * ((20 - mStats->chopping) * (20 - mStats->chopping));
				if (rand() < levelUpChance * RAND_MAX) {
					mStats->chopping += 1;
				}
			}
		}
	}
}

void Colonist::MineNode() {
	// Check if the target has stone or ores
	bool hasStone = mTarget->inventory->getItemCount(itemId::stone) > 0;
	bool hasOres = mTarget->inventory->getItemCount(itemId::ores) > 0;
	if (mInventory->getCurrentWeight() >= mInventory->getMaxWeight() || (!hasStone && !hasOres))
	{
		mCurrentCommand = CommandType::None;
		mTarget = nullptr;
		return;
	}
	if (hasStone || hasOres)
	{
		duration = (clock() - deltaTime) / (double)CLOCKS_PER_SEC;
		if (duration > 0.25f)
		{
			// Formula to calculate efficiency, based off Smite's dmg
			const float X = 100.0f;
			float efficiencyMultiplier = std::max(1.0f, (float)mStats->mining / 2.0f);
			float itemsToMine = 1 * efficiencyMultiplier;

			// Create the stone/ore
			Item minedItem;
			itemId minedItemId;
			if (hasStone)
			{
				minedItemId = itemId::stone;
				minedItem.icon = "assets/icons/Stone.png";
				minedItem.name = "Stone";
				minedItem.weight = 2;
			}
			else if (hasOres)
			{
				minedItemId = itemId::ores;
				minedItem.icon = "assets/icons/Minerals.png";
				minedItem.name = "Ore";
				minedItem.weight = 4;
			}

			// May not get back the amount we tried to harvest, ie: mining for 3 stone when the node only has 2 stone left
			int actualMinedItems = mTarget->inventory->removeItem(minedItemId, (int)glm::floor(itemsToMine));
			mInventory->addItem(minedItem, actualMinedItems);

			deltaTime = clock();

			// Run a level up check for mining skill
			if (mStats->mining < 20) {
				float levelUpChance = 0.01f * ((20 - mStats->mining) * (20 - mStats->mining));
				if (rand() < levelUpChance * RAND_MAX) {
					mStats->mining += 1;
				}
			}
		}
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
	else if (glm::length(*mTarget->position - mGOColonist->mesh->position) <= 3.5f)
	{
		for (Item item : mInventory->getAllItems())
		{
			mInventory->removeItem(item.id);
			mTarget->inventory->addItem(item);
		}
		mCurrentCommand = CommandType::None;
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