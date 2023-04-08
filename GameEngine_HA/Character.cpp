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

void Character::LoadCharacterFromAssimp(const char* filename)
{
	unsigned int flags =
		aiProcess_Triangulate
		| aiProcess_LimitBoneWeights
		| aiProcess_JoinIdenticalVertices;
	m_Scene = new AssimpScene(filename, flags);

	aiMatrix4x4 g = m_Scene->RootNode->mTransformation;
	aiMatrix4x4 inverse = g.Inverse();

	// Node hierarchy for rendering
	m_RootNode = CreateNodeHierarchy(m_Scene->RootNode);
	CastToGLM(inverse, m_GlobalInverseTransform);
	if (m_Scene->HasMeshes())
	{
		aiMesh* mesh = m_Scene->Meshes[0];
		for (size_t i = 0; i < m_Scene->NumMeshes(); i++)
		{
			if (m_Scene->Meshes[i]->HasBones())
			{
				std::cout << "Mesh #" << i << " has bones!" << std::endl;
				mesh = m_Scene->Meshes[i];
			}
		}
		LoadAssimpBones(mesh);
	}
}

void Character::AttachMeshToBone(cMeshObject* mesh, const char* boneName, glm::vec3 offset, glm::quat rotationOffset)
{
	std::map<std::string, int>::iterator boneIt = m_BoneNameToIdMap.find(boneName);

	if (boneIt != m_BoneNameToIdMap.end()) {
		int nodeIndex = boneIt->second;
		aiBone* foundNode = m_BoneVec[nodeIndex];
		ChildCharacter* child = new ChildCharacter();
		child->mesh = mesh;
		child->offset = offset;
		child->attachedNode = foundNode;
		child->attachedNodeName = boneName;
		child->iAttachedNode = nodeIndex;
		child->rotationOffset = rotationOffset;
		m_ChildCharacters.push_back(child);
	}
	else {
		std::cout << "Error: could not find node " << boneName << " for the character " << this->m_Name << std::endl;
	}
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
		std::cout << "Loading animation: " << scene->mAnimations[i]->mName.C_Str() << std::endl;
		LoadAssimpAnimation(animation);
	}
}

void Character::LoadAssimpBones(const aiMesh* assimpMesh)
{
	// Record Vertex Weights
	int totalWeights = 0;
	m_BoneVertexData.resize(assimpMesh->mNumVertices);
	int boneCount = 0;

	int numBones = assimpMesh->mNumBones;
	for (int i = 0; i < numBones; i++)
	{
		aiBone* bone = assimpMesh->mBones[i];

		int boneIdx = 0;
		std::string boneName(bone->mName.data);

		printf("%d: %s\n", i, boneName.c_str());

		std::map<std::string, int>::iterator it = m_BoneNameToIdMap.find(boneName);
		if (it == m_BoneNameToIdMap.end())
		{
			boneIdx = boneCount;
			boneCount++;
			BoneInfo bi;
			bi.name = boneName;
			m_BoneInfoVec.push_back(bi);

			CastToGLM(bone->mOffsetMatrix, m_BoneInfoVec[boneIdx].boneOffset);
			m_BoneNameToIdMap[boneName] = boneIdx;
			m_BoneVec.push_back(bone);
		}
		else
		{
			boneIdx = it->second;
		}

		for (int weightIdx = 0; weightIdx < bone->mNumWeights; weightIdx++)
		{
			float weight = bone->mWeights[weightIdx].mWeight;
			int vertexId = bone->mWeights[weightIdx].mVertexId;
			m_BoneVertexData[vertexId].AddBoneInfo(boneIdx, weight);
		}
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

	//for (int i = 0; i < depth; i++)
	//	printf(" ");
	//printf("%s (%d)\n", newNode->name.c_str(), assimpNode->mNumChildren);

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

	if (m_NumAnimationsLoaded >= 14)
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
		//printf("%s\n", name.c_str());

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
	if (node->name == "RootNode")
	{
		nodeName = "Root";
	}
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

			if (!m_ChildCharacters.empty())
			{
				for (ChildCharacter* childChar : m_ChildCharacters)
				{
					if (node->name.c_str() == childChar->attachedNodeName)
					{
						// Use decompose to get the required vec3 and quat variables
						glm::vec3 translation, scale, skew;
						glm::vec4 perspective;
						glm::quat rotation;
						glm::decompose(m_BoneInfoVec[boneIdx].finalTransformation, scale, rotation, translation, skew, perspective);

						// Apply the scale to the offset and then add it to the translation
						glm::vec3 scaledOffset = childChar->offset * childChar->mesh->scaleXYZ;
						childChar->mesh->position = (translation / scale) * childChar->mesh->scaleXYZ + scaledOffset;
						childChar->mesh->qRotation = rotation;// *childChar->rotationOffset);
					}
				}
			}

		}

		for (int i = 0; i < node->children.size(); i++)
		{
			UpdateBoneHierarchy(node->children[i], globalTransformation, keyFrameTime);
		}
	}
}
//void Character::AttachTool(cMeshObject* tool, std::string nodeName) 
//{
//	std::map<std::string, int>::iterator boneIt = m_BoneNameToIdMap.find(nodeName);
//
//	if (boneIt != m_BoneNameToIdMap.end()) {
//		int nodeIndex = boneIt->second;
//		aiBone* foundNode = m_BoneVec[nodeIndex];
//		this->m_Tool = new Tool();
//		this->m_Tool->mesh = tool;
//		this->m_Tool->attachedNode = foundNode;
//		this->m_Tool->attachedNodeName = nodeName;
//		this->m_Tool->iAttachedNode = nodeIndex;
//	}
//	else {
//		std::cout << "Error: could not find node " << nodeName << " for the character " << this->m_Name << std::endl;
//	}
//}

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