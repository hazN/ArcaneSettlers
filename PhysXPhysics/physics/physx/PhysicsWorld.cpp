#include "PhysicsWorld.h"

#include <iostream>

#include <algorithm>
#include <math.h>
#include "RigidBody.h"
#include <Interface/SphereShape.h>
#include <Interface/BoxShape.h>
#include <Interface/CylinderShape.h>
#include <Interface/HeightFieldShape.h>

//#include <pvd/PxPvd.h>
//#include <extensions/PxSimpleFactory.h>

namespace physics
{
	physx::PxPhysics* PhysicsWorld::mPhysics = nullptr;
	physx::PxScene* PhysicsWorld::mScene = nullptr;
	physx::PxMaterial* PhysicsWorld::mMaterial = nullptr;

	PhysicsWorld::PhysicsWorld(void)
	{
		mFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, mDefaultAllocatorCallback, mDefaultErrorCallback);
		if (!mFoundation) throw("PxCreateFoundation failed!");
		mPvd = PxCreatePvd(*mFoundation);
		mTransport = physx::PxDefaultPvdSocketTransportCreate("127.0.0.1", 5425, 10);
		mPvd->connect(*mTransport, physx::PxPvdInstrumentationFlag::eALL);
		mToleranceScale.length = 100;        // typical length of an object
		mToleranceScale.speed = 981;         // typical speed of an object, gravity*1s is a reasonable choice
		mPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *mFoundation, mToleranceScale, true, mPvd);
		physx::PxSceneDesc sceneDesc(mPhysics->getTolerancesScale());
		sceneDesc.gravity = physx::PxVec3(0.0f, -9.81f, 0.0f);
		mDispatcher = physx::PxDefaultCpuDispatcherCreate(2);
		sceneDesc.cpuDispatcher = mDispatcher;
		sceneDesc.filterShader = physx::PxDefaultSimulationFilterShader;
		mScene = mPhysics->createScene(sceneDesc);

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
	}

	void PhysicsWorld::TimeStep(float dt)
	{
		mScene->simulate(1.0f / 60.0f);
		mScene->fetchResults(true);
	}

	void PhysicsWorld::SetGravity(const Vector3& gravity)
	{
	}

	void PhysicsWorld::AddBody(iCollisionBody* body)
	{
		if (body == nullptr)
			return;
		if (body->GetBodyType() == BodyType::RigidBody)
		{
			RigidBody* rigidBody = RigidBody::Cast(body);
			physx::PxTransform transform(physx::PxVec3(rigidBody->originalPosition.x, rigidBody->originalPosition.y, rigidBody->originalPosition.z), physx::PxQuat(rigidBody->originalRotation.x, rigidBody->originalRotation.y, rigidBody->originalRotation.z, rigidBody->originalRotation.w));
			if (rigidBody->GetShape()->GetShapeType() == ShapeType::Sphere)
			{
				SphereShape* sphereShape = (SphereShape*)rigidBody->GetShape();
				rigidBody->pShape = PhysicsWorld::mPhysics->createShape(physx::PxSphereGeometry(sphereShape->GetRadius()), *PhysicsWorld::mMaterial);
			}
			else if (rigidBody->GetShape()->GetShapeType() == ShapeType::Box)
			{
				BoxShape* boxShape = (BoxShape*)rigidBody->GetShape();
				physx::PxVec3 halfExtents(boxShape->GetHalfExtents().x, boxShape->GetHalfExtents().y, boxShape->GetHalfExtents().z);
				rigidBody->pShape = PhysicsWorld::mPhysics->createShape(physx::PxBoxGeometry(halfExtents), *PhysicsWorld::mMaterial);
			}
			else if (rigidBody->GetShape()->GetShapeType() == ShapeType::Cylinder)
			{
				CylinderShape* cylinderShape = (CylinderShape*)rigidBody->GetShape();
				rigidBody->pShape = PhysicsWorld::mPhysics->createShape(physx::PxCapsuleGeometry(cylinderShape->GetHalfExtents().x, cylinderShape->GetHalfExtents().y), *PhysicsWorld::mMaterial);

			}
			else if (rigidBody->GetShape()->GetShapeType() == ShapeType::HeightField)
			{
				HeightFieldShape* heightFieldShape = (HeightFieldShape*)rigidBody->GetShape();

				// Create heightfield desc
				physx::PxHeightFieldDesc heightFieldDesc;
				heightFieldDesc.nbRows = heightFieldShape->GetWidth();
				heightFieldDesc.nbColumns = heightFieldShape->GetDepth();
				heightFieldDesc.samples.data = heightFieldShape->GetHeightData().data();
				heightFieldDesc.samples.stride = sizeof(float);

				// Cook the heightfield 
				physx::PxDefaultMemoryOutputStream memoryOutputStream;
				physx::PxCookingParams cookingParams(PhysicsWorld::mPhysics->getTolerancesScale());
				physx::PxCooking* cooking = PxCreateCooking(PX_PHYSICS_VERSION, *PhysicsWorld::mFoundation, cookingParams);
				if (!cooking->cookHeightField(heightFieldDesc, memoryOutputStream))
				{
					std::cout << "Error: failed to cook height field" << std::endl;
					return;
				}

				// Create the height field
				physx::PxDefaultMemoryInputData memoryInputData(memoryOutputStream.getData(), memoryOutputStream.getSize());
				physx::PxHeightField* heightField = PhysicsWorld::mPhysics->createHeightField(memoryInputData);
				physx::PxHeightFieldGeometry heightFieldGeom(heightField, physx::PxMeshGeometryFlags(), heightFieldShape->GetResolution(), heightFieldShape->GetResolution(), heightFieldShape->GetResolution());
				// Create the shape for it
				rigidBody->pShape = PhysicsWorld::mPhysics->createShape(heightFieldGeom, *PhysicsWorld::mMaterial);

				// Release the cooking object
				cooking->release();
			}
			if (rigidBody->IsStatic())
			{
				rigidBody->rigidBody = PhysicsWorld::mPhysics->createRigidStatic(transform);
				rigidBody->rigidBody->attachShape(*rigidBody->pShape);
			}
			else
			{
				//rigidBody->rigidBody = PhysicsWorld::mPhysics->createRigidDynamic(transform);
				physx::PxRigidDynamic* rigidActor = mPhysics->createRigidDynamic(transform);
				rigidActor->attachShape(*rigidBody->pShape);
				physx::PxRigidBodyExt::updateMassAndInertia(*rigidActor, rigidBody->m_Mass);
				rigidBody->rigidBody = rigidActor;
			}
			mActors.push_back((iRigidBody*)rigidBody);
			mScene->addActor(*rigidBody->rigidBody);
			mOriginalTransforms.emplace(rigidBody->rigidBody, transform);
			rigidBody->pShape->release();
		}
	}
	//	//else if (body->GetBodyType() == BodyType::SoftBody)
	//	//{
	//	//	SoftBody* rigidBody = SoftBody::Cast(body);

	//	//	if (std::find(m_SoftBodies.begin(), m_SoftBodies.end(), rigidBody) == m_SoftBodies.end())
	//	//	{
	//	//		m_SoftBodies.push_back(rigidBody);
	//	//	}
	//	//}
	//}

	void PhysicsWorld::RemoveBody(iCollisionBody* body)
	{
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
	}
	void PhysicsWorld::ResetWorld()
	{
		// Loop through actors
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
	}
};