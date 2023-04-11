#include "PhysicsFactory.h"

#include "PhysicsWorld.h"

#include "RigidBody.h"
#include "SoftBody.h"
#include "CharacterController.h"
namespace physics
{
	PhysicsFactory::PhysicsFactory(void)
		: iPhysicsFactory()
	{
		printf("Created PhysX Physics Factory!\n");
	}

	PhysicsFactory::~PhysicsFactory(void)
	{
	}

	iPhysicsWorld* PhysicsFactory::CreateWorld(void)
	{
		PhysicsWorld* AWholeNewWorld = new PhysicsWorld();

		return AWholeNewWorld;
	}

	iCharacterController* PhysicsFactory::CreateCharacterController(iShape* shape, const Vector3& position, const Quaternion& orientation)
	{
		return new CharacterController(shape, position, orientation);
	}

	iRigidBody* PhysicsFactory::CreateRigidBody(const RigidBodyDesc& desc, iShape* shape)
	{
		return new RigidBody(desc, shape);
	}

	iSoftBody* PhysicsFactory::CreateSoftBody(const SoftBodyDesc& desc)
	{
		return new SoftBody(desc);
	}
};