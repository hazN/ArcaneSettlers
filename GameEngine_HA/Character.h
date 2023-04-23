#pragma once
#pragma once

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Animation.h"
#include "cMeshObject.h"
#include "BoneHierarchy.h"

class AssimpScene
{
public:
	AssimpScene(const char* filename, unsigned int flags);
	~AssimpScene();

	inline bool HasAnimations() const { return mScene->HasAnimations(); }
	inline bool HasCameras() const { return mScene->HasCameras(); }
	inline bool HasLights() const { return mScene->HasLights(); }
	inline bool HasMaterials() const { return mScene->HasMaterials(); }
	inline bool HasMeshes() const { return mScene->HasMeshes(); }
	inline bool HasTextures() const { return mScene->HasTextures(); }

	inline unsigned int NumAnimations() const { return mScene->mNumAnimations; }
	inline unsigned int NumCameras() const { return mScene->mNumCameras; }
	inline unsigned int NumLights() const { return mScene->mNumLights; }
	inline unsigned int NumMaterials() const { return mScene->mNumMaterials; }
	inline unsigned int NumMeshes() const { return mScene->mNumMeshes; }
	inline unsigned int NumTextures() const { return mScene->mNumTextures; }

	aiNode* RootNode;
	aiAnimation** Animations;
	aiCamera** Cameras;
	aiLight** Lights;
	aiMaterial** Materials;
	aiMesh** Meshes;
	aiTexture** Textures;

private:
	Assimp::Importer mImporter;
	const aiScene* mScene;
};

class Character
{
public:
	Character();
	~Character();

	// Load Character, AKA BONES
	void LoadCharacterFromAssimp(const char* filename);
	// Load Animations from file(will load multiple from single file)
	void LoadAnimationFromAssimp(const char* filename);
	// Load Bones
	void LoadAssimpBones(const aiMesh* assimpMesh);
	void LoadAssimpBones(const aiMesh* mainMesh, const std::vector<aiMesh*> additionalMeshes);
	// Update the Bones
	void UpdateTransforms(std::vector<glm::mat4>& transforms, std::vector<glm::mat4>& globals, float dt);
	// Get Animation by ID or Name
	int GetAnimationID(const char* animation);
	int GetCurrentAnimationID();
	std::string GetCurrentAnimationName();
	void SetAnimation(int animationId, float time = 1.f) {
		mTransitionTime = time;
		mPreviousAnimation = mCurrentAnimation;
		mCurrentAnimation = animationId;
		mCurrentTimeInSeconds = 0;
	}
	// Render
	cMeshObject* Mesh;
	bool m_IsLooping;
	bool UseAssimp;

private:
	// Loading
	AnimNode* CreateNodeHierarchy(aiNode* animNode, int depth = 0);
	void LoadAssimpAnimation(const aiAnimation* animation);
	AnimationData* FindAnimationData(const std::string& nodeName, int animation);
	// Animating
	void UpdateBoneHierarchy(AnimNode* node, const glm::mat4& parentTransformationMatrix, float keyFrameTime);

	// Find the transforms for specific keyframes
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
	Assimp::Importer mAnimationImporter;
	AssimpScene* mScene;

	std::string mName;
	AnimNode* mRootNode;
	glm::mat4 mGlobalInverseTransform;

	// Bone Data
	std::vector<BoneVertexData> mBoneVertexData;		// used for rendering
	std::vector<BoneInfo> mBoneInfoVec;				// Used for offsets
	std::vector<BoneInfo> mBoneInfoVecChildren;		// Used for offsets
	std::map<std::string, int> mBoneNameToIdMap;		// Used for lookups
	std::vector<aiBone*> mBoneVec;						// Used for lookups

	// Animation info
	int mNumAnimationsLoaded;
	int mCurrentAnimation;
	// Used for blending
	int mPreviousAnimation;
	float mTransitionTime;
	// Channels, aka amount of animations max per character
	std::vector<AnimationData*> mChannels[20];
	double mDurationInTicks[20];
	double mTicksPerSecond[20];
	double mDurationInSeconds[20];

	std::map<std::string, int> mBoneNameToAnimationMap;

	double mCurrentTimeInSeconds;

	bool mIsPlaying;
	double mAnimationSpeed;
};
