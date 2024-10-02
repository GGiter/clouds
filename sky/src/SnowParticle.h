#pragma once
#include "Particle.h"
class SnowParticle
	: public Particle
{
public:
	SnowParticle(const glm::vec3& InVelocity) : Particle(InVelocity)  {};
	virtual glm::vec3 GetScale() const;
};

