#include "PhysicsWorld.h"

#include <iostream>

#include <algorithm>
#include <math.h>
#include "RigidBody.h"
#include <Interface/SphereShape.h>
#include <Interface/BoxShape.h>
#include <Interface/CylinderShape.h>
#include "../../../GameEngine_HA/cVAOManager/sModelDrawInfo.h"
#include <physx/cooking/PxCooking.h>
#include <physx/geometry/PxHeightField.h>
#include <physx/geometry/PxHeightFieldDesc.h>
#include <Interface/TriangleMeshShape.h>
#include "CharacterController.h"

//#include <pvd/PxPvd.h>
//#include <extensions/PxSimpleFactory.h>

namespace physics
{
	physx::PxPhysics* PhysicsWorld::mPhysics = nullptr;
	physx::PxScene* PhysicsWorld::mScene = nullptr;
	physx::PxMaterial* PhysicsWorld::mMaterial = nullptr;
	physx::PxCooking* PhysicsWorld::mCooking = nullptr;
	physx::PxControllerManager* PhysicsWorld::mControllerManager = nullptr;
	CRITICAL_SECTION PhysicsWorld::physicsCriticalSection;
	std::map<iCharacterController*, PxController*> PhysicsWorld::mMapControllers;
	PhysicsWorld::PhysicsWorld(void)
	{
		InitializeCriticalSection(&physicsCriticalSection);
		mFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, mDefaultAllocatorCallback, mDefaultErrorCallback);
		if (!mFoundation) throw("PxCreateFoundation failed!");
		mPvd = PxCreatePvd(*mFoundation);
		mTransport = physx::PxDefaultPvdSocketTransportCreate("127.0.0.1", 5425, 10);
		mPvd->connect(*mTransport, physx::PxPvdInstrumentationFlag::eALL);
		mToleranceScale.length = 100;        // typical length of an object
		mToleranceScale.speed = 981;         // typical speed of an object, gravity*1s is a reasonable choice
		mPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *mFoundation, mToleranceScale, true, mPvd);
		mCooking = PxCreateCooking(PX_PHYSICS_VERSION, *mFoundation, physx::PxCookingParams(mToleranceScale));
		physx::PxSceneDesc sceneDesc(mPhysics->getTolerancesScale());
		sceneDesc.gravity = physx::PxVec3(0.0f, -9.81f, 0.0f);
		mDispatcher = physx::PxDefaultCpuDispatcherCreate(2);
		sceneDesc.cpuDispatcher = mDispatcher;
		sceneDesc.filterShader = physx::PxDefaultSimulationFilterShader;
		mScene = mPhysics->createScene(sceneDesc);
		mControllerManager = PxCreateControllerManager(*mScene);

		physx::PxPvdSceneClient* pvdClient = mScene->getScenePvdClient();
		if (pvdClient)
		{
			pvdClient->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
			pvdClient->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
			pvdClient->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
		}
		mMaterial = mPhysics->createMaterial(0.5f, 0.5f, 0.6f);
	}

	PhysicsWorld::~PhysicsWorld(void)
	{
		mActors.clear();
		mScene->release();
		mDispatcher->release();
		mPvd->release();
		mTransport->release();
		mFoundation->release();
		mPhysics->release();
		mCooking->release();
		mControllerManager->release();
	}

	void PhysicsWorld::TimeStep(float dt)
	{
		EnterCriticalSection(&physicsCriticalSection);
		mScene->simulate(1.0f / 60.0f);
		mScene->fetchResults(true);
		LeaveCriticalSection(&physicsCriticalSection);
	}

	void PhysicsWorld::SetGravity(const Vector3& gravity)
	{
	}

	void PhysicsWorld::AddCharacterController(iCharacterController* characterController)
	{
		if (characterController == nullptr)
			return;
		CharacterController* character = (CharacterController*)characterController;
		mCharacterControllers.push_back(character);
	}

	void PhysicsWorld::RemoveCharacterController(iCharacterController* characterController)
	{
		EnterCriticalSection(&physicsCriticalSection);
		PxController* pxController = mMapControllers[characterController];
		if (pxController)
		{
			pxController->release();
			mMapControllers.erase(characterController);
			for (size_t i = 0; i < mCharacterControllers.size(); i++)
			{
				if (mCharacterControllers[i] == characterController)
				{
					delete mCharacterControllers[i];
					mCharacterControllers.erase(mCharacterControllers.begin() + i);
					break;
				}
			}
		}
		LeaveCriticalSection(&physicsCriticalSection);
	}


	void PhysicsWorld::AddBody(iCollisionBody* body)
	{
		if (body == nullptr)
			return;
		if (body->GetBodyType() == BodyType::RigidBody)
		{
			int userData;
			RigidBody* rigidBody = RigidBody::Cast(body);
			physx::PxTransform transform(physx::PxVec3(rigidBody->originalPosition.x, rigidBody->originalPosition.y, rigidBody->originalPosition.z), physx::PxQuat(rigidBody->originalRotation.x, rigidBody->originalRotation.y, rigidBody->originalRotation.z, rigidBody->originalRotation.w));
			if (rigidBody->GetShape()->GetShapeType() == ShapeType::Sphere)
			{
				SphereShape* sphereShape = (SphereShape*)rigidBody->GetShape();
				rigidBody->pShape = PhysicsWorld::mPhysics->createShape(physx::PxSphereGeometry(sphereShape->GetRadius()), *PhysicsWorld::mMaterial);
				userData = sphereShape->GetUserData();
			}
			else if (rigidBody->GetShape()->GetShapeType() == ShapeType::Box)
			{
				BoxShape* boxShape = (BoxShape*)rigidBody->GetShape();
				physx::PxVec3 halfExtents(boxShape->GetHalfExtents().x, boxShape->GetHalfExtents().y, boxShape->GetHalfExtents().z);
				rigidBody->pShape = PhysicsWorld::mPhysics->createShape(physx::PxBoxGeometry(halfExtents), *PhysicsWorld::mMaterial);
				userData = boxShape->GetUserData();
			}
			else if (rigidBody->GetShape()->GetShapeType() == ShapeType::Cylinder)
			{
				CylinderShape* cylinderShape = (CylinderShape*)rigidBody->GetShape();
				rigidBody->pShape = PhysicsWorld::mPhysics->createShape(physx::PxCapsuleGeometry(cylinderShape->GetHalfExtents().x, cylinderShape->GetHalfExtents().y), *PhysicsWorld::mMaterial);
				userData = cylinderShape->GetUserData();
			}
			else if (rigidBody->GetShape()->GetShapeType() == ShapeType::TriangleMesh)
			{
				TriangleMeshShape* triangleMeshShape = (TriangleMeshShape*)rigidBody->GetShape();
				const std::vector<Vector3>& vertices = triangleMeshShape->GetVertices();
				const std::vector<unsigned int>& indices = triangleMeshShape->GetIndices();

				physx::PxTriangleMeshDesc meshDesc;
				meshDesc.points.count = (physx::PxU32)vertices.size();
				meshDesc.points.stride = sizeof(Vector3);
				meshDesc.points.data = vertices.data();

				meshDesc.triangles.count = (physx::PxU32)indices.size() / 3;
				meshDesc.triangles.stride = 3 * sizeof(physx::PxU32);
				meshDesc.triangles.data = indices.data();

				physx::PxTriangleMesh* triangleMesh = NULL;
				physx::PxDefaultMemoryOutputStream writeBuffer;
				if (mCooking->cookTriangleMesh(meshDesc, writeBuffer))
				{
					physx::PxDefaultMemoryInputData readBuffer(writeBuffer.getData(), writeBuffer.getSize());
					triangleMesh = mPhysics->createTriangleMesh(readBuffer);
				}

				if (triangleMesh)
				{
					physx::PxTriangleMeshGeometry triangleMeshGeom(triangleMesh);
					rigidBody->pShape = PhysicsWorld::mPhysics->createShape(triangleMeshGeom, *PhysicsWorld::mMaterial);
				}
				userData = triangleMeshShape->GetUserData();
			}
			rigidBody->pShape->userData = (void*)userData;
			EnterCriticalSection(&physicsCriticalSection);
			if (rigidBody->IsStatic())
			{
				rigidBody->rigidBody = PhysicsWorld::mPhysics->createRigidStatic(transform);
				rigidBody->rigidBody->attachShape(*rigidBody->pShape);
				if (rigidBody->GetShape()->GetShapeType() == ShapeType::Cylinder)
				{
					PxTransform transform = rigidBody->rigidBody->getGlobalPose();
					transform.q.w = rigidBody->originalRotation.w;
					transform.q.x = rigidBody->originalRotation.x;
					transform.q.y = rigidBody->originalRotation.y;
					transform.q.z = rigidBody->originalRotation.z;
				}
			}
			else
			{
				//rigidBody->rigidBody = PhysicsWorld::mPhysics->createRigidDynamic(transform);
				physx::PxRigidDynamic* rigidActor = mPhysics->createRigidDynamic(transform);
				rigidActor->attachShape(*rigidBody->pShape);
				physx::PxRigidBodyExt::updateMassAndInertia(*rigidActor, rigidBody->m_Mass);
				rigidBody->rigidBody = rigidActor;
			}
			LeaveCriticalSection(&physicsCriticalSection);
			mActors.push_back((iRigidBody*)rigidBody);
			mScene->addActor(*rigidBody->rigidBody);
			mOriginalTransforms.emplace(rigidBody->rigidBody, transform);
			rigidBody->pShape->release();
		}
	}

	void PhysicsWorld::RemoveBody(iCollisionBody* body)
	{
		EnterCriticalSection(&physicsCriticalSection);
		for (size_t i = 0; i < mActors.size(); i++)
		{
			if (mActors[i] == body)
			{
				RigidBody* bodyToDelete = (RigidBody*)mActors[i];
				bodyToDelete->rigidBody->release();
				mActors.erase(mActors.begin() + i);
				delete body;
			}
		}
		LeaveCriticalSection(&physicsCriticalSection);
	}
	void PhysicsWorld::ResetWorld()
	{
		// Loop through actors
		EnterCriticalSection(&physicsCriticalSection);
		for (iRigidBody* body : mActors)
		{
			RigidBody* actor = RigidBody::Cast(body);
			physx::PxTransform originalTransform;
			physx::PxVec3 originalVelocity;
			originalTransform = mOriginalTransforms[actor->rigidBody];
			if (!actor->IsStatic())
			{
				physx::PxRigidDynamic* dynamicActor = (physx::PxRigidDynamic*)actor->rigidBody;
				dynamicActor->setLinearVelocity(physx::PxVec3(0.0f));
				dynamicActor->setAngularVelocity(physx::PxVec3(0.0f));
			}
			actor->rigidBody->setGlobalPose(originalTransform);
		}
		LeaveCriticalSection(&physicsCriticalSection);
	}
};