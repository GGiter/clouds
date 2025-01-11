
#include "Application.h"

int main()
{
	Application application;
	return application.Run();;
}

/*
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

// Callback for resizing the window
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

// Function to initialize GLFW and GLAD
GLFWwindow* initOpenGL(int width, int height, const char* title)
{
	// Initialize GLFW
	if(!glfwInit())
	{
		std::cerr << "Failed to initialize GLFW!" << std::endl;
		return nullptr;
	}

	// Configure GLFW: OpenGL 3.3 Core Profile
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Create a window
	GLFWwindow* window = glfwCreateWindow(width, height, title, nullptr, nullptr);
	if(!window)
	{
		std::cerr << "Failed to create GLFW window!" << std::endl;
		glfwTerminate();
		return nullptr;
	}

	// Make the OpenGL context current
	glfwMakeContextCurrent(window);

	// Load OpenGL functions using GLAD
	if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cerr << "Failed to initialize GLAD!" << std::endl;
		glfwDestroyWindow(window);
		glfwTerminate();
		return nullptr;
	}

	// Set the viewport
	glViewport(0, 0, width, height);

	// Set the framebuffer resize callback
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	return window;
}

int main()
{
	// Initialize OpenGL
	GLFWwindow* window = initOpenGL(800, 600, "Yellow Screen with GLFW and GLAD");
	if(!window) return -1;

	// Render loop
	while(!glfwWindowShouldClose(window))
	{
		// Clear the screen with yellow color
		glClearColor(1.0f, 1.0f, 0.0f, 1.0f); // Yellow
		glClear(GL_COLOR_BUFFER_BIT);

		// Swap buffers and poll events
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// Clean up and exit
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
*/
