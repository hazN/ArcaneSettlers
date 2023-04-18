#include "Colonist.h"
#include "quaternion_utils.h"
#include "globalThings.h"
#include <iostream>
#include "TerrainManager.h"
Colonist::Colonist()
{
	mStats = new ColonistStats();
	mInventory = new Inventory();
	InitializeCriticalSection(&mStatsCriticalSection);
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
	InitializeCriticalSection(&mStatsCriticalSection);
}

Colonist::~Colonist()
{
	DeleteCriticalSection(&mStatsCriticalSection);
}

void Colonist::Update(float deltaTime) {
	if (isDead) return;
	// Hunger decrease
	if (rand() < 0.01f * RAND_MAX) {
		mStats->hunger -= 0.000139f;
	}
	// Health regeneration
	if (mStats->hp < 100.f)
	{
		if (rand() < 0.01f * RAND_MAX) {
			mStats->hp += 1.f;
		}
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
	{
		if (this->mGOColonist->animCharacter->GetCurrentAnimationID() != 11)
			this->mGOColonist->animCharacter->SetAnimation(11);
		ExecuteCommand();
		break;
	}
	case ActionType::AttackIntruder:
	{

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
		currentAction = "In combat...";
		Attack();
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

bool Colonist::isHungry() {
	return mStats->hunger < 30;
}

bool Colonist::getIsIntruderInRange()
{
	for (Enemy* enemy : vecEnemies)
	{
		if (glm::distance(this->mGOColonist->mesh->position, enemy->mGOEnemy->mesh->position) <= 30.f)
		{
			mEnemyTarget = enemy;
			return true;
		}
	}
	mEnemyTarget = nullptr;
	return false;
}

void Colonist::TakeDamage(float dmg)
{
	EnterCriticalSection(&mStatsCriticalSection);
	mStats->hp -= dmg;
	if (mStats->hp <= 0)
	{
		isDead = true;
		this->mGOColonist->animCharacter->SetAnimation(0);
		this->mGOColonist->animCharacter->m_IsLooping = false;
		this->exitThread = true;
	}
	LeaveCriticalSection(&mStatsCriticalSection);
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
	// Check if target is occupied
	for (size_t i = 0; i < vecColonists.size(); i++)
	{
		if (vecColonists[i] != this)
		{
			float distance = glm::length(*mTarget->position - vecColonists[i]->mGOColonist->mesh->position);
			float distance2 = glm::length(this->mGOColonist->mesh->position - vecColonists[i]->mGOColonist->mesh->position);

			if (distance <= 1.3f && distance2 <= 2.f)
			{
				// Set animation
				if (this->mGOColonist->animCharacter->GetCurrentAnimationID() != 1)
					this->mGOColonist->animCharacter->SetAnimation(1);
				return;
			}
		}
	}

	// Set animation
	if (this->mGOColonist->animCharacter->GetCurrentAnimationID() != 10)
		this->mGOColonist->animCharacter->SetAnimation(10);

	// Get current position and convert to grid coord
	glm::vec3 currentPos = mGOColonist->mesh->position;
	glm::vec2 currentGridPos = TerrainManager::worldToGridCoords(currentPos);
	currentGridPos = glm::round(currentGridPos);

	// Get the direction from the flowfield
	glm::vec2 flowDirection = flowDirection = mFlowfield[currentGridPos.y][currentGridPos.x];

	glm::vec3 moveDirection = moveDirection = glm::vec3(flowDirection.x, 0.5f, flowDirection.y);

	// Move the colonist
	float speed = 1.4f;
	float deltaTime = 0.1f;
	glm::vec3 dir = moveDirection * speed * deltaTime;
	mCharacterController->Move(dir);
	glm::vec3 lookDir = glm::normalize(glm::vec3(moveDirection.x, -mGOColonist->mesh->position.y, moveDirection.z));
	glm::quat targetDir = q_utils::LookAt(lookDir, glm::vec3(0, 1, 0));
	if (std::isnan(mGOColonist->mesh->qRotation.x))
		mGOColonist->mesh->qRotation = glm::quat();
	mGOColonist->mesh->qRotation = q_utils::RotateTowards(mGOColonist->mesh->qRotation, targetDir, 3.14f * 0.05f);
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
			gPathFinder->removeBuilding(mTarget->id);
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
			gPathFinder->removeBuilding(mTarget->id);
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
	else if (glm::length(*mTarget->position - mGOColonist->mesh->position) <= 10.f)
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

void Colonist::Attack()
{
	if (glm::distance(mEnemyTarget->mGOEnemy->mesh->position, mGOColonist->mesh->position) >= 3.1f)
	{
		// Set animation
		if (this->mGOColonist->animCharacter->GetCurrentAnimationID() != 10)
			this->mGOColonist->animCharacter->SetAnimation(10);
		// Move towards the colonist
		glm::vec2 direction = glm::vec2(mGOColonist->mesh->position.x, mEnemyTarget->mGOEnemy->mesh->position.z) - glm::vec2(mGOColonist->mesh->position.x, mGOColonist->mesh->position.z);
		glm::vec2 moveDirection = glm::normalize(direction);
		float speed = 1.0f;
		float deltaTime = 0.1f;
		glm::vec3 dir = glm::vec3(moveDirection.x, 0.5f, moveDirection.y) * speed * deltaTime;
		mCharacterController->Move(dir);
		glm::vec3 lookDir = glm::normalize(glm::vec3(moveDirection.x, -mGOColonist->mesh->position.y, moveDirection.y));
		glm::quat targetDir = q_utils::LookAt(lookDir, glm::vec3(0, 1, 0));
		if (std::isnan(mGOColonist->mesh->qRotation.x))
			mGOColonist->mesh->qRotation = glm::quat();
		mGOColonist->mesh->qRotation = q_utils::RotateTowards(mGOColonist->mesh->qRotation, targetDir, 3.14f * 0.05f);
	}
	float duration = (clock() - attackTime) / (double)CLOCKS_PER_SEC;
	if (this->mGOColonist->animCharacter->GetCurrentAnimationID() != 11)
		this->mGOColonist->animCharacter->SetAnimation(11);
	if (duration >= 0.25f)
	{
		attackTime = clock();
		if (mEnemyTarget != nullptr)
		{
			float baseDamage = mStats->combat;
			float damage = (baseDamage * (1.0f + 0.2f * (rand() / (float)(RAND_MAX) * 2.0f - 1.0f))) * 0.7f;
			damage += buildingManager->getModifierForType(DUMMY);
			mEnemyTarget->TakeDamage(damage);
			if (mEnemyTarget->isDead)
			{
				mEnemyTarget = nullptr;
				getIsIntruderInRange();
				return;
			}
		}
		// Run a level up check for combat skill
		if (mStats->combat < 20) {
			float levelUpChance = 0.0001f * ((20 - mStats->mining) * (20 - mStats->mining));
			if (rand() < levelUpChance * RAND_MAX) {
				mStats->mining += 1;
			}
		}
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
	while (!pColonistData->bExitThread && !pColonistData->pColonist->exitThread) {
		if (!pColonistData->bSuspendThread) {
			pColonistData->pColonist->Update(1.0f / 60.0f);
		}
		Sleep(pColonistData->suspendTime_ms);
	}

	return 0;
}