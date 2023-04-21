#pragma once
#include "ParticleSystem.h"

ParticleSystem::ParticleSystem(const char* meshName)
{
    mMeshName = meshName;
}

ParticleSystem::~ParticleSystem()
{
    for (Particle* particle : mParticles)
    {
        delete particle->mMesh;
        delete particle;
    }
}

void ParticleSystem::Update(float dt)
{
    // Loop through particles
    for (Particle* particle : mParticles)
    {
        if (particle->isAlive)
        {
            particle->mElapsedTime += dt;
            // Check if the particle has reached its lifetime
            if (particle->mElapsedTime >= particle->mLifetime)
            {
                particle->isAlive = false;
                particle->mMesh->bIsVisible = false;
            }
            // Otherwise update the particles position
            else
            {
                particle->mVelocity += particle->mAcceleration * dt;
                particle->mMesh->position += particle->mVelocity * dt;
            }
        }
    }
}

void ParticleSystem::GenerateParticles(size_t numParticles, const glm::vec3& position, float minLifetime, float maxLifetime, float scale, glm::vec4 colour)
{
    std::vector<Particle*> deadParticles = GetDeadParticles();

    // Loop through and generate the particles
    for (size_t i = 0; i < numParticles; i++)
    {
        Particle* pParticle = nullptr;
        // First reuse any dead particles we have
        if (i < deadParticles.size())
        {
            pParticle = deadParticles[i];
        }
        // Otherwise create a new one
        else
        {
            pParticle = new Particle;
            pParticle->mMesh = new cMeshObject();
            pParticle->mMesh->meshName = mMeshName;
            mParticles.push_back(pParticle);
        }
        pParticle->mMesh->position = position;
        pParticle->mMesh->bUse_RGBA_colour = true;
        pParticle->mMesh->RGBA_colour = colour;
        pParticle->mMesh->SetUniformScale(scale);
        pParticle->mMesh->isWireframe = false;
        pParticle->mMesh->bDoNotLight = true;
        pParticle->mMesh->bIsVisible = true;
        pParticle->mVelocity = glm::vec3(((float)rand() / (float)RAND_MAX) * 2.0f - 1.0f, 0.0f, ((float)rand() / (float)RAND_MAX) * 2.0f - 1.0f);
        pParticle->mAcceleration = glm::vec3(0.f, ((float)rand() / (float)RAND_MAX) * (7.81f - 5.81f) + 5.81f, 0.f);
        pParticle->mLifetime = minLifetime + (float)(rand()) / ((float)(RAND_MAX / (maxLifetime - minLifetime)));
        pParticle->mElapsedTime = 0.0f;
        pParticle->isAlive = true;
    }
}

std::vector<Particle*> ParticleSystem::GetAliveParticles()
{
    std::vector<Particle*> pParticles;
    // Get all the alive particles
    for (Particle* particle : mParticles)
    {
        if (particle->isAlive)
            pParticles.push_back(particle);
    }
    return pParticles;
}

std::vector<Particle*> ParticleSystem::GetDeadParticles()
{
    std::vector<Particle*> pParticles;
    // Get all the dead particles
    for (Particle* particle : mParticles)
    {
        if (!particle->isAlive)
            pParticles.push_back(particle);
    }
    return pParticles;
}