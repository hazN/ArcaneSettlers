#include "PhysicsHelper.h"
#include <iostream>

std::vector<float> PhysicsHelper::generateHeightData(sModelDrawInfo drawInfo, unsigned int gridWidth, unsigned int gridDepth)
{
    std::vector<float> heightData(gridWidth * gridDepth, 0.0f);

    for (unsigned int z = 0; z < gridDepth; ++z)
    {
        for (unsigned int x = 0; x < gridWidth; ++x)
        {
            unsigned int index = z * gridWidth + x;
            if (index < drawInfo.numberOfVertices)
            {
                heightData[index] = drawInfo.pVertices[index].y; 
            }
        }
    }

    return heightData;
}

void PhysicsHelper::getTerrainGridSize(sModelDrawInfo drawInfo, float resolution, unsigned int& gridWidth, unsigned int& gridDepth)
{
    float width = drawInfo.maxX - drawInfo.minX;
    float depth = drawInfo.maxZ - drawInfo.minZ;

    gridWidth = (unsigned int)(width / resolution) + 1.f;
    gridDepth = (unsigned int)(depth / resolution) + 1.f;
}