#include "PathFinder.h"
#include <iostream>
#include <queue>
#include <map>

PathFinder::PathFinder(int width, int height)
{
	mWidth = width;
	mHeight = height;
	mCellSize = 1.f;
    InitializeCriticalSection(&mGridCriticalSection);
	createGrid();
}

PathFinder::~PathFinder()
{
    DeleteCriticalSection(&mGridCriticalSection);
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
    EnterCriticalSection(&mGridCriticalSection);
	mGrid[y][x].buildingType = buildingType;
	mGrid[y][x].height = height;
	mGrid[y][x].goId = goId;
    LeaveCriticalSection(&mGridCriticalSection);
}
void PathFinder::removeBuilding(int goId)
{
    EnterCriticalSection(&mGridCriticalSection);
    for (int y = 0; y < mHeight; y++)
    {
        for (int x = 0; x < mWidth; x++)
        {
            if (mGrid[y][x].goId == goId)
            {
                mGrid[y][x].buildingType = BuildingType::NONE;
                mGrid[y][x].goId = -1;
            }
        }
    }
    LeaveCriticalSection(&mGridCriticalSection);
}

DWORD WINAPI CalculateFlowFieldThread(LPVOID pFlowFieldThreadData)
{
    FlowFieldThreadData* flowFieldData = (FlowFieldThreadData*)pFlowFieldThreadData;
    flowFieldData->flowField = flowFieldData->pathFinder->calculateFlowfield(flowFieldData->destination);
    SetEvent(flowFieldData->hEvent);
    return 0;
}

void PathFinder::calculateFlowfieldAsync(glm::vec2 destination, FlowFieldThreadData* flowFieldData)
{
    flowFieldData->pathFinder = this;
    flowFieldData->destination = destination;
    HANDLE hThread = CreateThread(NULL, 0, CalculateFlowFieldThread, (void*)flowFieldData, 0, 0);
}

std::vector<std::vector<glm::vec2>> PathFinder::calculateFlowfield(glm::vec2 destination)
{
    std::vector<std::vector<glm::vec2>> flowField(mHeight, std::vector<glm::vec2>(mWidth, glm::vec2(0, 0)));
    std::vector<std::vector<bool>> visited(mHeight, std::vector<bool>(mWidth, false));

    // Using pairs to store current and previous position
    std::queue<std::pair<glm::vec2, glm::vec2>> queue;

    // Go through each open cell
    for (int y = 0; y < mHeight; y++)
    {
        for (int x = 0; x < mWidth; x++)
        {
            Node* node = &mGrid[y][x];
            // Assign cost of 1 to open cells, cost of 1000 to buildings, this is so
            // that it will avoid buildings but still use them if its the only option 
            if (node->buildingType == NONE)
                node->cost = 1.0f;
            else node->cost = 1000.0f;
            // Assign destination
            if (node->position == destination) {
                queue.push(std::make_pair(destination, destination));
                visited[y][x] = true;
            }
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
                    // Skip if it has been visited
                    if (visited[neighbourY][neighbourX])
                        continue;

                    // Calculate the cost of the path from the current cell to the neighbor
                    float Cost = curNode->cost + neighbour->cost;

                    // Compare cost 
                    if (Cost < prevPos.y * mWidth + prevPos.x)
                    {
                        // Update the direction to the neighbor
                        flowField[neighbourY][neighbourX] = curPos - neighbour->position;
                        visited[neighbourY][neighbourX] = true;
                        queue.push(std::make_pair(neighbour->position, curPos));
                    }
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