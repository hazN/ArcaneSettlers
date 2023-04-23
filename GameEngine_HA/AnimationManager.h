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
    void UpdateAll(float dt);

private:
    std::map<std::string, GameObject*> charactersByName;
    std::vector<GameObject*> characters;
    float gPrevTime = 0.f;
};
