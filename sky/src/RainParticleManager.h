#pragma once
#include "RainParticle.h"
#include "ParticleManager.h"
#include <vector>

class RainParticleManager : public ParticleManager
{
public:
	RainParticleManager(int NumParticles, const glm::vec3& InVelocity);
	void SpawnParticles();
	unsigned int FirstUnusedParticle();
	void SetRainIntensity(float InRainIntensity);
	void RespawnParticle(RainParticle& particle,const glm::vec3& offset);
	virtual void Update(float DeltaTime);
	virtual void Draw(Renderer& renderer, const glm::mat4& MV, Shader* particleShader, bool bShowDebugInfo);
	glm::vec3 FindRainParticlePosition();
	bool DoesParticleCollideWithWorld(RainParticle& particle) const;
	void SetCenterPosition(const glm::vec3& centerPosition);
	void SetWind(const glm::vec3& wind);
	void SetInitialParticleVelocity(const glm::vec3& InVelocity);

protected:
	unsigned int m_NumParticles;
	std::vector<RainParticle> m_particles;
	unsigned int m_lastUsedParticle;
	float m_rainIntensity;
	glm::vec3 m_centerPosition;
	glm::vec3 m_initialParticleVelocity;
	glm::vec3 m_wind;
};

