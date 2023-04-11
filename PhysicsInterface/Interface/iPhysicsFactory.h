#pragma once

#include "iPhysicsWorld.h"
#include "iRigidBody.h"
#include "iSoftBody.h"
#include "RigidBodyDesc.h"
#include "SoftBodyDesc.h"
#include "iShape.h"
#include "iCharacterController.h"

namespace physics
{
	class iPhysicsFactory
	{
	public:
		virtual ~iPhysicsFactory() {}

		virtual iPhysicsWorld* CreateWorld() = 0;
		virtual iRigidBody* CreateRigidBody(const RigidBodyDesc& desc, iShape* shape) = 0;
		virtual iSoftBody* CreateSoftBody(const SoftBodyDesc& desc) = 0;
		virtual iCharacterController* CreateCharacterController(iShape* shape, const Vector3& position, const Quaternion& orientation) = 0;
	protected:
		iPhysicsFactory() {}

	private:
		iPhysicsFactory(const iPhysicsFactory&) {}
		iPhysicsFactory& operator=(const iPhysicsFactory&) {
			return *this;
		}
	};
}
