#pragma once

#include "cMeshObject.h"
#include <vector>
#include <glm/glm.hpp>

struct Particle {
	cMeshObject* mMesh;
	glm::vec3 mVelocity;
	glm::vec3 mAcceleration;
	float mLifetime;
	float mElapsedTime;
	bool isAlive;
};

class ParticleSystem {
public:
	ParticleSystem(const char* meshName);
	~ParticleSystem();
	void Update(float dt);
	void GenerateParticles(size_t numParticles, const glm::vec3& position, float minLifetime, float maxLifetime, float scale, glm::vec4 colour);
	std::vector<Particle*> GetAliveParticles();
	std::vector<Particle*> GetDeadParticles();
private:
	std::vector<Particle*> mParticles;
	std::string mMeshName;
};