#include "Enemy.h"
#include "TerrainManager.h"
#include "quaternion_utils.h"

Enemy::Enemy() 
{
    mStats = new EnemyStats();
    lastFlowFieldTarget = glm::vec3(-5000.f);
    InitializeCriticalSection(&mStatsCriticalSection);
}

Enemy::Enemy(EnemyStats stats)
{
    mStats = new EnemyStats;
    mStats->hp = stats.hp;
    mStats->combat = stats.combat;
    InitializeCriticalSection(&mStatsCriticalSection);
}

Enemy::~Enemy()
{
    DeleteCriticalSection(&mStatsCriticalSection);
}

void Enemy::Update(float deltaTime) {
    EnemyActionType action = mDecisionTable.getNextAction(*this);

    switch (action) {
    case EnemyActionType::AttackColonist:
        currentAction = "Attacking...";
        if (mColonistTarget != nullptr) 
        {
            if (glm::distance(mColonistTarget->mGOColonist->mesh->position, this->mGOEnemy->mesh->position) <= 30.f)
            {
                Attack();
            }
        }
        break;
    case EnemyActionType::AttackBuilding:
        currentAction = "Attacking...";
        if (mGOTarget != nullptr)
        {
            if (glm::distance(mGOTarget->mesh->position, this->mGOEnemy->mesh->position) <= 10.f)
            {
                Attack();
            }
        }
        break;
    case EnemyActionType::Move:
        currentAction = "Moving...";
            Move();
        break;
    default:
        currentAction = "Idle...";
        break;
    }
}

void Enemy::Attack()
{
    float duration = (clock() - attackTime) / (double)CLOCKS_PER_SEC;

    if (duration >= 0.25f)
    {
        attackTime = clock();
        if (mColonistTarget != nullptr)
        {
        }
        else if (mGOTarget != nullptr)
        {
        }
    }
}

void Enemy::Move()
{
    if (mFlowfield.empty())
    {
        return;
    }

    // Get current position and convert to grid coordinates
    glm::vec3 currentPos = mGOEnemy->mesh->position;
    glm::vec2 currentGridPos = TerrainManager::worldToGridCoords(currentPos);
    currentGridPos = glm::round(currentGridPos);

    // Get the direction from the flowfield
    glm::vec2 flowDirection = mFlowfield[currentGridPos.y][currentGridPos.x];

    // Move the enemy
    float speed = 1.2f;
    float deltaTime = 0.1f;
    glm::vec3 moveDirection = glm::vec3(flowDirection.x, 0.5f, flowDirection.y);
    glm::vec3 dir = moveDirection * speed * deltaTime;
    mCharacterController->Move(dir);

    glm::vec3 lookDir = glm::normalize(glm::vec3(moveDirection.x, -mGOEnemy->mesh->position.y, moveDirection.z));
    glm::quat targetDir = q_utils::LookAt(lookDir, glm::vec3(0, 1, 0));
    if (std::isnan(mGOEnemy->mesh->qRotation.x))
        mGOEnemy->mesh->qRotation = glm::quat();
    mGOEnemy->mesh->qRotation = q_utils::RotateTowards(mGOEnemy->mesh->qRotation, targetDir, 3.14f * 0.05f);
}

void Enemy::TakeDamage(int dmg)
{
    EnterCriticalSection(&mStatsCriticalSection);
    mStats->hp -= dmg;
    if (mStats->hp <= 0)
    {
        isDead = true;
    }
    LeaveCriticalSection(&mStatsCriticalSection);
}


DWORD WINAPI UpdateEnemyThread(LPVOID pVOIDEnemy) {
    EnemyThreadData* pThreadData = static_cast<EnemyThreadData*>(pVOIDEnemy);

    while (!pThreadData->bExitThread) {
        if (!pThreadData->bSuspendThread) {
            pThreadData->pEnemy->Update(static_cast<float>(pThreadData->suspendTime_ms) / 1000.0f);
        }
        Sleep(pThreadData->suspendTime_ms);
    }

    return 0;
}
