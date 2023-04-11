#pragma once

#include "Math.h"

namespace physics
{
    class iCharacterController
    {
    public:
        virtual ~iCharacterController() {}

        virtual void Move(const Vector3& displacement) = 0;
        virtual void SetPosition(const Vector3& position) = 0;
        virtual void GetPosition(Vector3& position) const = 0;

        virtual void SetRotation(const Quaternion& rotation) = 0;
        virtual void GetRotation(Quaternion& rotation) const = 0;

        virtual void SetGravity(const Vector3& gravity) = 0;

    protected:
        iCharacterController() {}

    private:
        iCharacterController(const iCharacterController&) {}
        iCharacterController& operator=(const iCharacterController&) {
            return *this;
        }
    };
}
