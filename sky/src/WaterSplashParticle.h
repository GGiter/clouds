#pragma once
#include "Particle.h"
class WaterSplashParticle
	: public Particle
{
public:
	WaterSplashParticle(const glm::vec3& InVelocity) : Particle(InVelocity) {};
};

