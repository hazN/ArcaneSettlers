#pragma once
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include "Animation.h"
#include "BoneHierarchy.h"

#include <Interface/iRigidBody.h>
#include "../cMeshObject.h"
#include "../Character.h"
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
    cMeshObject* mesh;
    Character* animCharacter;

    int id;
    static int nextId;
    bool Enabled;
    bool useModelMatrix;
    glm::mat4 modelMatrix;

};

int GameObject::nextId = 0;
