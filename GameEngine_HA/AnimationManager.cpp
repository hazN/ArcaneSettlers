#include "AnimationManager.h"
#include <iostream>
#include <GLFW/glfw3.h>
AnimationManager::AnimationManager()
{
}

AnimationManager::~AnimationManager()
{
}
// Make sure go->mesh is not nullptr
Character* AnimationManager::CreateAnimatedCharacter(const char* filename, GameObject* go, glm::vec3 scale)
{
	go->animCharacter = new Character();
	if (go->mesh == nullptr)
	{
		std::cout << "Error: make sure mesh is not nullptr before calling CreateAnimatedCharacter on " << filename << std::endl;
		return nullptr;
	}
	go->animCharacter->Mesh = go->mesh;
	go->mesh->scaleXYZ = glm::vec3(scale);
	//assets/models/RPGCharacters/FBX/Warrior.fbx
	go->animCharacter->LoadCharacterFromAssimp(filename);
	go->animCharacter->LoadAnimationFromAssimp(filename);
	go->hasBones = true;
	_characters.emplace(filename, go);
	return go->animCharacter;
}

void AnimationManager::UpdateAll(float dt)
{
	for (std::pair<std::string, GameObject*> goCharacter : _characters)
	{
		// Get the current time in seconds
		double currTime = glfwGetTime();
		float elapsedTimeInSeconds = static_cast<float>(currTime - g_PrevTime);
		g_PrevTime = currTime;

		if (elapsedTimeInSeconds > 0.1f)
			elapsedTimeInSeconds = 0.1f;
		if (elapsedTimeInSeconds <= 0.f)
			elapsedTimeInSeconds = 0.001f;

		// Update the animation
		//animationManager->Update(goVector, elapsedTimeInSeconds);

		if (goCharacter.second->animCharacter != nullptr)
		{
			goCharacter.second->animCharacter->UpdateTransforms(goCharacter.second->BoneModelMatrices,
				goCharacter.second->GlobalTransformations,
				elapsedTimeInSeconds);
		}
	}
}
