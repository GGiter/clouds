#pragma once

#include <GLFW/glfw3.h>

class Input
{
public:
	Input(GLFWwindow* window);
	inline static bool IsKeyPressed(int keycode) { return s_Instance->IsKeyPressedImpl(keycode); }
	inline static bool IsMouseButtonPressed(int button) { return s_Instance->IsMouseButtonPressedImpl(button); }
	inline static float GetMouseX() { return s_Instance->GetMouseXImpl(); }
	inline static float GetMouseY() { return s_Instance->GetMouseYImpl(); }
	inline static float GetLastScrollValue() { return s_Instance->GetLastScrollValueImpl(); }
	static Input* s_Instance;
protected:
	static void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
	virtual float GetLastScrollValueImpl();
	virtual bool IsKeyPressedImpl(int keycode);
	virtual bool IsMouseButtonPressedImpl(int button);
	virtual float GetMouseXImpl();
	virtual float GetMouseYImpl();
	float m_lastScrollValue;
};