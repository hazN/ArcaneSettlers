#include "Character.h"

#include <iostream>
#include <glm/gtx/easing.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include "globalThings.h"

void Character::CastToGLM(const aiMatrix4x4& in, glm::mat4& out)
{
	out = glm::transpose(glm::make_mat4(&in.a1));
}

void Character::CastToGLM(const aiQuaternion& in, glm::quat& out)
{
	out.w = in.w;
	out.x = in.x;
	out.y = in.y;
	out.z = in.z;
}

void Character::CastToGLM(const aiVector3D& in, glm::vec3& out)
{
	out.x = in.x;
	out.y = in.y;
	out.z = in.z;
}

AssimpScene::AssimpScene(const char* filename, unsigned int flags)
{
	mScene = mImporter.ReadFile(filename, flags);

	Animations = mScene->mAnimations;
	Cameras = mScene->mCameras;
	Lights = mScene->mLights;
	Materials = mScene->mMaterials;
	Meshes = mScene->mMeshes;
	Textures = mScene->mTextures;

	RootNode = mScene->mRootNode;
}

AssimpScene::~AssimpScene() { }

Character::Character()
	: mIsPlaying(true)
	, mAnimationSpeed(1.0)
	, mCurrentTimeInSeconds(0.0)
	, mCurrentAnimation(0)
	, mNumAnimationsLoaded(0)
{
}

Character::~Character()
{
}

void Character::LoadCharacterFromAssimp(const char* filename)
{
	unsigned int flags =
		aiProcess_Triangulate
		| aiProcess_LimitBoneWeights
		| aiProcess_JoinIdenticalVertices;
	mScene = new AssimpScene(filename, flags);

	aiMatrix4x4 g = mScene->RootNode->mTransformation;
	aiMatrix4x4 inverse = g.Inverse();

	// Node hierarchy for rendering
	mRootNode = CreateNodeHierarchy(mScene->RootNode);
	CastToGLM(inverse, mGlobalInverseTransform);
	// Check if it has meshes
	if (mScene->HasMeshes())
	{
		aiMesh* mainMesh = nullptr;
		std::vector<aiMesh*> additionalMeshes;

		int maxBones = -1;
		// Loop through meshes (fbx files can have multiple)
		for (size_t i = 0; i < mScene->NumMeshes(); i++)
		{
			// Check for bones before adding them
			if (mScene->Meshes[i]->HasBones())
			{
				int boneCount = mScene->Meshes[i]->mNumBones;
				// The mesh with the most bones is probably the main mesh
				if (boneCount > maxBones)
				{
					maxBones = boneCount;
					mainMesh = mScene->Meshes[i];
				}
				else
				{
					additionalMeshes.push_back(mScene->Meshes[i]);
				}
			}
		}
		// Load the bones now
		if (mainMesh)
		{
			LoadAssimpBones(mainMesh, additionalMeshes);
		}
	}
}

void Character::LoadAnimationFromAssimp(const char* filename)
{
	// Read in and get the number of animations in the file
	const aiScene* scene = mAnimationImporter.ReadFile(filename, 0);
	unsigned int numAnimations = scene->mNumAnimations;

	// Loop through animations
	for (unsigned int i = 0; i < numAnimations; ++i)
	{
		aiAnimation* animation = scene->mAnimations[i];
		// Load the animation data for each one now
		LoadAssimpAnimation(animation);
	}
}

void Character::LoadAssimpBones(const aiMesh* assimpMesh)
{
	// Record Vertex Weights
	int totalWeights = 0;
	mBoneVertexData.resize(assimpMesh->mNumVertices);
	int boneCount = 0;

	int numBones = assimpMesh->mNumBones;
	// Loop through each bone
	for (int i = 0; i < numBones; i++)
	{
		aiBone* bone = assimpMesh->mBones[i];

		int boneIdx = 0;
		std::string boneName(bone->mName.data);

		// Check if we have already added this node
		std::map<std::string, int>::iterator it = mBoneNameToIdMap.find(boneName);
		if (it == mBoneNameToIdMap.end())
		{
			// We haven't so add it in
			boneIdx = boneCount;
			boneCount++;
			BoneInfo bi;
			bi.mName = boneName;
			mBoneInfoVec.push_back(bi);

			CastToGLM(bone->mOffsetMatrix, mBoneInfoVec[boneIdx].mBoneOffset);
			mBoneNameToIdMap[boneName] = boneIdx;
			mBoneVec.push_back(bone);
		}
		else
		{
			// Otherwise just return its id
			boneIdx = it->second;
		}

		// Loop through the bone weights and add them to the vertex data
		for (int weightIdx = 0; weightIdx < bone->mNumWeights; weightIdx++)
		{
			float weight = bone->mWeights[weightIdx].mWeight;
			int vertexId = bone->mWeights[weightIdx].mVertexId;
			mBoneVertexData[vertexId].AddBoneInfo(boneIdx, weight);
		}
	}
}

void Character::LoadAssimpBones(const aiMesh* assimpMesh, const std::vector<aiMesh*> additionalMeshes)
{
	// Record Vertex Weights
	int totalWeights = 0;
	mBoneVertexData.resize(assimpMesh->mNumVertices);
	int boneCount = 0;

	int numBones = assimpMesh->mNumBones;
	// Loop through each bone
	for (int i = 0; i < numBones; i++)
	{
		aiBone* bone = assimpMesh->mBones[i];

		int boneIdx = 0;
		std::string boneName(bone->mName.data);

		// Check if we have already added this node
		std::map<std::string, int>::iterator it = mBoneNameToIdMap.find(boneName);
		if (it == mBoneNameToIdMap.end())
		{
			// We haven't so add it in
			boneIdx = boneCount;
			boneCount++;
			BoneInfo bi;
			bi.mName = boneName;
			mBoneInfoVec.push_back(bi);

			CastToGLM(bone->mOffsetMatrix, mBoneInfoVec[boneIdx].mBoneOffset);
			mBoneNameToIdMap[boneName] = boneIdx;
			mBoneVec.push_back(bone);
		}
		else
		{
			// Otherwise just return its id
			boneIdx = it->second;
		}

		// Loop through the bone weights and add them to the vertex data
		for (int weightIdx = 0; weightIdx < bone->mNumWeights; weightIdx++)
		{
			float weight = bone->mWeights[weightIdx].mWeight;
			int vertexId = bone->mWeights[weightIdx].mVertexId;
			mBoneVertexData[vertexId].AddBoneInfo(boneIdx, weight);
		}
	}
}

void Character::UpdateTransforms(std::vector<glm::mat4>& transforms, std::vector<glm::mat4>& globals, float dt)
{
	if (mIsPlaying && mAnimationSpeed != 0.0f)
	{
		mCurrentTimeInSeconds += dt * mAnimationSpeed;

		// If its looping reset the time at the end of the animation
		if (m_IsLooping)
		{
			mCurrentTimeInSeconds = fmod(mCurrentTimeInSeconds, mDurationInSeconds[mCurrentAnimation]);
		}
		// Otherwise clamp the time to be within the duration of the animation
		else
		{
			mCurrentTimeInSeconds = glm::clamp((float)mCurrentTimeInSeconds, 0.0f, (float)mDurationInSeconds[mCurrentAnimation]);
		}

		// Get the current keyframe
		int keyFrameTime = (int)((mCurrentTimeInSeconds / mDurationInSeconds[mCurrentAnimation]) * mDurationInTicks[mCurrentAnimation]);

		glm::mat4 identity(1.f);

		// If transitioning between animations, reduce the remaining time as it goes 
		if (mTransitionTime > 0.0f)
		{
			mTransitionTime -= dt;
			if (mTransitionTime < 0.0f)
			{
				mTransitionTime = 0.0f;
			}
		}

		// Update the bone transforms for the keyframe
		UpdateBoneHierarchy(mRootNode, identity, keyFrameTime);

		// Now update the current transforms with them
		transforms.resize(mBoneInfoVec.size());
		globals.resize(mBoneInfoVec.size());
		int numTransforms = mBoneInfoVec.size();
		for (int i = 0; i < numTransforms; i++)
		{
			transforms[i] = mBoneInfoVec[i].mFinalTransformation;
			globals[i] = mBoneInfoVec[i].mGlobalTransformation;
		}
	}
}

int Character::GetAnimationID(const char* animation)
{
	for (size_t i = 0; i < mScene->NumAnimations(); i++)
	{
		if (mScene->Animations[i]->mName.C_Str() == animation)
		{
			return i;
		}
	}
	return -1;
}

int Character::GetCurrentAnimationID()
{
	return mCurrentAnimation;
}

std::string Character::GetCurrentAnimationName()
{
	return mScene->Animations[mCurrentAnimation]->mName.C_Str();
}

AnimNode* Character::CreateNodeHierarchy(aiNode* assimpNode, int depth)
{
	AnimNode* newNode = new AnimNode();
	newNode->mName = std::string(assimpNode->mName.data);
	CastToGLM(assimpNode->mTransformation, newNode->mTransformation);

	for (int i = 0; i < assimpNode->mNumChildren; i++)
	{
		AnimNode* childNode = CreateNodeHierarchy(assimpNode->mChildren[i], depth + 1);
		newNode->AddChild(childNode);
	}

	return newNode;
}

void Character::LoadAssimpAnimation(const aiAnimation* animation)
{
	if (animation == nullptr)
		return;

	if (mNumAnimationsLoaded >= 14)
		return;

	unsigned int numChannels = animation->mNumChannels;

	std::vector<AnimationData*>& channels = mChannels[mNumAnimationsLoaded];

	mDurationInTicks[mNumAnimationsLoaded] = animation->mDuration;
	mTicksPerSecond[mNumAnimationsLoaded] = animation->mTicksPerSecond;
	mDurationInSeconds[mNumAnimationsLoaded] =
		mDurationInTicks[mNumAnimationsLoaded] / mTicksPerSecond[mNumAnimationsLoaded];

	mNumAnimationsLoaded++;

	channels.resize(numChannels);
	for (int i = 0; i < numChannels; i++)
	{
		const aiNodeAnim* nodeAnim = animation->mChannels[i];
		std::string name(nodeAnim->mNodeName.data);

		mBoneNameToAnimationMap[name] = i;

		unsigned int numPositionKeys = nodeAnim->mNumPositionKeys;
		unsigned int numRotationKeys = nodeAnim->mNumRotationKeys;
		unsigned int numScalingKeys = nodeAnim->mNumScalingKeys;

		AnimationData* animData = new AnimationData();
		channels[i] = animData;
		animData->Name = name;

		animData->PositionKeyFrames.resize(numPositionKeys);
		animData->RotationKeyFrames.resize(numRotationKeys);
		animData->ScaleKeyFrames.resize(numScalingKeys);

		for (int keyIdx = 0; keyIdx < numPositionKeys; keyIdx++)
		{
			const aiVectorKey& posKey = nodeAnim->mPositionKeys[keyIdx];
			animData->PositionKeyFrames[keyIdx].time = posKey.mTime;
			animData->PositionKeyFrames[keyIdx].value.x = posKey.mValue.x;
			animData->PositionKeyFrames[keyIdx].value.y = posKey.mValue.y;
			animData->PositionKeyFrames[keyIdx].value.z = posKey.mValue.z;
		}

		for (int keyIdx = 0; keyIdx < numRotationKeys; keyIdx++)
		{
			const aiQuatKey& rotKey = nodeAnim->mRotationKeys[keyIdx];
			animData->RotationKeyFrames[keyIdx].time = rotKey.mTime;
			animData->RotationKeyFrames[keyIdx].value.x = rotKey.mValue.x;
			animData->RotationKeyFrames[keyIdx].value.y = rotKey.mValue.y;
			animData->RotationKeyFrames[keyIdx].value.z = rotKey.mValue.z;
			animData->RotationKeyFrames[keyIdx].value.w = rotKey.mValue.w;
		}

		for (int keyIdx = 0; keyIdx < numScalingKeys; keyIdx++)
		{
			const aiVectorKey& scaleKey = nodeAnim->mScalingKeys[keyIdx];
			animData->ScaleKeyFrames[keyIdx].time = scaleKey.mTime;
			animData->ScaleKeyFrames[keyIdx].value.x = scaleKey.mValue.x;
			animData->ScaleKeyFrames[keyIdx].value.y = scaleKey.mValue.y;
			animData->ScaleKeyFrames[keyIdx].value.z = scaleKey.mValue.z;
		}
	}
}

AnimationData* Character::FindAnimationData(const std::string& nodeName, int animation)
{
	std::map<std::string, int>::iterator animIt = mBoneNameToAnimationMap.find(nodeName);

	for (int i = 0; i < mChannels[animation].size(); i++)
	{
		if (nodeName == mChannels[animation][i]->Name)
		{
			return mChannels[animation][i];
		}
	}

	if (animIt != mBoneNameToAnimationMap.end())
	{
		int breakhereplz = 0;
	}

	return nullptr;
}

void Character::UpdateBoneHierarchy(AnimNode* node, const glm::mat4& parentTransformationMatrix, float keyFrameTime)
{
	if (node == nullptr)
		return;

	std::string nodeName(node->mName);

	glm::mat4 transformationMatrix = node->mTransformation;
	if (node->mName == "RootNode")
	{
		nodeName = "Root";
	}

	AnimationData* animNode = FindAnimationData(nodeName, mCurrentAnimation);
	AnimationData* animNode2 = FindAnimationData(nodeName, mPreviousAnimation);
	if (animNode != nullptr)
	{
		// Calculate the position of this node.
		glm::vec3 position = GetAnimationPosition(*animNode, keyFrameTime);
		glm::vec3 scale = GetAnimationScale(*animNode, keyFrameTime);
		glm::quat rotation = GetAnimationRotation(*animNode, keyFrameTime);

		if (animNode2 != nullptr && mTransitionTime > 0.0f)
		{
			glm::vec3 position2 = GetAnimationPosition(*animNode2, keyFrameTime);
			glm::vec3 scale2 = GetAnimationScale(*animNode2, keyFrameTime);
			glm::quat rotation2 = GetAnimationRotation(*animNode2, keyFrameTime);

			float currRatio = 1.0f - mTransitionTime;
			float prevRatio = mTransitionTime;

			position = position * currRatio + position2 * prevRatio;
			scale = scale * currRatio + scale2 * prevRatio;
			rotation = glm::slerp(rotation2, rotation, currRatio);
		}

		// Calculate our transformation matrix
		glm::mat4 translationMatrix = glm::translate(glm::mat4(1.f), position);
		glm::mat4 rotationMatrix = glm::mat4_cast(rotation);
		glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), scale);

		transformationMatrix = translationMatrix * rotationMatrix * scaleMatrix;

		glm::mat4 globalTransformation = parentTransformationMatrix * transformationMatrix;

		if (mBoneNameToIdMap.find(nodeName) != mBoneNameToIdMap.end())
		{
			//printf("%d : %s\n", boneIt->second, nodeName.c_str());
			int boneIdx = mBoneNameToIdMap[nodeName];
			mBoneInfoVec[boneIdx].mFinalTransformation = mGlobalInverseTransform * globalTransformation * mBoneInfoVec[boneIdx].mBoneOffset;
			mBoneInfoVec[boneIdx].mGlobalTransformation = globalTransformation;
		}

		for (int i = 0; i < node->children.size(); i++)
		{
			UpdateBoneHierarchy(node->children[i], globalTransformation, keyFrameTime);
		}
	}
}

int Character::FindPositionKeyFrameIndex(const AnimationData& animation, float keyFrameTime)
{
	for (int i = 0; i < animation.PositionKeyFrames.size() - 1; i++)
	{
		if (animation.PositionKeyFrames[i + 1].time > keyFrameTime)
			return i;
	}

	return 0;
}

int Character::FindScaleKeyFrameIndex(const AnimationData& animation, float keyFrameTime)
{
	for (int i = 0; i < animation.ScaleKeyFrames.size() - 1; i++)
	{
		if (animation.ScaleKeyFrames[i + 1].time > keyFrameTime)
			return i;
	}

	return 0;
}

int Character::FindRotationKeyFrameIndex(const AnimationData& animation, float keyFrameTime)
{
	for (int i = 0; i < animation.RotationKeyFrames.size() - 1; i++)
	{
		if (animation.RotationKeyFrames[i + 1].time > keyFrameTime)
			return i;
	}

	return 0;
}

glm::vec3 Character::GetAnimationPosition(const AnimationData& animation, float keyFrameTime)
{
	if (animation.PositionKeyFrames.size() == 1)
		return animation.PositionKeyFrames[0].value;

	int positionKeyFrameIndex = FindPositionKeyFrameIndex(animation, keyFrameTime);
	int nextPositionKeyFrameIndex = positionKeyFrameIndex + 1;
	PositionKeyFrame positionKeyFrame = animation.PositionKeyFrames[positionKeyFrameIndex];
	PositionKeyFrame nextPositionKeyFrame = animation.PositionKeyFrames[nextPositionKeyFrameIndex];
	float difference = nextPositionKeyFrame.time - positionKeyFrame.time;
	float ratio = (keyFrameTime - positionKeyFrame.time) / difference;

	if (ratio < 0.0f) ratio = 0.0f;
	if (ratio > 1.0f) ratio = 1.0f;

	glm::vec3 result = (nextPositionKeyFrame.value - positionKeyFrame.value) * ratio + positionKeyFrame.value;

	return result;
}

glm::vec3 Character::GetAnimationScale(const AnimationData& animation, float keyFrameTime)
{
	if (animation.ScaleKeyFrames.size() == 1)
		return animation.ScaleKeyFrames[0].value;

	int scaleKeyFrameIndex = FindScaleKeyFrameIndex(animation, keyFrameTime);
	int nextScaleKeyFrameIndex = scaleKeyFrameIndex + 1;
	ScaleKeyFrame scaleKeyFrame = animation.ScaleKeyFrames[scaleKeyFrameIndex];
	ScaleKeyFrame nextScaleKeyFrame = animation.ScaleKeyFrames[nextScaleKeyFrameIndex];
	float difference = nextScaleKeyFrame.time - scaleKeyFrame.time;
	float ratio = (keyFrameTime - scaleKeyFrame.time) / difference;

	if (ratio < 0.0f) ratio = 0.0f;
	if (ratio > 1.0f) ratio = 1.0f;

	glm::vec3 result = (nextScaleKeyFrame.value - scaleKeyFrame.value) * ratio + scaleKeyFrame.value;

	return result;
}

glm::quat Character::GetAnimationRotation(const AnimationData& animation, float keyFrameTime)
{
	if (animation.RotationKeyFrames.size() == 1)
		return animation.RotationKeyFrames[0].value;

	int rotationKeyFrameIndex = FindRotationKeyFrameIndex(animation, keyFrameTime);
	int nextRotationKeyFrameIndex = rotationKeyFrameIndex + 1;
	RotationKeyFrame rotationKeyFrame = animation.RotationKeyFrames[rotationKeyFrameIndex];
	RotationKeyFrame nextRotationKeyFrame = animation.RotationKeyFrames[nextRotationKeyFrameIndex];
	float difference = nextRotationKeyFrame.time - rotationKeyFrame.time;
	float ratio = (keyFrameTime - rotationKeyFrame.time) / difference;

	if (ratio < 0.0f) ratio = 0.0f;
	if (ratio > 1.0f) ratio = 1.0f;

	glm::quat result;
	result = glm::slerp(rotationKeyFrame.value, nextRotationKeyFrame.value, ratio);
	result = glm::normalize(result);

	return result;
}