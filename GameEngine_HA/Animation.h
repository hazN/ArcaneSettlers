#pragma once

#include <map>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <string>
#include "BoneHierarchy.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

struct BoneInfo
{
	std::string mName;
	glm::mat4 mBoneOffset;
	glm::mat4 mGlobalTransformation;
	glm::mat4 mFinalTransformation; 
};

struct BoneVertexData
{
	BoneVertexData()
	{
		mIds[0] = 0;
		mIds[1] = 0;
		mIds[2] = 0;
		mIds[3] = 0;
		mWeights[0] = 0.f;
		mWeights[1] = 0.f;
		mWeights[2] = 0.f;
		mWeights[3] = 0.f;
	}

	unsigned int mIds[4];
	float mWeights[4];

	void AddBoneInfo(int id, float weight)
	{
		int numIds = sizeof(mIds) / sizeof(mIds[0]);
		for (int i = 0; i < numIds; i++)
		{
			if (mWeights[i] == 0.f)
			{
				mIds[i] = id;
				mWeights[i] = weight;
				return;
			}
		}

		assert(0);
	}
};

enum EasingType
{
	EaseIn,
	EaseOut,
	EaseInOut,
	None
};

struct PositionKeyFrame
{
	PositionKeyFrame()
		: value(0.f), time(0.f), type(None) { }
	PositionKeyFrame(glm::vec3 value, float time, EasingType type = None)
		: value(value), time(time), type(type) { }

	glm::vec3 value;
	float time;
	EasingType type;
};

struct ScaleKeyFrame
{
	ScaleKeyFrame()
		: value(0.f), time(0.f), type(None) { }
	ScaleKeyFrame(glm::vec3 value, float time, EasingType type = None)
		: value(value), time(time), type(type) { }

	glm::vec3 value;
	float time;
	EasingType type;
};

struct RotationKeyFrame
{
	RotationKeyFrame()
		: value(1.0f, 0.f, 0.f, 0.f), time(0.f), useSlerp(true) { }
	RotationKeyFrame(glm::quat value, float time, bool useSlerp = true)
		: value(value), time(time), useSlerp(useSlerp) { }
	glm::quat value;
	float time;
	bool useSlerp;
};

struct AnimationData
{
	// This is one node or one channel
	std::vector<PositionKeyFrame> PositionKeyFrames;
	std::vector<ScaleKeyFrame> ScaleKeyFrames;
	std::vector<RotationKeyFrame> RotationKeyFrames;
	std::string Name;
	double Duration;
	double TicksPerSecond;
};

struct CharacterAnimationData
{
	CharacterAnimationData(const aiScene* scene) : AIScene(scene) { }
	const aiScene* AIScene;

	BoneHierarchy* BoneHierarchy;			

	std::vector<BoneInfo> boneInfoVec;
	std::map<std::string, int> boneNameToIdMap;

	std::vector<AnimationData*> Channels;
	std::string Name;
	double Duration;
	double TicksPerSecond;
};

struct Animation
{
	std::vector<glm::mat4> GlobalTransformations;
	bool IsCharacterAnimation;
	std::string AnimationType;
	float AnimationTime;
	bool IsPlaying;
	bool IsLooping;
	float Speed;
};