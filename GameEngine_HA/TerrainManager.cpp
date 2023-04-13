#include "TerrainManager.h"
#include <Interface/BoxShape.h>
#include <Interface/CylinderShape.h>
#include <iostream>
#include "IDGenerator.h"

TerrainManager::TerrainManager(GameObject* goTerrain, sModelDrawInfo* terrainInfo)
{
    this->goTerrain = goTerrain;
    this->terrainInfo = terrainInfo;
}

TerrainManager::~TerrainManager()
{
}

void TerrainManager::placeObjectsOnTerrain(const int maxObjects[3])
{
    std::vector<GameObject*> meshesToLoadIntoTerrain;
    const int NUM_TREES = maxObjects[0];
    const int NUM_ROCKS = maxObjects[1];
    const int NUM_GOLD = maxObjects[2];
    const glm::vec3 SCALE = glm::vec3(10.f);
    GameObject* goDepot = new GameObject;
    goDepot->id = IDGenerator::GenerateID();
    goDepot->buildingType = DEPOT;
    goDepot->inventory = new Inventory(999);
    Item food;
    food.icon = "assets/icons/food.png";
    food.id = itemId::food;
    food.name = "Food";
    food.weight = 1;
    goDepot->inventory->addItem(food, 10);
    goDepot->mesh = new cMeshObject();
    goDepot->mesh->meshName = "Depot";
    goDepot->mesh->friendlyName = "Depot";
    goDepot->mesh->bUse_RGBA_colour = true;
    goDepot->mesh->RGBA_colour = glm::normalize(glm::vec4(150.f, 75.f, 0.f, 1.f));
    goDepot->mesh->scaleXYZ = glm::vec3(SCALE * 2.f);
    goDepot->mesh->position = glm::vec3(0);
    goDepot->mesh->qRotation = glm::vec3(0);
    meshesToLoadIntoTerrain.push_back(goDepot);
    goMap.emplace(goDepot->id, goDepot);
    g_pMeshObjects.push_back(goDepot->mesh);

    for (size_t i = 0; i < NUM_TREES; i++)
    {
        GameObject* goTree = new GameObject;
        goTree->id = IDGenerator::GenerateID();
        Item wood;
        wood.icon = "assets/icons/wood.png";
        wood.id = itemId::wood;
        wood.name = "Wood";
        wood.weight = 2;
        goTree->inventory = new Inventory(20);
        goTree->inventory->addItem(wood, 10);
        goTree->buildingType = TREE;
        goTree->mesh = new cMeshObject();
        goTree->mesh->meshName = "PineTree";
        goTree->mesh->friendlyName = "PineTree";
        goTree->mesh->bUse_RGBA_colour = true;
        goTree->mesh->RGBA_colour = glm::vec4(0.2f, 0.8f, 0.2f, 1.f);
        goTree->mesh->scaleXYZ = glm::vec3(SCALE);
        goTree->mesh->position = glm::vec3(0);
        goTree->mesh->qRotation = glm::vec3(0);
        meshesToLoadIntoTerrain.push_back(goTree);
        goMap.emplace(goTree->id, goTree);
        g_pMeshObjects.push_back(goTree->mesh);
    }
    for (size_t i = 0; i < NUM_ROCKS; i++)
    {
        GameObject* goRock = new GameObject;
        goRock->id = IDGenerator::GenerateID();
        goRock->mesh = new cMeshObject();
        goRock->mesh->meshName = "Rock";
        goRock->mesh->friendlyName = "Rock";
        goRock->mesh->bUse_RGBA_colour = true;
        goRock->mesh->RGBA_colour = glm::vec4(0.5f, 0.5f, 0.5f, 1.f);
        goRock->mesh->scaleXYZ = glm::vec3(SCALE * 4.f);
        goRock->mesh->position = glm::vec3(0);
        goRock->mesh->qRotation = glm::vec3(0);
        meshesToLoadIntoTerrain.push_back(goRock);
        goMap.emplace(goRock->id, goRock);
        g_pMeshObjects.push_back(goRock->mesh);
    }
    for (size_t i = 0; i < NUM_GOLD; i++)
    {
        GameObject* goGold = new GameObject;
        goGold->id = IDGenerator::GenerateID();
        goGold->mesh = new cMeshObject();
        goGold->mesh->meshName = "Gold";
        goGold->mesh->friendlyName = "Gold";
        goGold->mesh->bUse_RGBA_colour = true;
        goGold->mesh->RGBA_colour = glm::vec4(1.f, 0.8f, 0.f, 1.f);
        goGold->mesh->scaleXYZ = glm::vec3(SCALE);
        goGold->mesh->position = glm::vec3(0);
        goGold->mesh->qRotation = glm::vec3(0);
        meshesToLoadIntoTerrain.push_back(goGold);
        goMap.emplace(goGold->id, goGold);
        g_pMeshObjects.push_back(goGold->mesh);
    }
    createPhysicsObjects(meshesToLoadIntoTerrain);
}

int TerrainManager::getRandom(int min, int max) {
    return min + rand() % (max - min + 1);
}

float TerrainManager::getRandom(float min, float max) {
    return min + (float)(rand()) / ((float)(RAND_MAX / (max - min)));
}

void TerrainManager::getTerrainHeightAndNormal(const glm::vec3& position, float& outHeight, glm::vec3& outNormal) {
    glm::vec3 rayOrigin(position.x, 10000.0f, position.z);
    glm::vec3 rayDirection(0.0f, -1.0f, 0.0f);

    iRayCast::RayCastHit hit;
    if (rayCast->doRayCast(rayOrigin, rayDirection, 20000.0f, hit)) {
        outHeight = hit.position.y;
        outNormal = hit.normal;
    }
    else {
        outHeight = 0.0f;
        outNormal = glm::vec3(0.0f, 1.0f, 0.0f);
    }
}

    void TerrainManager::createPhysicsObjects(std::vector<GameObject*> gameObjects) {
        for (GameObject* go : gameObjects) {

            // Get the draw info
            sModelDrawInfo drawInfo;
            pVAOManager->FindDrawInfoByModelName(go->mesh->meshName, drawInfo);

            // Randomize position and rotation
            glm::vec3 position;
            glm::vec3 normal;
            float terrainHeight;
            if (go->mesh->meshName == "Depot") {
                position = glm::vec3(0, 0, 0);
                normal = glm::vec3(0.0f, 1.0f, 0.0f);
                getTerrainHeightAndNormal(position, terrainHeight, normal);
                position.y = terrainHeight + 0.5f;
            }
            else {
                do {
                    position.x = goTerrain->mesh->position.x + getRandom(terrainInfo->minX * goTerrain->mesh->scaleXYZ.x, terrainInfo->maxX * goTerrain->mesh->scaleXYZ.x);
                    position.z = goTerrain->mesh->position.z + getRandom(terrainInfo->minZ * goTerrain->mesh->scaleXYZ.z, terrainInfo->maxZ * goTerrain->mesh->scaleXYZ.z);
                    getTerrainHeightAndNormal(position, terrainHeight, normal);
                    position.y = terrainHeight + 0.5f;
                } while (glm::distance(position, glm::vec3(0.0f, 0.0f, 0.0f)) < 10.0f);
            }
            glm::quat rotation = glm::quatLookAt(-normal, glm::vec3(0.0f, 1.0f, 0.0f));
            go->mesh->position = position;
            go->mesh->qRotation = rotation;
            RigidBodyDesc desc;
            desc.isStatic = true;
            desc.mass = 0.f;
            desc.position = position;
            desc.linearVelocity = glm::vec3(0.f);
            desc.rotation = rotation;

            iShape* shape;
            BuildingType buildingType;
            if (go->mesh->meshName == "PineTree") {
                shape = new BoxShape(Vector3(drawInfo.extentX * go->mesh->scaleXYZ.x / 2.f, drawInfo.extentY * go->mesh->scaleXYZ.y / 2.f , drawInfo.extentZ * go->mesh->scaleXYZ.z / 2.f));
                shape->SetUserData(go->id);
                buildingType = TREE;
            }
            else if (go->mesh->meshName == "Rock") {
                shape = new BoxShape(Vector3(drawInfo.extentX * go->mesh->scaleXYZ.x / 2.f, drawInfo.extentY * go->mesh->scaleXYZ.y / 2.f, drawInfo.extentZ * go->mesh->scaleXYZ.z / 2.f));
                shape->SetUserData(go->id);
                buildingType = ROCK;
            }
            else if (go->mesh->meshName == "Gold") {
                shape = new BoxShape(Vector3(drawInfo.extentX * go->mesh->scaleXYZ.x / 2.f, drawInfo.extentY * go->mesh->scaleXYZ.y / 2.f, drawInfo.extentZ * go->mesh->scaleXYZ.z / 2.f));
                shape->SetUserData(go->id);
                buildingType = GOLD;
            }
            else if (go->mesh->meshName == "Depot") {
                shape = new BoxShape(Vector3(drawInfo.extentX * go->mesh->scaleXYZ.x / 2.f, drawInfo.extentY * go->mesh->scaleXYZ.y/ 2.f, drawInfo.extentZ * go->mesh->scaleXYZ.z / 2.f));
                shape->SetUserData(go->id);
                buildingType = DEPOT;
            }
            else
            {
                std::cout << "Unknown building passed into createPhysicsObjects" << std::endl;
                return;
            }

            // Update the GameObject with the new physics object
            go->rigidBody = _physicsFactory->CreateRigidBody(desc, shape);
            world->AddBody(go->rigidBody);
        }
    }
