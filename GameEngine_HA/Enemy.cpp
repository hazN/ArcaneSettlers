#include "Enemy.h"
#include "TerrainManager.h"
#include "quaternion_utils.h"
#include <iostream>

Enemy::Enemy()
{
	mStats = new EnemyStats();
	lastFlowFieldTarget = glm::vec3(-5000.f);
	InitializeCriticalSection(&mStatsCriticalSection);
}

Enemy::Enemy(EnemyStats stats)
{
	mStats = new EnemyStats;
	mStats->hp = stats.hp;
	mStats->combat = stats.combat;
	InitializeCriticalSection(&mStatsCriticalSection);
}

Enemy::~Enemy()
{
	DeleteCriticalSection(&mStatsCriticalSection);
}

void Enemy::Update(float deltaTime) {
	EnemyActionType action = mDecisionTable.getNextAction(*this);

	switch (action) {
	case EnemyActionType::AttackColonist:
		currentAction = "Attacking...";
		if (mColonistTarget != nullptr)
		{
			float distance = glm::distance(mColonistTarget->mGOColonist->mesh->position, this->mGOEnemy->mesh->position);
			if (distance <= 2.f)
			{
				Attack();
			}
			else
			{
				// Set animation
				if (this->mGOEnemy->animCharacter->GetCurrentAnimationID() != 10)
					this->mGOEnemy->animCharacter->SetAnimation(10);
				// Move towards the colonist
				glm::vec2 direction = glm::vec2(mColonistTarget->mGOColonist->mesh->position.x, mColonistTarget->mGOColonist->mesh->position.z) - glm::vec2(mGOEnemy->mesh->position.x, mGOEnemy->mesh->position.z);
				glm::vec2 moveDirection = glm::normalize(direction);
				float speed = 1.0f;
				float deltaTime = 0.1f;
				glm::vec3 dir = glm::vec3(moveDirection.x, 0.5f, moveDirection.y) * speed * deltaTime;
				mCharacterController->Move(dir);
				glm::vec3 lookDir = glm::normalize(glm::vec3(moveDirection.x, -mGOEnemy->mesh->position.y, moveDirection.y));
				glm::quat targetDir = q_utils::LookAt(lookDir, glm::vec3(0, 1, 0));
				if (std::isnan(mGOEnemy->mesh->qRotation.x))
					mGOEnemy->mesh->qRotation = glm::quat();
				mGOEnemy->mesh->qRotation = q_utils::RotateTowards(mGOEnemy->mesh->qRotation, targetDir, 3.14f * 0.05f);
			}
		}
		break;
	case EnemyActionType::AttackBuilding:
		currentAction = "Attacking...";
		if (mGOTarget != nullptr)
		{
			if (glm::distance(mGOTarget->mesh->position, this->mGOEnemy->mesh->position) <= 10.f)
			{
				Attack();
			}
		}
		break;
	case EnemyActionType::Move:
		currentAction = "Moving...";
		Move();
		break;
	default:
		currentAction = "Idle...";
		break;
	}
}

void Enemy::Attack()
{
	if (this->mGOEnemy->animCharacter->GetCurrentAnimationID() != 13)
		this->mGOEnemy->animCharacter->SetAnimation(13);
	float duration = (clock() - attackTime) / (double)CLOCKS_PER_SEC;

	if (duration >= 0.25f)
	{
		attackTime = clock();
		if (mColonistTarget != nullptr)
		{
			float baseDamage = mStats->combat;
			float damage = (baseDamage * (1.0f + 0.2f * (rand() / (float)(RAND_MAX) * 2.0f - 1.0f))) * 0.7f;
			mColonistTarget->TakeDamage(damage);
			if (mColonistTarget->isDead)
			{
				mColonistTarget = nullptr;
				return;
			}
		}
		else if (mGOTarget != nullptr)
		{
			float baseDamage = mStats->combat;
			float damage = (baseDamage * (1.0f + 0.2f * (rand() / (float)(RAND_MAX) * 2.0f - 1.0f))) * 0.7f;
			int count = 0;
			for (Building building : buildingManager->mPlayerBuildings)
			{
				if (building.mId == mGOTarget->id)
				{
					building.mHealth -= damage;

					break;
				}
				count++;
			}
			if (buildingManager->mPlayerBuildings[count].mHealth <= 0)
			{
				gPathFinder->removeBuilding(mGOTarget->id);
				buildingManager->mPlayerBuildings.erase(buildingManager->mPlayerBuildings.begin() + count);
				mGOTarget = nullptr;
				return;
			}
		}
	}
}


void Enemy::Move()
{
	if (mFlowfield.empty() || mGOTarget == nullptr)
	{
		return;
	}
	// Check if target is occupied
	for (size_t i = 0; i < vecEnemies.size(); i++)
	{
		if (i < vecEnemies.size())
		{
			if (vecEnemies[i] != this)
			{
				float distance = glm::length(mGOTarget->mesh->position - vecEnemies[i]->mGOEnemy->mesh->position);
				float distance2 = glm::length(this->mGOEnemy->mesh->position - vecEnemies[i]->mGOEnemy->mesh->position);

				if (distance <= 1.3f && distance2 <= 2.f)
				{
					// Set animation
					if (this->mGOEnemy->animCharacter->GetCurrentAnimationID() != 1)
						this->mGOEnemy->animCharacter->SetAnimation(1);
					return;
				}
			}
		}
	}

	// Set animation
	if (this->mGOEnemy->animCharacter->GetCurrentAnimationID() != 10)
		this->mGOEnemy->animCharacter->SetAnimation(10);

	// Get current position and convert to grid coord
	glm::vec3 currentPos = mGOEnemy->mesh->position;
	glm::vec2 currentGridPos = TerrainManager::worldToGridCoords(currentPos);

	// Get the direction from the flowfield
	glm::vec2 flowDirection = mFlowfield[currentGridPos.y][currentGridPos.x];

	glm::vec3 moveDirection = glm::vec3(flowDirection.x, 0.5f, flowDirection.y);

	// Move the colonist
	float speed = 1.2f;
	float deltaTime = 0.1f;
	glm::vec3 dir = moveDirection * speed * deltaTime;
	mCharacterController->Move(dir);
	glm::vec3 lookDir = glm::normalize(glm::vec3(moveDirection.x, -mGOEnemy->mesh->position.y, moveDirection.z));
	glm::quat targetDir = q_utils::LookAt(lookDir, glm::vec3(0, 1, 0));
	if (std::isnan(mGOEnemy->mesh->qRotation.x))
		mGOEnemy->mesh->qRotation = glm::quat();
	mGOEnemy->mesh->qRotation = q_utils::RotateTowards(mGOEnemy->mesh->qRotation, targetDir, 3.14f * 0.05f);
}

void Enemy::TakeDamage(float dmg)
{
	EnterCriticalSection(&mStatsCriticalSection);
	mStats->hp -= dmg;
	if (mStats->hp <= 0)
	{
		isDead = true;
		this->mGOEnemy->animCharacter->SetAnimation(1);
		this->mGOEnemy->animCharacter->m_IsLooping = false;
		exitThread = true;
	}
	LeaveCriticalSection(&mStatsCriticalSection);
}

DWORD WINAPI UpdateEnemyThread(LPVOID pVOIDEnemy) {
	EnemyThreadData* pThreadData = static_cast<EnemyThreadData*>(pVOIDEnemy);

	while (!pThreadData->bExitThread && !pThreadData->pEnemy->exitThread) {
		if (!pThreadData->bSuspendThread) {
			pThreadData->pEnemy->Update((float)(pThreadData->suspendTime_ms) / 1000.0f);
		}
		Sleep(pThreadData->suspendTime_ms);
	}

	return 0;
}