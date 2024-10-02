#include "SnowParticleManager.h"
#include "AABBCollision.h"
#include "GameWorld.h"
#include "TracingSystem.h"

#pragma optimize("", off)

//TODO maybe inherit from RainParticleManager
SnowParticleManager::SnowParticleManager(int NumParticles, const glm::vec3& InVelocity)
{  
    m_NumParticles = NumParticles;
    m_lastUsedParticle = 0;
    m_centerPosition = glm::vec3(0,0,0);
    m_wind = glm::vec3(0.0f);
    m_initialParticleVelocity = InVelocity;
}

void SnowParticleManager::SetWind(const glm::vec3& wind)
{
    m_wind = wind;
}

void SnowParticleManager::SetInitialParticleVelocity(const glm::vec3& InVelocity)
{
     m_initialParticleVelocity = InVelocity;
}

void SnowParticleManager::SpawnParticles()
{
	for (unsigned int i = 0; i < m_NumParticles; ++i)
    {
       SnowParticle SnowParticle(m_initialParticleVelocity);
       m_particles.push_back(std::move(SnowParticle));
       m_particles[i].Initialize();
       m_particles[i].Position = FindSnowParticlePosition();
       m_particles[i].Life = 100.f;
       m_particles[i].Velocity = glm::vec3(0,0,-25.f);
    }
}

unsigned int SnowParticleManager::FirstUnusedParticle()
{
    for (unsigned int i = m_lastUsedParticle; i < m_NumParticles; ++i) {
        if (m_particles[i].Life <= 0.0f || m_particles[i].Position.z <= 0.0f){
            m_lastUsedParticle = i;
            return i;
        }
    }

    for (unsigned int i = 0; i < m_lastUsedParticle; ++i) {
        if (m_particles[i].Life <= 0.0f || m_particles[i].Position.z <= 0.0f){
            m_lastUsedParticle = i;
            return i;
        }
    }
    // override first particle if all others are alive
    m_lastUsedParticle = 0;
    return -1;

}

void SnowParticleManager::SetSnowIntensity(float InRainIntensity)
{
     m_snowIntensity = InRainIntensity;
}

void SnowParticleManager::RespawnParticle(SnowParticle& particle, const glm::vec3& position)
{
    particle.Position = position;
    particle.Color = glm::vec4(1.0f);
    particle.Life = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX/(10.f)));
}

void SnowParticleManager::Update(float DeltaTime)
{
	unsigned int nr_new_particles = 300 * m_snowIntensity;
    // add new particles
    for (unsigned int i = 0; i < nr_new_particles; ++i)
    {
        int unusedParticle = FirstUnusedParticle();
        if(unusedParticle >= 0)
        {
           RespawnParticle(m_particles[unusedParticle], FindSnowParticlePosition());
        }
    }
    // update all particles
    for (unsigned int i = 0; i < m_NumParticles; ++i)
    {
        SnowParticle &p = m_particles[i];
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
        }
    }  
}

void SnowParticleManager::Draw(Renderer& renderer, const glm::mat4& MVP, Shader* particleShader)
{
    for (SnowParticle& particle : m_particles)
    {
        particle.Draw(renderer, MVP, particleShader);
    } 
}

glm::vec3 SnowParticleManager::FindSnowParticlePosition()
{
    float x = -GameWorld::WorldExtent.x + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX/(GameWorld::WorldExtent.x * 2)));
    float y = -GameWorld::WorldExtent.y + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX/(GameWorld::WorldExtent.y * 2)));
    return glm::vec3((x + m_centerPosition.x), (y + m_centerPosition.y), 100.f);
}

bool SnowParticleManager::DoesParticleCollideWithWorld(SnowParticle& particle) const
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

void SnowParticleManager::SetCenterPosition(const glm::vec3& centerPosition)
{
    m_centerPosition = centerPosition;
}
