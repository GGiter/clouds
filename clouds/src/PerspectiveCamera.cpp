#include "PerspectiveCamera.h"
#include <glm/gtc/matrix_transform.hpp>

PerspectiveCamera::PerspectiveCamera(float FOV, float Height, float Width)
	: m_ProjectionMatrix(glm::perspective( 90.f ,  Height / Width , 1.f , 10.0f )), m_RotationX(0.0f), m_RotationY(0.0f), m_RotationZ(0.0f), m_Position(0.f, 0.f, 0.f)
{
	m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
}

void PerspectiveCamera::RecalculateViewMatrix()
{
  glm::mat4 transform =
      glm::translate(glm::mat4(1.0f), m_Position) *
      glm::rotate(glm::mat4(1.0f), m_RotationX, glm::vec3(0, 1, 0));

	m_ViewMatrix = transform;
	m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
}
