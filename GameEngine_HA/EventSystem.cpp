#include "EventSystem.h"
#include <cstdlib>
#include "GameGUI.h"
#include "ColonistManager.h"
#include "IDGenerator.h"
#include <Interface/CylinderShape.h>
extern ColonistManager* colonistManager;
EventSystem::EventSystem()
{
    mWealth = 0;
	mLastEventTime = (float)(clock() / CLOCKS_PER_SEC);
    mEventInterval = 5.f;
}

void EventSystem::AddWealth(int wealth)
{
    mWealth += wealth;
}

void EventSystem::Update()
{
    float currentTime = (float)(clock() / CLOCKS_PER_SEC);
    float timeSinceLastEvent = currentTime - mLastEventTime;

    if (timeSinceLastEvent >= mEventInterval)
    {
        mLastEventTime = currentTime;

        if (rand() / (float)(RAND_MAX) < EventProbability())
        {
            TriggerEvent();
        }
    }
}

float EventSystem::EventProbability()
{
    return mWealth / 50.f;
}

void EventSystem::SpawnEnemy()
{
    GameGUI::addMessage("Event triggered: Enemies are raiding you!");
	int NUMENEMIES = 1;
	sModelDrawInfo terrainInfo;
	GameObject* goTerrain = goMap[0];
	pVAOManager->FindDrawInfoByModelName(goTerrain->mesh->meshName, terrainInfo);
	for (int i = 0; i < NUMENEMIES; i++) {
		// Create colonist mesh
		cMeshObject* pEnemy = new cMeshObject();
		pEnemy->meshName = "Cleric";
		pEnemy->friendlyName = "Enemy" + std::to_string(i);
		// Randomize position near the edge of the terrain
		float edgeOffset = 5.0f; 
		glm::vec3 position;
		float terrainHeight;
		glm::vec3 normal;
		do {
			position.x = goTerrain->mesh->position.x + TerrainManager::getRandom(edgeOffset, terrainInfo.maxX * goTerrain->mesh->scaleXYZ.x - edgeOffset);
			position.z = goTerrain->mesh->position.z + TerrainManager::getRandom(edgeOffset, terrainInfo.maxZ * goTerrain->mesh->scaleXYZ.z - edgeOffset);
			TerrainManager::getTerrainHeightAndNormal(position, terrainHeight, normal);
			position.y = terrainHeight + 0.5f;
		} while (glm::distance(position, glm::vec3(0.0f, 0.0f, 0.0f)) < 10.0f);
		pEnemy->position = position;
		pEnemy->bUse_RGBA_colour = false;
		pEnemy->scaleXYZ = glm::vec3(1);
		pEnemy->setRotationFromEuler(glm::vec3(0.f, glm::radians(90.f), glm::radians(90.f)));
		pEnemy->textures[0] = "RedCleric.bmp";
		pEnemy->textureRatios[0] = 1.f;
		pEnemy->textureRatios[1] = 1.f;
		pEnemy->textureRatios[2] = 1.f;
		pEnemy->textureRatios[3] = 1.f;
		// Create GameObject
		GameObject* goEnemy = new GameObject();
		goEnemy->id = IDGenerator::GenerateID();
		goEnemy->mesh = pEnemy;
		goEnemy->mesh->scaleXYZ = glm::vec3(0.01f);
		goEnemy->animCharacter = animationManager->CreateAnimatedCharacter("assets/models/Characters/riggedCleric.fbx", goEnemy, glm::vec3(0.01f));
		goEnemy->animCharacter->SetAnimation(10);
		// Create Character controller
		iShape* cylinderShape = new CylinderShape(Vector3(0.7f, 2.f, 0.7f));
		cylinderShape->SetUserData(goEnemy->id);
		glm::quat rotation = glm::quat(glm::vec3(0));
		iCharacterController* playerCharacterController = _physicsFactory->CreateCharacterController(cylinderShape, position, rotation);
		world->AddCharacterController(playerCharacterController);
		playerCharacterController->SetGravity(Vector3(0.f, -9.81f, 0.f));
		goEnemy->characterController = playerCharacterController;
		goVector.push_back(goEnemy);
		goMap.emplace(goEnemy->id, goEnemy);
		colonistManager->AddEnemy(goEnemy);
	}
}

void EventSystem::TriggerEvent()
{
	mLastEventTime = (float)(clock() / CLOCKS_PER_SEC);
	std::thread spawnEnemyThread(&EventSystem::SpawnEnemy, this);
	spawnEnemyThread.detach();
}
