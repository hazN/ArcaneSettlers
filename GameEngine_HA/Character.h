#pragma once
#pragma once

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Animation.h"
#include "cMeshObject.h"
#include "BoneHierarchy.h"
enum NodeNames
{
	Head,
	RightHand,
	LeftHand
};

class AssimpScene
{
public:
	AssimpScene(const char* filename, unsigned int flags);
	~AssimpScene();

	inline bool HasAnimations() const { return m_Scene->HasAnimations(); }
	inline bool HasCameras() const { return m_Scene->HasCameras(); }
	inline bool HasLights() const { return m_Scene->HasLights(); }
	inline bool HasMaterials() const { return m_Scene->HasMaterials(); }
	inline bool HasMeshes() const { return m_Scene->HasMeshes(); }
	inline bool HasTextures() const { return m_Scene->HasTextures(); }

	inline unsigned int NumAnimations() const { return m_Scene->mNumAnimations; }
	inline unsigned int NumCameras() const { return m_Scene->mNumCameras; }
	inline unsigned int NumLights() const { return m_Scene->mNumLights; }
	inline unsigned int NumMaterials() const { return m_Scene->mNumMaterials; }
	inline unsigned int NumMeshes() const { return m_Scene->mNumMeshes; }
	inline unsigned int NumTextures() const { return m_Scene->mNumTextures; }

	aiNode* RootNode;
	aiAnimation** Animations;
	aiCamera** Cameras;
	aiLight** Lights;
	aiMaterial** Materials;
	aiMesh** Meshes;
	aiTexture** Textures;

private:
	Assimp::Importer m_Importer;
	const aiScene* m_Scene;
};
struct ChildCharacter {
	std::string attachedNodeName;
	aiMesh* mesh;
	aiBone* attachedNode;
	int iAttachedNode;
};
class Character
{
public:
	Character();
	~Character();

	// Loading
	void LoadCharacterFromAssimp(const char* filename);
	void AttachMeshToBone(cMeshObject* mesh, const char* boneName, glm::vec3 offset, glm::quat rotationOffset);
	//void AttachMeshToBone(cMeshObject* mesh, const char* boneName, glm::vec3 offset);
	void LoadAnimationFromAssimp(const char* filename);
	void LoadAssimpBones(const aiMesh* assimpMesh);
	void LoadAssimpBones(const aiMesh* mainMesh, const std::vector<aiMesh*> additionalMeshes);
	//void LoadAssimpBones(std::vector<aiMesh*> assimpMeshes);
	void UpdateTransforms(std::vector<glm::mat4>& transforms, std::vector<glm::mat4>& globals, float dt);
	//void AttachTool(cMeshObject* tool, std::string nodeName);

	void SetAnimation(int animationId, float time = 1.f) {
		m_TransitionTime = time;
		m_PreviousAnimation = m_CurrentAnimation;
		m_CurrentAnimation = animationId;
		m_CurrentTimeInSeconds = 0;
	}

	// Render
	cMeshObject* Mesh;

	bool UseAssimp;

private:
	// Loading
	AnimNode* CreateNodeHierarchy(aiNode* animNode, int depth = 0);
	void LoadAssimpAnimation(const aiAnimation* animation);

	AnimationData* FindAnimationData(const std::string& nodeName, int animation);

	// Animating
	void UpdateBoneHierarchy(AnimNode* node, const glm::mat4& parentTransformationMatrix, float keyFrameTime);

	int FindPositionKeyFrameIndex(const AnimationData& animation, float keyFrameTime);
	int FindScaleKeyFrameIndex(const AnimationData& animation, float keyFrameTime);
	int FindRotationKeyFrameIndex(const AnimationData& animation, float keyFrameTime);

	glm::vec3 GetAnimationPosition(const AnimationData& animation, float keyFrameTime);
	glm::vec3 GetAnimationScale(const AnimationData& animation, float keyFrameTime);
	glm::quat GetAnimationRotation(const AnimationData& animation, float keyFrameTime);

private:
	// Utilities
	void CastToGLM(const aiVector3D& in, glm::vec3& out);
	void CastToGLM(const aiQuaternion& in, glm::quat& out);
	void CastToGLM(const aiMatrix4x4& in, glm::mat4& out);

private:
	// Assimp
	Assimp::Importer m_AnimationImporter;
	AssimpScene* m_Scene;

	std::string m_Name;

	AnimNode* m_RootNode;
	glm::mat4 m_GlobalInverseTransform;

	std::vector<ChildCharacter*> m_ChildCharacters;

	std::vector<AnimNode*> m_BodyNodes;
	std::map<NodeNames, int> m_NameToBodyNodeIndex;

	// Bones
	std::vector<BoneVertexData> m_BoneVertexData;		// Just need for Rendering vertex info
	std::vector<BoneInfo> m_BoneInfoVec;				// This is used for offsets and final matrix
	std::vector<BoneInfo> m_BoneInfoVecChildren;				// This is used for offsets and final matrix
	std::map<std::string, int> m_BoneNameToIdMap;		// Used for bone lookups
	std::vector<aiBone*> m_BoneVec;		// Used for bone lookups
	// Animation (Supports 2 animations)
	int m_NumAnimationsLoaded;
	int m_CurrentAnimation;
	int m_PreviousAnimation;
	float m_TransitionTime;

	std::vector<AnimationData*> m_Channels[20];
	double m_DurationInTicks[20];
	double m_TicksPerSecond[20];
	double m_DurationInSeconds[20];

	std::map<std::string, int> m_BoneNameToAnimationMap;

	double m_CurrentTimeInSeconds;

	bool m_IsPlaying;
	double m_AnimationSpeed;
};
