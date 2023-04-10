#pragma once
#include "cVAOManager/sModelDrawInfo.h"
#include <vector>
class PhysicsHelper
{
public:
	PhysicsHelper() = default;
	~PhysicsHelper() = default;

	std::vector<float> generateHeightData(sModelDrawInfo drawInfo, unsigned int gridWidth, unsigned int gridDepth);
	void getTerrainGridSize(sModelDrawInfo drawInfo, float resolution, unsigned int& gridWidth, unsigned int& gridDepth);

};