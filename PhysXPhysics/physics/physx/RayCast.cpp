#include "RayCast.h"
#include "PhysicsWorld.h"
#include <Interface/iShape.h>
namespace physics
{
    RayCast::RayCast()
    {
    }

    bool RayCast::doRayCast(const glm::vec3& origin, const glm::vec3& direction, float maxDistance, RayCastHit& hit)
    {
        PxRaycastBuffer hitInfo;
        PxVec3 rayOrigin(origin.x, origin.y, origin.z);
        PxVec3 rayDir(direction.x, direction.y, direction.z);

        bool rayCast = PhysicsWorld::mScene->raycast(rayOrigin, rayDir, maxDistance, hitInfo);

        if (rayCast)
        {
            hit.position = Vector3(hitInfo.block.position.x, hitInfo.block.position.y, hitInfo.block.position.z);
            hit.normal = Vector3(hitInfo.block.normal.x, hitInfo.block.normal.y, hitInfo.block.normal.z);
            hit.distance = hitInfo.block.distance;

            // Get the userData from the pxShape and store it in the hit object
            if (hitInfo.block.shape)
            {
                PxShape* hitShape = hitInfo.block.shape;
                hit.userData = (int)hitShape->userData;
            }
            else
                hit.userData = -1; 
        }
        return rayCast;
    }

}