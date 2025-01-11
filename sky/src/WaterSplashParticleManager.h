#pragma once
#include "ParticleManager.h"
#include "WaterSplashParticle.h"
#include <vector>

class WaterSplashParticleManager : public ParticleManager
{
public:
	WaterSplashParticleManager(int NumParticles);
	void SpawnParticles();
	unsigned int FirstUnusedParticle();
	void RespawnParticle(WaterSplashParticle& particle,const glm::vec3& offset);
	void SetRainIntensity(float InRainIntensity);
	virtual void Update(float DeltaTime);
	virtual void Draw(Renderer& renderer, const glm::mat4& MV, Shader* particleShader, bool bShowDebugInfo);
	glm::vec3 FindWaterSplashPosition();
	void SetCenterPosition(const glm::vec3& centerPosition);

protected:
	unsigned int m_NumParticles;
	std::vector<WaterSplashParticle> m_particles;
	unsigned int m_lastUsedParticle;
	float m_rainIntensity;
	glm::vec3 m_centerPosition;
};

