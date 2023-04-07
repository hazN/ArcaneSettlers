#include "Character.h"

#include <glm/gtx/easing.hpp>
#include <iostream>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include "globalThings.h"

// #define PRINT_DEBUG_INFO

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
	m_Scene = m_Importer.ReadFile(filename, flags);

	Animations = m_Scene->mAnimations;
	Cameras = m_Scene->mCameras;
	Lights = m_Scene->mLights;
	Materials = m_Scene->mMaterials;
	Meshes = m_Scene->mMeshes;
	Textures = m_Scene->mTextures;

	RootNode = m_Scene->mRootNode;
}

AssimpScene::~AssimpScene() { }

Character::Character()
	: m_IsPlaying(true)
	, m_AnimationSpeed(1.0)
	, m_CurrentTimeInSeconds(0.0)
	, m_CurrentAnimation(0)
	, m_NumAnimationsLoaded(0)
{
}

Character::~Character()
{
}

void Character::LoadAnimationFromAssimp(const char* filename)
{
	const aiScene* scene = m_AnimationImporter.ReadFile(filename, 0);

	unsigned int numAnimations = scene->mNumAnimations;
	printf("-Loading %d animations!\n", numAnimations);

	// Loop through animations
	for (unsigned int i = 0; i < numAnimations; ++i)
	{
		aiAnimation* animation = scene->mAnimations[i];
		LoadAssimpAnimation(animation);
	}
}

void Character::UpdateTransforms(std::vector<glm::mat4>& transforms, std::vector<glm::mat4>& globals, float dt)
{
	if (m_IsPlaying && m_AnimationSpeed != 0.0f)
	{
		m_CurrentTimeInSeconds += dt * m_AnimationSpeed;
		m_CurrentTimeInSeconds = fmod(
			m_CurrentTimeInSeconds, m_DurationInSeconds[m_CurrentAnimation]);

		int keyFrameTime = (int)((m_CurrentTimeInSeconds / m_DurationInSeconds[m_CurrentAnimation]) *
			m_DurationInTicks[m_CurrentAnimation]);

		glm::mat4 identity(1.f);

		if (m_TransitionTime > 0.0f)
		{
			m_TransitionTime -= dt;
			if (m_TransitionTime < 0.0f)
			{
				m_TransitionTime = 0.0f;
			}
		}

		printf("--------------------\n");
		printf("Time: %.4f %d/%d\n", m_CurrentTimeInSeconds, keyFrameTime, (int)m_DurationInTicks);

		UpdateBoneHierarchy(m_RootNode, identity, keyFrameTime);
		transforms.resize(m_BoneInfoVec.size());
		globals.resize(m_BoneInfoVec.size());
		int numTransforms = m_BoneInfoVec.size();
		for (int i = 0; i < numTransforms; i++)
		{
			transforms[i] = m_BoneInfoVec[i].finalTransformation;
			globals[i] = m_BoneInfoVec[i].globalTransformation;
		}
	}
}

AnimNode* Character::CreateNodeHierarchy(aiNode* assimpNode, int depth)
{
	AnimNode* newNode = new AnimNode();
	newNode->name = std::string(assimpNode->mName.data);
	CastToGLM(assimpNode->mTransformation, newNode->transformation);

	for (int i = 0; i < depth; i++)
		printf(" ");
	printf("%s (%d)\n", newNode->name.c_str(), assimpNode->mNumChildren);

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

	if (m_NumAnimationsLoaded >= 10)
		return;

	unsigned int numChannels = animation->mNumChannels;

	// Hacking in which animation channel to add
	std::vector<AnimationData*>& channels = m_Channels[m_NumAnimationsLoaded];

	m_DurationInTicks[m_NumAnimationsLoaded] = animation->mDuration;
	m_TicksPerSecond[m_NumAnimationsLoaded] = animation->mTicksPerSecond;
	m_DurationInSeconds[m_NumAnimationsLoaded] =
		m_DurationInTicks[m_NumAnimationsLoaded] / m_TicksPerSecond[m_NumAnimationsLoaded];

	m_NumAnimationsLoaded++;

	channels.resize(numChannels);
	for (int i = 0; i < numChannels; i++)
	{
		const aiNodeAnim* nodeAnim = animation->mChannels[i];
		std::string name(nodeAnim->mNodeName.data);
		printf("%s\n", name.c_str());

		m_BoneNameToAnimationMap[name] = i;

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
	std::map<std::string, int>::iterator animIt = m_BoneNameToAnimationMap.find(nodeName);

	//printf("%s\n", nodeName.c_str());
	for (int i = 0; i < m_Channels[animation].size(); i++)
	{
		if (nodeName == m_Channels[animation][i]->Name)
		{
			return m_Channels[animation][i];
		}
	}

	if (animIt != m_BoneNameToAnimationMap.end())
	{
		int breakhereplz = 0;
	}

	return nullptr;
}

void Character::UpdateBoneHierarchy(AnimNode* node, const glm::mat4& parentTransformationMatrix, float keyFrameTime)
{
	if (node == nullptr)
		return;

	std::string nodeName(node->name);

	glm::mat4 transformationMatrix = node->transformation;

	AnimationData* animNode = FindAnimationData(nodeName, m_CurrentAnimation);
	AnimationData* animNode2 = FindAnimationData(nodeName, m_PreviousAnimation);
	if (animNode != nullptr)
	{
		// Calculate the position of this node.
		glm::vec3 position = GetAnimationPosition(*animNode, keyFrameTime);
		glm::vec3 scale = GetAnimationScale(*animNode, keyFrameTime);
		glm::quat rotation = GetAnimationRotation(*animNode, keyFrameTime);

		if (animNode2 != nullptr && m_TransitionTime > 0.0f)
		{
			glm::vec3 position2 = GetAnimationPosition(*animNode2, keyFrameTime);
			glm::vec3 scale2 = GetAnimationScale(*animNode2, keyFrameTime);
			glm::quat rotation2 = GetAnimationRotation(*animNode2, keyFrameTime);

			float currRatio = 1.0f - m_TransitionTime;
			float prevRatio = m_TransitionTime;

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

		if (m_BoneNameToIdMap.find(nodeName) != m_BoneNameToIdMap.end())
		{
			//printf("%d : %s\n", boneIt->second, nodeName.c_str());
			int boneIdx = m_BoneNameToIdMap[nodeName];
			m_BoneInfoVec[boneIdx].finalTransformation = m_GlobalInverseTransform * globalTransformation * m_BoneInfoVec[boneIdx].boneOffset;
			m_BoneInfoVec[boneIdx].globalTransformation = globalTransformation;
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
	// Assert animation.PositionKeyFrames.size() > 0

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

	//glm::vec3 result = glm::mix(positionKeyFrame.value, nextPositionKeyFrame.value, ratio);
	glm::vec3 result = (nextPositionKeyFrame.value - positionKeyFrame.value) * ratio + positionKeyFrame.value;

	return result;
}

glm::vec3 Character::GetAnimationScale(const AnimationData& animation, float keyFrameTime)
{
	// Assert animation.ScaleKeyFrames.size() > 0

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

	//glm::vec3 result = glm::mix(scaleKeyFrame.value, nextScaleKeyFrame.value, ratio);
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