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
	go->animCharacter->m_IsLooping = true;
	go->hasBones = true;
	charactersByName.emplace(go->mesh->friendlyName, go);
	characters.push_back(go);
	return go->animCharacter;
}

void AnimationManager::SetAnimation(const char* name, int animation)
{
	std::map<std::string, GameObject*>::iterator it = charactersByName.find(name);
	if (it != charactersByName.end())
	{
		GameObject* go = it->second;
		if (go->animCharacter)
		{
			go->animCharacter->SetAnimation(animation);
		}
	}
}

void AnimationManager::SetAnimation(const char* name, const char* animation)
{
	std::map<std::string, GameObject*>::iterator it = charactersByName.find(name);
	if (it != charactersByName.end())
	{
		GameObject* go = it->second;
		if (go->animCharacter)
		{
			// Assuming you have a method to convert the animation name to its corresponding ID
			int iAnimation = go->animCharacter->GetAnimationID(animation);
			go->animCharacter->SetAnimation(iAnimation);
		}
	}
}

void AnimationManager::UpdateAll(float elapsedTimeInSeconds)
{
	for (GameObject* goCharacter : characters)
	{
		if (goCharacter == nullptr)
			continue;
		// Update the animation
		if (goCharacter->animCharacter != nullptr)
		{
			goCharacter->animCharacter->UpdateTransforms(goCharacter->BoneModelMatrices,
				goCharacter->GlobalTransformations,
				elapsedTimeInSeconds);
		}
	}
}
