#pragma once
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include "Animation.h"
#include "BoneHierarchy.h"

#include <Interface/iRigidBody.h>
#include <Interface/ICharacterController.h>
#include "../cMeshObject.h"
#include "../Character.h"
#include "../BuildingType.h"
#include "../Inventory.h"
//#include "../Colonist.h"
using namespace physics;

struct GameObjectBoneData
{
    glm::vec3 Position;
    glm::vec3 Scale;
    glm::quat Rotation;
    glm::mat4 ModelMatrix;
};

class GameObject
{
public:
    GameObject() { id = nextId++; };
    GameObject(iRigidBody* rigidBody, cMeshObject* mesh) { this->rigidBody = rigidBody, this->mesh = mesh, id = nextId++; }
    ~GameObject() = default;
    iRigidBody* rigidBody;
    iCharacterController* characterController;
    cMeshObject* mesh;
    Character* animCharacter;
    //Colonist* colonist;
    Inventory* inventory;
    BuildingType buildingType;
    glm::vec3* position;
    int id;
    static int nextId;
    bool Enabled;
    bool isSelected;
    // Animation
    bool hasBones;
    std::vector<glm::mat4> BoneModelMatrices;
    std::vector<glm::mat4> GlobalTransformations;
    glm::mat4 BoneRotationMatrices[66];

    std::vector<GameObject*> childObjects;
};

