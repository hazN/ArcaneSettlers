#pragma once

#include "globalThings.h"
#include <string>
#include <vector>
#include "BuildingType.h"
#include <Interface/iRayCast.h>
#include "PathFinder.h"

class TerrainManager {
public:
	TerrainManager(GameObject* goTerrain, sModelDrawInfo* terrainInfo);
	~TerrainManager();
	void placeObjectsOnTerrain(const int maxObjects[3]);

	std::vector<std::vector<int>> mGrid;

	static void getTerrainHeightAndNormal(const glm::vec3& position, float& outHeight, glm::vec3& outNormal);
	static glm::vec2 worldToGridCoords(glm::vec3 worldCoords);
	void createBuilding(BuildingType type, iRayCast::RayCastHit hit);
	PathFinder* mPathFinder;
private:
	static GameObject* goTerrain;
	static sModelDrawInfo* terrainInfo;
	int getRandom(int min, int max);
	float getRandom(float min, float max);
	void createPhysicsObjects(std::vector<GameObject*> gameObjects);
};
