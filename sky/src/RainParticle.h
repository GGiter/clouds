#pragma once
#include "Particle.h"
class RainParticle
	: public Particle
{
public:
	RainParticle(const glm::vec3& InVelocity) : Particle(InVelocity) {};
	virtual glm::vec3 GetScale() const;
};

