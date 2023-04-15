#include "PathFinder.h"
#include <iostream>

PathFinder::PathFinder(int width, int height)
{
    mWidth = width;
    mHeight = height;
    mCellSize = 1.f;
    createGrid();
}

void PathFinder::createGrid()
{
    mGrid.resize(mHeight, std::vector<Node>(mWidth));
    for (int y = 0; y < mHeight; y++)
    {
        for (int x = 0; x < mWidth; x++)
        {
            mGrid[y][x].position = glm::vec2(x, y);
        }
    }
}

void PathFinder::addBuilding(int x, int y, BuildingType buildingType, float height, int goId)
{
    if (x >= mWidth || y >= mHeight)
    {
        std::cout << "Error: Invalid coords" << std::endl;
        return;
    }

    mGrid[y][x].buildingType = buildingType;
    mGrid[y][x].height = height;
    mGrid[y][x].goId = goId;
}

float PathFinder::getCellSize()
{
    return mCellSize;
}

int PathFinder::getWidth()
{
    return mWidth;
}

int PathFinder::getHeight()
{
    return mHeight;
}
