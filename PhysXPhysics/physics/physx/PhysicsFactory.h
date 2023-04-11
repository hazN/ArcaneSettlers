#ifndef PhysicsFactory_H
#define PhysicsFactory_H
// Interface
#include <Interface/iPhysicsFactory.h>
#include <Interface/iRigidBody.h>

namespace physics
{
	class PhysicsFactory : public iPhysicsFactory
	{
	public:
		PhysicsFactory(void);
		virtual ~PhysicsFactory(void);

		virtual iPhysicsWorld* CreateWorld(void);

		virtual iCharacterController* CreateCharacterController(iShape* shape, const Vector3& position, const Quaternion& orientation);
		virtual iRigidBody* CreateRigidBody(const RigidBodyDesc& desc, iShape* shape);
		virtual iSoftBody* CreateSoftBody(const SoftBodyDesc& desc);
	};
};

#endif