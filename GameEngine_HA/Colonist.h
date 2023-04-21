#pragma once

#include "DecisionTable.h"
#include "Inventory.h"
#include <vector>
#include <Windows.h>
#include "GameObject/GameObject.h"
#include <Interface/iCharacterController.h>
#include "Inventory.h"
#include <time.h>
#include "PathFinder.h"
#include "ParticleSystem.h"
struct ColonistStats 
{
    // MAX 100
    float maxHp = 100;
    float hp = 100;
    float hunger = 100.f;
    // Skills from lvl 1-20
    int mining = 1;
    int chopping = 1;
    int combat = 1;
};

class Colonist 
{
public:
    Colonist();
    Colonist(ColonistStats stats);
    ~Colonist();

    void Update(float deltaTime);

    void SetCommand(CommandType command, GameObject* target);
    void ExecuteCommand();
    bool isHungry();
    bool getIsIntruderInRange();
    void TakeDamage(float dmg);
    ColonistStats getStats();
    std::string icon;
    std::string currentAction;
    std::string name;
    bool isDead = false;
    bool exitThread = false;
//private:
    void Move();
    void HarvestTree();
    void MineNode();
    void DropOffLoot();
    void Attack();
    void Eat();
    float duration;
    float deltaTime = clock();
    float attackTime = clock();
    GameObject* mGOColonist;
    iCharacterController* mCharacterController;
    CRITICAL_SECTION mStatsCriticalSection;
    ColonistStats* mStats;
    Inventory* mInventory;
    CommandType mCurrentCommand;
    GameObject* mTarget;
    Enemy* mEnemyTarget;
    DecisionTable mDecisionTable;
    std::vector<std::vector<glm::vec2>> mFlowfield;
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
