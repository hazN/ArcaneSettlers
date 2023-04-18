#pragma once
#include <vector>

// Physics Interfaces
#include <Interface/iPhysicsWorld.h>
#include <Interface/iCollisionBody.h>
#include <Interface/iRigidBody.h>
#include <Interface/iCharacterController.h>

#include <glm/vec3.hpp>

#include <physx/PxPhysicsAPI.h>
#include <map>
#include <Windows.h>
//#include <physx/foundation/PxFoundation.h>
//#include <physx/foundation/PxErrorCallback.h>
//#include <physx/foundation/PxAllocator.h>
//#include <physx/foundation/PxAllocatorCallback.h>

using namespace physx;

namespace physics
{
	class UserErrorCallback : public PxErrorCallback
	{
	public:
		virtual void reportError(PxErrorCode::Enum code, const char* message, const char* file, int line)
		{
		}
	};

	class UserAllocatorCallback : public PxAllocatorCallback
	{
	public:
		virtual ~UserAllocatorCallback() {}
		virtual void* allocate(size_t size, const char* typeName, const char* filename,
			int line)
		{
			return 0;
		}
		virtual void deallocate(void* ptr)
		{
		}
	};

	class PhysicsWorld : public iPhysicsWorld
	{
	public:
		PhysicsWorld();
		virtual ~PhysicsWorld();

		virtual void TimeStep(float dt) override;

		virtual void SetGravity(const Vector3& gravity) override;

		virtual void AddCharacterController(iCharacterController* characterController) override;
		virtual void RemoveCharacterController(iCharacterController* characterController) override;
		virtual void AddBody(iCollisionBody* body) override;
		virtual void RemoveBody(iCollisionBody* body) override;
		virtual void ResetWorld() override;

		virtual void RegisterCollisionListener(iCollisionListener* listener) override
		{
			m_CollisionListener = listener;
		}

		static physx::PxPhysics* mPhysics;
		static physx::PxScene* mScene;
		static physx::PxMaterial* mMaterial;
		static physx::PxCooking* mCooking;
		static physx::PxControllerManager* mControllerManager;
		static CRITICAL_SECTION physicsCriticalSection;
		static std::map<iCharacterController*, PxController*> mMapControllers;
	private:
		physx::PxDefaultAllocator      mDefaultAllocatorCallback;
		physx::PxDefaultErrorCallback  mDefaultErrorCallback;
		physx::PxDefaultCpuDispatcher* mDispatcher = NULL;
		physx::PxTolerancesScale       mToleranceScale;

		physx::PxFoundation* mFoundation = NULL;
		physx::PxPvdTransport* mTransport;
		physx::PxPvd* mPvd = NULL;
		//std::map<iRigidBody*,physx::PxRigidActor*> mActors;
		std::vector<iRigidBody*> mActors;
		std::vector<iCharacterController*> mCharacterControllers;
		std::map<physx::PxRigidActor*, physx::PxTransform> mOriginalTransforms;

		iCollisionListener* m_CollisionListener;

		PhysicsWorld(const PhysicsWorld& other) { ; }
		PhysicsWorld& operator=(const PhysicsWorld& other) { return *this; }
	};
}