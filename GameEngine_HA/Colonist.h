#pragma once

#include "DecisionTable.h"
#include "Inventory.h"
#include <vector>
#include <Windows.h>
#include "GameObject/GameObject.h"
#include <Interface/iCharacterController.h>
#include "Inventory.h"
#include <time.h>
struct ColonistStats {
    // MAX 100
    int hp = 100;
    float hunger = 100.f;
    // Skills from lvl 1-20
    int mining = 1;
    int chopping = 1;
    int combat = 1;
};

class Colonist {
public:
    Colonist();
    Colonist(ColonistStats stats);
    ~Colonist();

    void Update(float deltaTime);

    void SetCommand(CommandType command, GameObject* target);
    void ExecuteCommand();
    void UpdateDecisionTable();
    bool isHungry();
    bool getIsIntruderInRange();
    ColonistStats getStats();
    std::string icon;
    std::string currentAction;
    std::string name;
//private:
    void HarvestTree();
    void MineNode();
    void DropOffLoot();
    void Eat();
    float duration;
    float deltaTime = clock();
    GameObject* mGOColonist;
    iCharacterController* mCharacterController;
    ColonistStats* mStats;
    Inventory* mInventory;
    CommandType mCurrentCommand;
    GameObject* mTarget;
    DecisionTable mDecisionTable;
};

struct ColonistThreadData {
    ColonistThreadData() 
    {
        pColonist = NULL;
        bExitThread = NULL;
        bSuspendThread = NULL;
        suspendTime_ms = 16;
    }

    Colonist* pColonist;
    bool bExitThread;
    bool bSuspendThread;
    unsigned int suspendTime_ms;
};

DWORD WINAPI UpdateColonistThread(LPVOID pVOIDColonist);
