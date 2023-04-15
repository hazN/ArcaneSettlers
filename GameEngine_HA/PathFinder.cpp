#include "PathFinder.h"
#include <iostream>
#include <queue>
#include <map>

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

std::vector<std::vector<glm::vec2>> PathFinder::calculateFlowfield(glm::vec2 destination)
{
    std::vector<std::vector<glm::vec2>> flowField(mHeight, std::vector<glm::vec2>(mWidth, glm::vec2(0, 0)));

    // Using pairs to store current and previous position
    std::queue<std::pair<glm::vec2, glm::vec2>> queue;

    // Go through each open cell
    for (int y = 0; y < mHeight; y++)
    {
        for (int x = 0; x < mWidth; x++)
        {
            Node* node = &mGrid[y][x];
            if (node->buildingType != NONE)
                continue;
            // Reset the direction per flow field call
            flowField[y][x] = glm::vec2(0, 0);
            // Add it to the queue if its the destination
            if (node->position == destination)
                queue.push(std::make_pair(destination, destination));
        }
    }

    // Loop through every open cell
    while (!queue.empty())
    {
        glm::vec2 curPos, prevPos;
        std::tie(curPos, prevPos) = queue.front();
        queue.pop();

        // Calculate the new direction
        Node* curNode = &mGrid[curPos.y][curPos.x];
        flowField[curPos.y][curPos.x] = prevPos - curPos;

        for (int dirY = -1; dirY <= 1; dirY++)
        {
            for (int dirX = -1; dirX <= 1; dirX++)
            {
                // Skip itself
                if (dirX == 0 && dirY == 0) continue;

                // Get position of neighbour
                int neighbourX = curPos.x + dirX;
                int neighbourY = curPos.y + dirY;

                // Check if its in bounds 
                if (neighbourX >= 0 && neighbourX < mWidth && neighbourY >= 0 && neighbourY < mHeight)
                {
                    Node* neighbour = &mGrid[neighbourY][neighbourX];
                    // Skip if its a building or already has a direction
                    if (neighbour->buildingType != NONE || flowField[neighbourY][neighbourX] != glm::vec2(0, 0))
                        continue;

                    // Otherwise, add it to the queue and update the flow field for this neighbor
                    queue.push(std::make_pair(neighbour->position, curPos));
                    flowField[neighbourY][neighbourX] = curPos - neighbour->position;
                }
            }
        }
    }

    return flowField;
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

std::vector<std::vector<Node>>& PathFinder::getGrid()
{
    return mGrid;
}
