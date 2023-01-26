#pragma once

#include <glm/glm.hpp>

class PerspectiveCamera
{
public:
	PerspectiveCamera(float FOV, float Height, float Width);

	const glm::vec3& GetPosition() const { return m_Position; }
	void SetPosition(const glm::vec3& position) { m_Position = position; RecalculateViewMatrix(); }

	float GetRotationX() const { return m_RotationX; }
	void SetRotationX(float rotation) { m_RotationX = rotation; RecalculateViewMatrix(); }

	float GetRotationY() const { return m_RotationY; }
    void SetRotationY(float rotation) { m_RotationY = rotation; RecalculateViewMatrix(); }

    float GetRotationZ() const { return m_RotationZ; }
    void SetRotationZ(float rotation) { m_RotationZ = rotation; RecalculateViewMatrix(); }

	const glm::mat4& GetProjectionMatrix() const { return m_ProjectionMatrix; }
	const glm::mat4& GetViewMatrix() const { return m_ViewMatrix; }
	const glm::mat4& GetViewProjectionMatrix() const { return m_ViewProjectionMatrix; }

private:
	void RecalculateViewMatrix();

	glm::mat4 m_ProjectionMatrix;
	glm::mat4 m_ViewMatrix;
	glm::mat4 m_ViewProjectionMatrix;

	glm::vec3 m_Position;
    float m_RotationX, m_RotationY, m_RotationZ;
};