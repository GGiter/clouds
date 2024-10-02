#include "AABBCollision.h"

AABBCollision::AABBCollision(const glm::vec3& position, const glm::vec3& extent)
{
	m_position = position;
	m_extent = extent;
}