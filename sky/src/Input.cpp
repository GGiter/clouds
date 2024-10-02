#include "Input.h"
#include "Window.h"

Input* Input::s_Instance = nullptr;

Input::Input(GLFWwindow* window)
	: m_lastScrollValue(0.0f)
{
	glfwSetScrollCallback(window, ScrollCallback);
}

bool Input::IsKeyPressedImpl(int keycode)
{
	auto state = glfwGetKey(Window::window, keycode);
	return state == GLFW_PRESS || state == GLFW_REPEAT;
}

bool Input::IsMouseButtonPressedImpl(int button)
{
	auto state = glfwGetMouseButton(Window::window, button);
	return state == GLFW_PRESS;
}

float Input::GetMouseXImpl()
{
	double xpos, ypos;
	glfwGetCursorPos(Window::window, &xpos, &ypos);

	return (float)xpos;
}

float Input::GetMouseYImpl()
{
	double xpos, ypos;
	glfwGetCursorPos(Window::window, &xpos, &ypos);

	return (float)ypos;
}

void Input::ScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
	Input::s_Instance->m_lastScrollValue = (float)yoffset;
}

float Input::GetLastScrollValueImpl()
{
	const float lastScrollValue = m_lastScrollValue;
	m_lastScrollValue = 0.0f;
	return lastScrollValue;
}
