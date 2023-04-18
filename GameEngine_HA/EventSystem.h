#pragma once
#include <ctime>
#include <thread>

class EventSystem
{
public:
    EventSystem();
    void AddWealth(int wealth);
    void Update();

private:
    void SpawnEnemy();
    float EventProbability();
    void TriggerEvent();

private:
    int mWealth;
    float mPauseTime;
    float mLastEventTime;
    float mEventInterval;
};
