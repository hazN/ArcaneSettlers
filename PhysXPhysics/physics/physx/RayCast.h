#pragma once

#include <Interface/iRayCast.h>
#include <physx/PxPhysicsAPI.h>

using namespace physx;
namespace physics
{
    class RayCast : public iRayCast
    {
    public:
        RayCast();

        bool doRayCast(const glm::vec3& origin, const glm::vec3& direction, float maxDistance, RayCastHit& hit) override;

    private:
    };
}