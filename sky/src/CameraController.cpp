#include "CameraController.h"
#include "Input.h"
//#include "Application.h"
#include <iostream>


CameraController::CameraController()
    : m_cameraRotationSpeed(0.01f), m_cameraZoomSpeed(0.01f),
      m_cameraMoveSpeed(0.01f), m_translation(glm::vec3(0.0f)),
      m_rotationX(0.0f), m_rotationY(0.0f), m_rotationZ(0.0f) {
}

void CameraController::update()
{
	if (Input::IsKeyPressed(GLFW_KEY_LEFT))
		m_translation.x -= m_cameraMoveSpeed;

	if (Input::IsKeyPressed(GLFW_KEY_RIGHT))
		m_translation.x += m_cameraMoveSpeed;

	if (Input::IsKeyPressed(GLFW_KEY_DOWN))
		m_translation.y -= m_cameraMoveSpeed;

	if (Input::IsKeyPressed(GLFW_KEY_UP))
		m_translation.y += m_cameraMoveSpeed;

	m_translation.z += Input::GetLastScrollValue() * m_cameraZoomSpeed;

	//if(!Application::bUseProcedural)
	{
		//if (m_translation.z < -4.0f) {
		//	 m_translation.z = -4.f;		
		//}
	}


	if (Input::IsKeyPressed(GLFW_KEY_D))
        m_rotationY -= m_cameraRotationSpeed;

	if (Input::IsKeyPressed(GLFW_KEY_A))
        m_rotationY += m_cameraRotationSpeed;

	if(Input::IsKeyPressed(GLFW_KEY_Q))
		m_rotationX += m_cameraRotationSpeed;

	if(Input::IsKeyPressed(GLFW_KEY_E))
		m_rotationX -= m_cameraRotationSpeed;

	if(Input::IsKeyPressed(GLFW_KEY_Z))
		m_rotationZ += m_cameraRotationSpeed;

	if(Input::IsKeyPressed(GLFW_KEY_C))
		m_rotationZ -= m_cameraRotationSpeed;
}
