#pragma once
#include "BuildingType.h"
#include <glm/glm.hpp>
#include <vector>

struct Node
{
    int id;
    BuildingType buildingType;
    glm::vec2 position;
    glm::vec2 direction;
    float height; 
    int goId;
    Node()
    {
        id = -1;
        buildingType = NONE;
        position = glm::vec2(0, 0);
        float height = 0.f;
        goId = -1;
    }
    Node(int id, BuildingType buildingType, glm::vec2 position, float height, int goId)
    {
        this->id = id;
        this->buildingType = buildingType;
        this->position = position;
        this->height = height;
        this->goId = goId;
    }
};

class PathFinder
{
public:
    PathFinder(int width, int height);
    void createGrid();
    void addBuilding(int x, int y, BuildingType buildingType, float height, int goId);
    std::vector<std::vector<glm::vec2>> calculateFlowfield(glm::vec2 target);
    float getCellSize();
    int getWidth();
    int getHeight();
    std::vector<std::vector<Node>>& getGrid();
private:
    int mWidth;
    int mHeight;
    float mCellSize;
    std::vector<std::vector<Node>> mGrid;
};
