#include "Colonist.h"

Colonist::Colonist()
{
	mStats = new ColonistStats();
	mInventory = new ColonistInventory();
}

Colonist::Colonist(ColonistStats stats)
{
	mStats = new ColonistStats();
	mStats->hp = stats.hp;
	mStats->hunger = stats.hunger;
	mStats->combat = stats.combat;
	mStats->chopping = stats.chopping;
	mStats->mining = stats.mining;
	mInventory = new ColonistInventory();
}

Colonist::~Colonist()
{
}

void Colonist::Update(float deltaTime)
{
	if (mCurrentCommand == CommandType::Move)
	{
		glm::vec3 direction = glm::normalize(*mTarget->position - mGOColonist->mesh->position);
		float distance = glm::length(*mTarget->position - mGOColonist->mesh->position);
		float speed = 1.0f; 

		if (distance > 0.1f) 
		{
			// Move the colonist towards the target 
			glm::vec3 dir = direction * speed * deltaTime;
			mCharacterController->Move(dir);
		}
		else
		{
			// Reset since its reached the target
			mCurrentCommand = CommandType::None;
		}
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
	case ActionType::Move:
	{
		// Move the colonist to the target position
		glm::vec3 direction = glm::normalize(*mTarget->position - mGOColonist->mesh->position);
		mCharacterController->Move(direction);
		break;
	}
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