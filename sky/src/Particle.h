#pragma once
#include "Util.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/string_cast.hpp"
#include "Renderer.h"
#include <memory>

class Particle
{
public:
	glm::vec3 Position;
	glm::vec3 Velocity;
	glm::vec4 Color;
	float RotationY;
	glm::mat4 transform;
	float Life;
	bool bIs2D;

	Particle(const glm::vec3& InVelocity);
	void Initialize();
	void Draw(Renderer& renderer, const glm::mat4& MVP, Shader* particleShader);
	virtual glm::vec3 GetScale() const;
	std::unique_ptr<VertexArray> vaParticle;
	std::unique_ptr<VertexBuffer> vbParticle;
    std::unique_ptr<IndexBuffer> ibParticle;
};

