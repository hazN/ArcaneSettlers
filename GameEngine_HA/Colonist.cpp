#include "Colonist.h"
#include "quaternion_utils.h"
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
	case ActionType::Move: {
		// Move the colonist towards the target
		glm::vec3 direction = glm::normalize(*mTarget->position - mGOColonist->mesh->position);
		float distance = glm::length(*mTarget->position - mGOColonist->mesh->position);
		float speed = 1.0f;

		if (glm::length(*mTarget->position - mGOColonist->mesh->position) >= 0.2f) 
		{
			glm::vec3 dir = direction * speed * deltaTime;
			mCharacterController->Move(dir);
			glm::vec3 lookDir = glm::normalize(glm::vec3(mTarget->position->x, mGOColonist->mesh->position.y + 100.f, mTarget->position->z) - mGOColonist->mesh->position);
			glm::quat targetDir = q_utils::LookAt(lookDir, glm::vec3(0, 1, 0));
			mGOColonist->mesh->qRotation = q_utils::RotateTowards(mGOColonist->mesh->qRotation, targetDir, 3.14f * 0.005f);
		}
		else
		{
			ExecuteCommand();
		}
		break;
	}
	case ActionType::HarvestTree:
	case ActionType::HarvestRock:
	case ActionType::AttackIntruder: {
		// Check if the target is in range
		if (glm::length(*mTarget->position - mGOColonist->mesh->position) <= 0.2f) 
		{
			ExecuteCommand(); 
		}
		else 
		{
			// Otherwise keep moving
			SetCommand(CommandType::Move, mTarget);
		}
		break;
	}
	default:
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
		break;
	case ActionType::AttackIntruder:
		break;
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
			wood.icon = "assets/icons/wood.png";
			wood.id = itemId::wood;
			wood.name = "Wood";
			wood.weight = 2;
			mInventory->addItem(wood, actualHarvestedWood);

			deltaTime = clock();
		}
	}
	else {
		// Once tree is fully harvested then reset command
		mCurrentCommand = CommandType::None;
		// Run a level up check
		if (mStats->chopping < 20) {
			float levelUpChance = 0.01f * ((20 - mStats->chopping) * (20 - mStats->chopping));
			if (rand() < levelUpChance * RAND_MAX) {
				mStats->chopping += 1;
			}
		}
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