#pragma once

#include "EnemyDecisionTable.h"
#include <string>
#include <Windows.h>
#include <Interface/iCharacterController.h>
#include <vector>
#include <glm/glm.hpp>
#include "GameObject/GameObject.h"
#include <time.h>
#include "Colonist.h"

struct EnemyStats {
    // MAX 100
    int hp = 100;
    // Skills from lvl 1-20
    int combat = 1;
};

class Enemy {
public:
    Enemy();
    Enemy(EnemyStats stats);
    ~Enemy();

    void Update(float deltaTime);
    void Attack();
    void Move();
    void TakeDamage(int dmg);
    std::string icon;
    std::string currentAction;
    std::string name;
    float attackTime = clock();
    bool isDead = false;
    CRITICAL_SECTION mStatsCriticalSection;
    glm::vec3 lastFlowFieldTarget;
    GameObject* mGOEnemy;
    iCharacterController* mCharacterController;
    EnemyStats* mStats;
    Colonist* mColonistTarget;
    GameObject* mGOTarget;
    EnemyDecisionTable mDecisionTable;
    std::vector<std::vector<glm::vec2>> mFlowfield;
};

struct EnemyThreadData {
    EnemyThreadData() {
        pEnemy = NULL;
        bExitThread = NULL;
        bSuspendThread = NULL;
        suspendTime_ms = 16;
    }

    Enemy* pEnemy;
    bool bExitThread;
    bool bSuspendThread;
    unsigned int suspendTime_ms;
};

DWORD WINAPI UpdateEnemyThread(LPVOID pVOIDEnemy);