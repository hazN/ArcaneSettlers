#include "CharacterController.h"
#include "PhysicsWorld.h"
#include <Interface/CylinderShape.h>
#include <iostream>
namespace physics
{
	CharacterController::CharacterController(iShape* shape, const Vector3& position, const Quaternion& rotation)
	{
		// Create a capsule shape for the controller
		PxControllerDesc* desc;
		if (CylinderShape* cylinder = (CylinderShape*)shape)
		{
			PxCapsuleControllerDesc capsuleDesc;
			float radius = cylinder->GetHalfExtents().x;
			capsuleDesc.height = cylinder->GetHalfExtents().y * 2.0f - radius * 2.0f;
			capsuleDesc.radius = radius;
			desc = &capsuleDesc;
			desc->userData = (void*)capsuleDesc.userData;
		}
		else
		{
			std::cout << "Error: incorrect shape passed into CharacterController..." << std::endl;
			return;
		}
		// Create the desc for the controller
		desc->position = PxExtendedVec3(position.x, position.y, position.z);
		desc->upDirection = PxVec3(0, 1, 0);
		desc->slopeLimit = 0.707f;
		desc->invisibleWallHeight = 0.0f;
		desc->maxJumpHeight = 0.0f;
		desc->contactOffset = 0.01f;
		desc->stepOffset = 0.5f;
		desc->density = 10;
		desc->scaleCoeff = 0.9f;
		desc->material = PhysicsWorld::mMaterial;
		// Create controller
		mController = PhysicsWorld::mControllerManager->createController(*desc);
		SetRotation(rotation);
		mGravity = new Vector3(0.f, -9.81f, 0.f);
		mVelocity = new Vector3(0.f, 0.f, 0.f);
	}

	CharacterController::~CharacterController()
	{
		mController->release();
	}

	// Move the character with a direction
	void CharacterController::Move(const Vector3& direction)
	{
		mVelocity->x += mGravity->x;
		mVelocity->y += mGravity->y;
		mVelocity->z += mGravity->z;

		PxVec3 dir = PxVec3(direction.x, mVelocity->y + direction.y, direction.z);
		PxControllerFilters filters;
		PxControllerCollisionFlags collisionFlags = mController->move(dir, 0.0f, 1.0f / 60.0f, filters);
		// Check if character is grounded
		if (collisionFlags & physx::PxControllerCollisionFlag::eCOLLISION_DOWN)
		{
			mVelocity->y = 0.0f;
		}
	}

	// Forcibly set the position
	void CharacterController::SetPosition(const Vector3& position)
	{
		mController->setPosition(PxExtendedVec3(position.x, position.y, position.z));
	}
	// Get the characters position
	void CharacterController::GetPosition(Vector3& position) const
	{
		PxExtendedVec3 pxPosition = mController->getPosition();
		position.x = (float)pxPosition.x;
		position.y = (float)pxPosition.y;
		position.z = (float)pxPosition.z;
	}
	// Set the characters rotation
	void CharacterController::SetRotation(const Quaternion& rotation)
	{
		PxQuat pxRotation(rotation.x, rotation.y, rotation.z, rotation.w);
		PxTransform transform(mController->getActor()->getGlobalPose());
		transform.q = pxRotation;
		mController->getActor()->setGlobalPose(transform);
	}

	// Get characters rotation
	void CharacterController::GetRotation(Quaternion& rotation) const
	{
		PxQuat pxRotation = mController->getActor()->getGlobalPose().q;
		rotation.x = pxRotation.x;
		rotation.y = pxRotation.y;
		rotation.z = pxRotation.z;
		rotation.w = pxRotation.w;
	}

	void physics::CharacterController::SetGravity(const Vector3& gravity)
	{
		*mGravity = gravity;
	}
}