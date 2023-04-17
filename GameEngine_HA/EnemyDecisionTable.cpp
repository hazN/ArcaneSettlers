#include "EnemyDecisionTable.h"
#include "Enemy.h"
#include "globalThings.h"

EnemyDecisionTable::EnemyDecisionTable()
{
	decisionTable =
	{
		{ {true, std::nullopt}, EnemyActionType::AttackColonist },
		{ {std::nullopt, true}, EnemyActionType::AttackBuilding },
		{ {false, std::nullopt}, EnemyActionType::Move },
		{ {std::nullopt, false}, EnemyActionType::Idle },
	};
}

EnemyDecisionTable::~EnemyDecisionTable()
{
}

EnemyActionType EnemyDecisionTable::getNextAction(Enemy& enemy)
{
	EnemyCondition currentCondition;

	// Check if the current target is still alive/in range
	bool colonistInRange = false;
	float range = 30.f;
	if (enemy.mColonistTarget != nullptr)
	{
		float distance = glm::distance(enemy.mColonistTarget->mGOColonist->mesh->position, enemy.mGOEnemy->mesh->position);
		if (distance <= range)
			colonistInRange = false;
		else enemy.mColonistTarget = nullptr;
	}
	// Now check if any colonist is in range
	if (!colonistInRange) {
		for (Colonist* colonist : vecColonists) 
		{
			float distance = glm::distance(colonist->mGOColonist->mesh->position, enemy.mGOEnemy->mesh->position);
			if (distance <= range) {
				colonistInRange = true;
				enemy.mColonistTarget = colonist;
				break;
			}
		}
	}
	currentCondition.isColonistInRange = colonistInRange;

	// Check if any building is in range
	bool buildingInRange = false;
	if (enemy.mColonistTarget == nullptr)
	{
		// Default to the Depot 
		Building closestBuilding(gDepot->id, getBuildingName(DEPOT), getBuildingIcon(DEPOT), DEPOT);
		closestBuilding.mPosition = gDepot->mesh->position;
		// Check the closest building
		for (Building building : buildingManager->mPlayerBuildings)
		{
			float distance = glm::distance(building.mPosition, enemy.mGOEnemy->mesh->position);
			float otherDistance = glm::distance(closestBuilding.mPosition, enemy.mGOEnemy->mesh->position);
			if (distance < otherDistance)
			{
				closestBuilding = building;
			}
		}
		enemy.mGOTarget = goMap[closestBuilding.mId];
	} 
	else
	{
		enemy.mGOTarget = gDepot;
	}
	float distance = glm::distance(enemy.mGOTarget->mesh->position, enemy.mGOEnemy->mesh->position);
	currentCondition.isBuildingInRange = distance < 10.f;

	for (const EnemyRule& rule : decisionTable) {
		if (rule.condition == currentCondition) {
			return rule.action;
		}
	}

	return EnemyActionType::Idle;
}