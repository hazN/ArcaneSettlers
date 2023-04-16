#pragma once
#include "BuildingType.h"
#include <glm/glm.hpp>
#include <vector>
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <assert.h>

struct FlowFieldThreadData;

struct Node
{
    int id;
    BuildingType buildingType;
    glm::vec2 position;
    glm::vec2 direction;
    float height; 
    int goId;
    float cost;
    Node()
    {
        id = -1;
        buildingType = NONE;
        position = glm::vec2(0, 0);
        float height = 0.f;
        goId = -1;
        cost = -1;
    }
    Node(int id, BuildingType buildingType, glm::vec2 position, float height, int goId)
    {
        this->id = id;
        this->buildingType = buildingType;
        this->position = position;
        this->height = height;
        this->goId = goId;
        cost = -1;
    }
};

class PathFinder
{
public:
    PathFinder(int width, int height);
    ~PathFinder();
    void createGrid();
    void addBuilding(int x, int y, BuildingType buildingType, float height, int goId);
    void removeBuilding(int goId);
    void calculateFlowfieldAsync(glm::vec2 destination, FlowFieldThreadData* flowFieldData);
    std::vector<std::vector<glm::vec2>> calculateFlowfield(glm::vec2 target);
    float getCellSize();
    int getWidth();
    int getHeight();
    std::vector<std::vector<Node>>& getGrid();
private:
    int mWidth;
    int mHeight;
    float mCellSize;
    CRITICAL_SECTION mGridCriticalSection;
    std::vector<std::vector<Node>> mGrid;
};

struct FlowFieldThreadData
{
    PathFinder* pathFinder;
    glm::vec2 destination;
    std::vector<std::vector<glm::vec2>> flowField;
    HANDLE hEvent;
};

DWORD WINAPI CalculateFlowFieldThread(LPVOID pFlowFieldThreadData);