#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>

class Application
{
public:
	int Run();

	static bool bUseProcedural;
private:
	void Render();
	void GenerateTextures();
	static void WindowSizeCallback(GLFWwindow* window, int width, int height);
	static bool bNeedsReload;
	static int windowHeight;
	static int windowWidth;
	static int downscaleFactor;
	static bool bUseFSR;
	static bool bUseFXAA;
	static bool bUseUpscaleShader;
	static bool bUseDenoiseShader;
	static bool bUseMipMaps;
	static int WeatherTextureSize;
	static int WorleyTextureSize;
	static int PerlinWorleyTextureSize;
	static float WeatherScale;
	static float WorleyScale;
	static float PerlinWorleyScale;
	static float CloudType;

};
