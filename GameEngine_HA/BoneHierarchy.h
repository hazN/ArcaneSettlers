#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <string>

struct BoneNode
{
	void AddChild(BoneNode* child)
	{
		child->mParent = this;
		children.push_back(child);
	}

	int boneId;
	std::string mName;
	BoneNode* mParent;
	glm::mat4 mTransformation;
	glm::mat4 mFinalTransformation;
	std::vector<BoneNode*> children;
};

typedef BoneNode AnimNode;


struct BoneHierarchy
{
	BoneNode* mRoot;
	glm::mat4 mGlobalInverseTransformation;
};