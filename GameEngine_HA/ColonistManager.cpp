#include "ColonistManager.h"
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

ColonistManager::ColonistManager()
{
	InitializeCriticalSection(&targetsCriticalSection);
	// Create flowfield for the depot
	FlowFieldThreadData flowFieldData;
	flowFieldData.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	// Call the async flow field function
	glm::vec2 targetPos = TerrainManager::worldToGridCoords(*gDepot->position);
	gPathFinder->calculateFlowfieldAsync(targetPos, &flowFieldData);
	// Wait for it to finish
	WaitForSingleObject(flowFieldData.hEvent, INFINITE);
	// Give the colonist the action
	CloseHandle(flowFieldData.hEvent);

	targetFlowfield* newTargetFlowfield = new targetFlowfield;
	newTargetFlowfield->target = new GameObject();
	newTargetFlowfield->target->position = new glm::vec3(*gDepot->position);
	newTargetFlowfield->flowfield = flowFieldData.flowField;
	targets.push_back(newTargetFlowfield);
}

ColonistManager::~ColonistManager()
{
	DeleteCriticalSection(&targetsCriticalSection);
}

void ColonistManager::AddColonist(GameObject* goColonist)
{
	// Name arrays, names taken from https://www.fantasynamegenerators.com
	std::string firstNames[] = { "Lhoris", "Nym", "Vaeril", "Halueve", "Gaeleath", "Kailu", "Folre", "Arryn", "Ailre",
								"Arryn", "Micaiah", "Elauthin", "Tarathiel", "Tanathil", "Aenwyn", "Elnaril", "Navarre",
								"Amrynn", "Dain", "Inchel", "Lierin", "Imizael", "Saida", "Arbane", "Eroan", "Tanathil",
								"Nym", "Ashryn", "Erendriel", "Elpharae", "Kymil", "Imizael", "Adresin", "Kilyn", "Artin",
								"Elisen", "Anhaern", "Wirenth", "Aubron", "Ailre", "Larrel", "Myriil", "Aenwyn", "Elandorr",
								"Falael", "Meriel", "Nithenoel", "Aerendyl", "Eroan", "Calarel" };
	std::string lastNames[] = { "Fuseforge", "Dustfeather", "Spiderbranch", "Morningrider", "Bladekiller",
							"Nightsword", "Moonpelt", "Warstrike", "Blackblossom", "Havenvale",
							"Silverblossom", "Snakemane", "Covenpelt", "Dayrage", "Reddream",
							"Bloodheart", "Duskeyes", "Deathforge", "Regaldream", "Titanward",
							"Wiseflower", "Steelgrip", "Fallentide", "Cinderchaser", "Evenstride",
							"Seabeard", "Clantree", "Sternvalor", "Mistdraft", "Slatebone" };

	// Create a new Colonist instance
	Colonist* colonist = new Colonist();
	colonist->mGOColonist = goColonist;
	colonist->currentAction = "Idle...";
	colonist->icon = "warrior.bmp";
	colonist->mCharacterController = goColonist->characterController;

	// Pick a random name
	int firstNameIndex = rand() % (sizeof(firstNames) / sizeof(*firstNames));
	int lastNameIndex = rand() % (sizeof(lastNames) / sizeof(*lastNames));
	colonist->name = firstNames[firstNameIndex] + " " + lastNames[lastNameIndex];

	ColonistThreadData* colonistData = new ColonistThreadData();
	colonistData->pColonist = colonist;
	HANDLE hColonistThread = CreateThread(NULL, 0, UpdateColonistThread, (void*)colonistData, 0, 0);
	vecColonists.push_back(colonist);
}

void ColonistManager::AddEnemy(GameObject* goEnemy)
{
	Enemy* enemy = new Enemy();
	enemy->mGOEnemy = goEnemy;
	enemy->mCharacterController = goEnemy->characterController;
	enemy->mGOEnemy->animCharacter->m_IsLooping = true;
	enemy->mFlowfield = GetFlowField(gDepot->mesh->position);

	EnemyThreadData* enemyData = new EnemyThreadData();
	enemyData->pEnemy = enemy;
	HANDLE hEnemyThread = CreateThread(NULL, 0, UpdateEnemyThread, (void*)enemyData, 0, 0);
	vecEnemies.push_back(enemy);
}

void ColonistManager::AssignCommand(std::vector<int> ids, CommandType command, GameObject* goTarget)
{
	if (!goTarget)
		return;

	GetFlowFieldThreadData* threadData = new GetFlowFieldThreadData;
	threadData->manager = this;
	threadData->ids = ids;
	threadData->command = command;
	threadData->goTarget = goTarget;

	HANDLE hThread = CreateThread(NULL, 0, GetFlowFieldThread, (void*)threadData, 0, 0);
}

std::vector<std::vector<glm::vec2>> ColonistManager::GetFlowField(glm::vec3 target)
{
	EnterCriticalSection(&targetsCriticalSection);

	targetFlowfield* foundTargetFlowfield = nullptr;

	for (targetFlowfield* thisTarget : targets)
	{
		if (*(thisTarget->target->position) == target)
		{
			foundTargetFlowfield = thisTarget;
			break;
		}
	}

	if (foundTargetFlowfield)
	{
		LeaveCriticalSection(&targetsCriticalSection);
		return foundTargetFlowfield->flowfield;
	}

	glm::vec2 targetPos = TerrainManager::worldToGridCoords(target);

	// Create a flowfield thread data
	FlowFieldThreadData flowFieldData;
	flowFieldData.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	// Call the async flow field function
	gPathFinder->calculateFlowfieldAsync(targetPos, &flowFieldData);
	// Wait for it to finish
	WaitForSingleObject(flowFieldData.hEvent, INFINITE);
	// Give the colonist the action
	CloseHandle(flowFieldData.hEvent);

	targetFlowfield* newTargetFlowfield = new targetFlowfield;
	newTargetFlowfield->target = new GameObject();
	newTargetFlowfield->target->position = new glm::vec3(target);
	newTargetFlowfield->flowfield = flowFieldData.flowField;
	targets.push_back(newTargetFlowfield);

	LeaveCriticalSection(&targetsCriticalSection);

	return flowFieldData.flowField;
}

void ColonistManager::Update()
{
	for (size_t i = 0; i < vecColonists.size(); i++)
		if (vecColonists[i]->isDead) vecColonists.erase(vecColonists.begin() + i);
	for (size_t i = 0; i < vecEnemies.size(); i++)
		if (vecEnemies[i]->isDead) vecEnemies.erase(vecEnemies.begin() + i);
	for (Colonist* colonist : vecColonists)
	{
		Vector3 position;
		colonist->mCharacterController->GetPosition(position);
		colonist->mGOColonist->mesh->position.x = position.x;
		colonist->mGOColonist->mesh->position.y = position.y - 2.f;
		colonist->mGOColonist->mesh->position.z = position.z;
		if (colonist->mTarget == gDepot)
		{
			// Pass it the gDepot flowfield
			for (targetFlowfield* thisTarget : targets)
			{
				if (thisTarget == nullptr)
					continue;
				if (*(thisTarget->target->position) == *gDepot->position)
				{
					colonist->mFlowfield = thisTarget->flowfield;
				}
			}
		}
	}
	for (Enemy* enemy : vecEnemies)
	{
		// Enemy update logic
		Vector3 position;
		enemy->mCharacterController->GetPosition(position);
		enemy->mGOEnemy->mesh->position.x = position.x;
		enemy->mGOEnemy->mesh->position.y = position.y - 2.f;
		enemy->mGOEnemy->mesh->position.z = position.z;

		// Assign flowfield to enemy

		// If it has a colonist target get the flow field for it
		if (enemy->mColonistTarget != nullptr)
		{
			if (glm::distance(enemy->mColonistTarget->mGOColonist->mesh->position, enemy->mGOEnemy->mesh->position) <= 10.f)
			{
				continue;
			}
		}
		// Otherwise check if its target is the depot
		if (enemy->mGOTarget != nullptr)
		{
			if (enemy->mGOTarget == gDepot)
			{
				// Pass it the gDepot flowfield
				for (targetFlowfield* thisTarget : targets)
				{
					if (thisTarget->target->mesh != nullptr)
					{
						if (thisTarget->target->mesh->position == *gDepot->position)
						{
							enemy->mFlowfield = thisTarget->flowfield;
						}
					}
				}
			}
			else enemy->mFlowfield = GetFlowField(enemy->mGOTarget->mesh->position);
		}
		else
		{
			enemy->mGOTarget = gDepot;
			enemy->mFlowfield = GetFlowField(enemy->mGOTarget->mesh->position);
		}
	}
}

void ColonistManager::OnFlowFieldReady(std::vector<int> ids, CommandType command, GameObject* goTarget, std::vector<std::vector<glm::vec2>> flowField)
{
	// Assign the command and flowfield
	for (int id : ids)
	{
		vecColonists[id]->mFlowfield = flowField;
		vecColonists[id]->SetCommand(command, goTarget);
	}
}

DWORD WINAPI GetFlowFieldThread(LPVOID pThreadData)
{
	GetFlowFieldThreadData* data = (GetFlowFieldThreadData*)pThreadData;
	ColonistManager* manager = data->manager;
	std::vector<std::vector<glm::vec2>> flowField = manager->GetFlowField(*(data->goTarget->position));

	manager->OnFlowFieldReady(data->ids, data->command, data->goTarget, flowField);

	return 0;
}