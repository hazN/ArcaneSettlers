#pragma once

#include "Math.h"
namespace physics {
    class iRayCast
    {
    public:
        virtual ~iRayCast() {}

        struct RayCastHit
        {
            Vector3 position;
            Vector3 normal;
            float distance;
            int userData;
        };

        virtual bool doRayCast(const Vector3& origin, const Vector3& direction, float maxDistance, RayCastHit& hit) = 0;
    };
}