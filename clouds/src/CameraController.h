#pragma once
#include "glm/glm.hpp"

class CameraController
{
public:
	CameraController();
	void update();

	float m_cameraRotationSpeed;
	float m_cameraZoomSpeed;
	float m_cameraMoveSpeed;
	glm::vec3 m_translation;
    float m_rotationX;
    float m_rotationY;
    float m_rotationZ;
};