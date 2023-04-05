#pragma once
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <glm/gtc/quaternion.hpp>
#include <physx/PxPhysicsAPI.h>
#include "PhysicsWorld.h"
#include <Interface/iRigidBody.h>
#include <Interface/RigidBodyDesc.h>
#include <Interface/iShape.h>

namespace physics
{
	class CollisionHandler;
	class RigidBody : public iRigidBody
	{
	public:
		RigidBody(const RigidBodyDesc& desc, iShape* shape);
		~RigidBody();

		static RigidBody* Cast(iCollisionBody* body);

		iShape* GetShape(void);

		bool IsStatic(void) const;

		//void setWorld(PhysicsWorld* world);
		//virtual void GetWorldTransform(glm::mat4& transformOut) override;

		virtual void GetPosition(glm::vec3& positionOut) override;
		virtual void SetPosition(const glm::vec3& positionIn) override;
		virtual glm::vec3 GetGLMPosition() override;

		virtual void GetRotation(glm::quat& orientationOut) override;
		virtual void SetRotation(const glm::quat& orientationIn) override;

		virtual void ApplyForce(const glm::vec3& force) override;
		virtual void ApplyForceAtPoint(const glm::vec3& force, const glm::vec3& relativePoint) override;

		virtual void ApplyImpulse(const glm::vec3& impulse) override;
		virtual void ApplyImpulseAtPoint(const glm::vec3& impulse, const glm::vec3& relativePoint) override;

		virtual void ApplyTorque(const glm::vec3& torque) override;
		virtual void ApplyTorqueImpulse(const glm::vec3& torqueImpulse) override;
		physx::PxRigidActor* rigidBody = nullptr;
		physx::PxShape* pShape = nullptr;
		Vector3 originalPosition;
		Quaternion originalRotation;
		float m_Mass;
	protected:

	private:
		float m_InvMass;
		float m_Friction;
		float m_Restitution;
		float m_LinearDamping;
		float m_AngularDamping;
		bool m_IsStatic;

		Vector3 m_Position;
		Vector3 m_PreviousPosition;
		Vector3 m_LinearVelocity;
		Vector3 m_LinearAcceleration;

		Vector3 m_Force;
		Vector3 m_Torque;
		Vector3 m_Gravity;

		Quaternion m_Rotation;
		Vector3 m_AngularVelocity;
		Vector3 m_AngularAcceleration;

		iShape* m_Shape;

		Vector3* m_RenderPosition;
		RigidBody(const RigidBody& other) { ; }
		RigidBody& operator=(const RigidBody& other) { return *this; }
	};
};