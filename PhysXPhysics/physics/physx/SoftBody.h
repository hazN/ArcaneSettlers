#pragma once
#include <vector>

#include <Interface/iSoftBody.h>
#include <Interface/SoftBodyDesc.h>

namespace physics
{
	class SoftBody : public iSoftBody
	{
	public:
		SoftBody(const SoftBodyDesc& desc);
		virtual ~SoftBody(void);

		static SoftBody* Cast(iCollisionBody* body);

		virtual unsigned int GetNumNodes();
		virtual void GetNodePosition(unsigned int nodeIndex, glm::vec3& nodePositionOut);
	};
};

