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
	Colonist* colonist = new Colonist();
	colonist->mGOColonist = goColonist;
	colonist->currentAction = "Idle...";
	colonist->icon = "warrior.bmp";
	colonist->name = "Alex";
	colonist->mCharacterController = goColonist->characterController;
	ColonistThreadData* colonistData = new ColonistThreadData();
	colonistData->pColonist = colonist;
	HANDLE hColonistThread = CreateThread(NULL, 0, UpdateColonistThread, (void*)colonistData, 0, 0);
	vecColonists.push_back(colonist);
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
                if (*(thisTarget->target->position) == *gDepot->position) 
                {
                    colonist->mFlowfield = thisTarget->flowfield;
                }
            }
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