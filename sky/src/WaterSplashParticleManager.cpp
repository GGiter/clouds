#include "WaterSplashParticleManager.h"
#include "AABBCollision.h"
#include "GameWorld.h"
#include "TracingSystem.h"

WaterSplashParticleManager::WaterSplashParticleManager(int NumParticles)
{  
    m_NumParticles = NumParticles;
    m_lastUsedParticle = 0;
    m_centerPosition = glm::vec3(0,0,0);
}

void WaterSplashParticleManager::SpawnParticles()
{
	for (unsigned int i = 0; i < m_NumParticles; ++i)
    {
       WaterSplashParticle WaterRippleParticle(glm::vec3(0.0));
       m_particles.push_back(std::move(WaterRippleParticle));
       m_particles[i].Initialize();
       m_particles[i].Position = FindWaterSplashPosition();
       m_particles[i].Life = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX/(10.f)));
    }

}

unsigned int WaterSplashParticleManager::FirstUnusedParticle()
{
    for (unsigned int i = m_lastUsedParticle; i < m_NumParticles; ++i) {
        if (m_particles[i].Life <= 0.0f){
            m_lastUsedParticle = i;
            return i;
        }
    }

    for (unsigned int i = 0; i < m_lastUsedParticle; ++i) {
        if (m_particles[i].Life <= 0.0f){
            m_lastUsedParticle = i;
            return i;
        }
    }
    // override first particle if all others are alive
    m_lastUsedParticle = 0;
    return -1;

}

void WaterSplashParticleManager::RespawnParticle(WaterSplashParticle& particle, const glm::vec3& position)
{
    particle.Position = position;
    particle.Color = glm::vec4(1.0f);
    particle.Life = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX/(10.f)));
}

void WaterSplashParticleManager::SetRainIntensity(float InRainIntensity)
{
    m_rainIntensity = InRainIntensity;
}

void WaterSplashParticleManager::Update(float DeltaTime)
{
	unsigned int nr_new_particles = 2 * m_rainIntensity;
    // add new particles
    for (unsigned int i = 0; i < nr_new_particles; ++i)
    {
        int unusedParticle = FirstUnusedParticle();
        if(unusedParticle >= 0)
        {
           RespawnParticle(m_particles[unusedParticle], FindWaterSplashPosition());
        }
    }
    // update all particles
    for (unsigned int i = 0; i < m_NumParticles; ++i)
    {
        Particle &p = m_particles[i];
        p.Life -= DeltaTime;
        if(p.Life > 0.0f)
        {
            p.Position += p.Velocity * DeltaTime;
            p.Color.a -= DeltaTime * 0.5f;
            if(p.Color.a < 0.0f)
                p.Color.a = 0.0f;
        }
    }  
}

void WaterSplashParticleManager::Draw(Renderer& renderer, const glm::mat4& MVP, Shader* particleShader, bool bShowDebugInfo)
{
    if (particleShader && bShowDebugInfo) {
        particleShader->StartTimer();
    }

    for (WaterSplashParticle& particle : m_particles)
    {
        particle.Draw(renderer, MVP, particleShader);
    }

    if (particleShader && bShowDebugInfo) {
        particleShader->StopTimer();
        double timeElapsed = particleShader->GetElapsedTime() / m_particles.size();

        renderer.CalculateParticleShaderTimes(particleShader->GetName(), timeElapsed);
    }
}

glm::vec3 WaterSplashParticleManager::FindWaterSplashPosition()
{
    float x = -GameWorld::WorldExtent.x + m_centerPosition.x + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX/(GameWorld::WorldExtent.x * 2.f)));
    float y = -GameWorld::WorldExtent.y + m_centerPosition.y + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX/(GameWorld::WorldExtent.y * 2.f)));
    AABBCollision RayAABB(glm::vec3(x,y, GameWorld::WorldExtent.z), glm::vec3(1.f, 1.f, GameWorld::WorldExtent.z));
    for(AABBCollision* AABB : GameWorld::AABBs)
    {
        if(TracingSystem::DoesAABBIntersect(AABB, &RayAABB))
        {
            //we assume that the rain will hit top part of the bounding box
            return glm::vec3(x, y, AABB->GetPosition().z + AABB->GetExtent().z);
        }
    }
    return glm::vec3(x, y, -9.f);
}

void WaterSplashParticleManager::SetCenterPosition(const glm::vec3& centerPosition)
{
    m_centerPosition = centerPosition;
}
