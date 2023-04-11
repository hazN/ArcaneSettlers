#pragma once

#include <Interface/iCharacterController.h>
#include <Interface/iShape.h>
#include <Interface/Math.h>
#include <physx/PxPhysicsAPI.h>

using namespace physx;

namespace physics
{
    class CharacterController : public iCharacterController
    {
    public:
        CharacterController(iShape* shape, const Vector3& position, const Quaternion& rotation);
        virtual ~CharacterController();

        virtual void Move(const Vector3& displacement) override;
        virtual void SetPosition(const Vector3& position) override;
        virtual void GetPosition(Vector3& position) const override;

        virtual void SetRotation(const Quaternion& rotation) override;
        virtual void GetRotation(Quaternion& rotation) const override;

        virtual void SetGravity(const Vector3& gravity) override;

    private:
        PxController* mController;
    };
}
