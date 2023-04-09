#pragma once
#include <vector>
#include <map>
#include <string>
#include <glm/glm.hpp>
#include "Character.h"
#include "GameObject/GameObject.h"

class AnimationManager
{
public:
    AnimationManager();
    ~AnimationManager();

    Character* CreateAnimatedCharacter(const char* filename, GameObject* go, glm::vec3 scale);
    void SetAnimation(const char* name, int animation);
    void SetAnimation(const char* name, const char* animation);
    //Character* CreateAnimatedCharacter(const char* filename, const char* animFilename, GameObject* go, glm::vec3 scale);

    void UpdateAll(float dt);

private:
    std::map<std::string, GameObject*> _mapCharacters;
    std::vector<GameObject*> _vecCharacters;
    float g_PrevTime = 0.f;
};
