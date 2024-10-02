#pragma once
#include "glm/glm.hpp"
class AABBCollision
{
public:
	AABBCollision(const glm::vec3& position, const glm::vec3& extent);
	AABBCollision() {}
	void SetPosition(const glm::vec3& position) { m_position = position; }
	glm::vec3 GetPosition() const { return m_position; }
	glm::vec3 GetExtent() const { return m_extent; }
private:
	glm::vec3 m_position;
	glm::vec3 m_extent;
};

