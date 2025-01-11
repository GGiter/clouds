#pragma once
#include "SnowParticle.h"
#include "ParticleManager.h"
#include <vector>

class SnowParticleManager : public ParticleManager
{
public:
	SnowParticleManager(int NumParticles, const glm::vec3& InVelocity);
	void SpawnParticles();
	unsigned int FirstUnusedParticle();
	void SetSnowIntensity(float InRainIntensity);
	void RespawnParticle(SnowParticle& particle,const glm::vec3& offset, float RandomRotationInY);
	virtual void Update(float DeltaTime);
	virtual void Draw(Renderer& renderer, const glm::mat4& MV, Shader* particleShader, bool bShowDebugInfo);
	glm::vec3 FindSnowParticlePosition();
	float GenerateRandomRotationInY();
	bool DoesParticleCollideWithWorld(SnowParticle& particle) const;
	void SetCenterPosition(const glm::vec3& centerPosition);
	void SetWind(const glm::vec3& wind);
	void SetInitialParticleVelocity(const glm::vec3& InVelocity);

protected:
	unsigned int m_NumParticles;
	std::vector<SnowParticle> m_particles;
	unsigned int m_lastUsedParticle;
	float m_snowIntensity;
	glm::vec3 m_centerPosition;
	glm::vec3 m_initialParticleVelocity;
	glm::vec3 m_wind;
};

