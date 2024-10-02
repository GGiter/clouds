#include "RainParticleManager.h"
#include "AABBCollision.h"
#include "GameWorld.h"
#include "TracingSystem.h"
#pragma optimize("", off)
RainParticleManager::RainParticleManager(int NumParticles, const glm::vec3& InVelocity)
{  
    m_NumParticles = NumParticles;
    m_lastUsedParticle = 0;
    m_centerPosition = glm::vec3(0,0,0);
    m_wind = glm::vec3(0.0f);
    m_initialParticleVelocity = InVelocity;
}

void RainParticleManager::SetWind(const glm::vec3& wind)
{
    m_wind = wind;
}

void RainParticleManager::SetInitialParticleVelocity(const glm::vec3& InVelocity)
{
     m_initialParticleVelocity = InVelocity;
}

void RainParticleManager::SpawnParticles()
{
	for (unsigned int i = 0; i < m_NumParticles; ++i)
    {
       RainParticle RainParticle(m_initialParticleVelocity);
       m_particles.push_back(std::move(RainParticle));
       m_particles[i].Initialize();
       m_particles[i].Position = FindRainParticlePosition();
       m_particles[i].Life = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX/(10.f)));
       m_particles[i].Velocity = glm::vec3(0,0,-50.f);
    }

}

unsigned int RainParticleManager::FirstUnusedParticle()
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

void RainParticleManager::SetRainIntensity(float InRainIntensity)
{
     m_rainIntensity = InRainIntensity;
}

void RainParticleManager::RespawnParticle(RainParticle& particle, const glm::vec3& position)
{
    particle.Position = position;
    particle.Color = glm::vec4(1.0f);
    particle.Life = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX/(10.f)));
}

void RainParticleManager::Update(float DeltaTime)
{
	unsigned int nr_new_particles = 300 * m_rainIntensity;
    // add new particles
    for (unsigned int i = 0; i < nr_new_particles; ++i)
    {
        int unusedParticle = FirstUnusedParticle();
        if(unusedParticle >= 0)
        {
           RespawnParticle(m_particles[unusedParticle], FindRainParticlePosition());
        }
    }
    // update all particles
    for (unsigned int i = 0; i < m_NumParticles; ++i)
    {
        RainParticle &p = m_particles[i];
        p.Life -= DeltaTime;
        if(DoesParticleCollideWithWorld(p))
        {
            p.Life = 0.0f;
            p.Color.a = 0.0f;
        }
        if(p.Life > 0.0f)
        {
            p.Velocity += m_wind * DeltaTime;
            p.Position += p.Velocity * DeltaTime;
            p.Color.a -= DeltaTime * 0.2f;
            if(p.Color.a < 0.0f)
                p.Color.a = 0.0f;
        }
    }  
}

void RainParticleManager::Draw(Renderer& renderer, const glm::mat4& MVP, Shader* particleShader)
{
    for (RainParticle& particle : m_particles)
    {
        particle.Draw(renderer, MVP, particleShader);
    } 
}

glm::vec3 RainParticleManager::FindRainParticlePosition()
{
    float x = -GameWorld::WorldExtent.x + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX/(GameWorld::WorldExtent.x * 2.f)));
    float y = -GameWorld::WorldExtent.y + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX/(GameWorld::WorldExtent.y * 2.f)));
    return glm::vec3((x + m_centerPosition.x) * 25, (y + m_centerPosition.y) * 25, 100.f);
}

bool RainParticleManager::DoesParticleCollideWithWorld(RainParticle& particle) const
{
    AABBCollision particleAABB(glm::vec3(particle.Position), particle.GetScale());
    if(particle.Position.z <= 0.0f)
        return true;
    for(AABBCollision* AABB : GameWorld::AABBs)
    {
        if(TracingSystem::DoesAABBOverlap(&particleAABB, AABB))
        {
            //we assume that the rain will hit top part of the bounding box
            return true;
        }
    }
    return false;
}

void RainParticleManager::SetCenterPosition(const glm::vec3& centerPosition)
{
    m_centerPosition = centerPosition;
}
