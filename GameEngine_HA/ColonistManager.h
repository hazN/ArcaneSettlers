#pragma once

#include <map>
#include "Colonist.h"
#include "PathFinder.h"
#include "TerrainManager.h"
#include <glm/glm.hpp>

struct targetFlowfield {
    GameObject* target;
    std::vector<std::vector<glm::vec2>> flowfield;
};

struct GetFlowFieldThreadData;

class ColonistManager {
public:
    ColonistManager();
    ~ColonistManager();

    void AddColonist(GameObject* goColonist);
    void AssignCommand(std::vector<int> colonists, CommandType command, GameObject* goTarget);
    std::vector<std::vector<glm::vec2>> GetFlowField(glm::vec3 target);
    void Update();
    void OnFlowFieldReady(std::vector<int> ids, CommandType command, GameObject* goTarget, std::vector<std::vector<glm::vec2>> flowField);

private:
    CRITICAL_SECTION targetsCriticalSection;
    std::vector<targetFlowfield*> targets;
};

struct GetFlowFieldThreadData {
    ColonistManager* manager;
    std::vector<int> ids;
    CommandType command;
    GameObject* goTarget;
};

DWORD WINAPI GetFlowFieldThread(LPVOID pThreadData);