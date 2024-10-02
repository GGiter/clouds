#pragma once
#include <GLAD/glad.h>
#include <GLFW/glfw3.h>
#include <ffx_fsr2.h>
#include <memory>
#include <gl/ffx_fsr2_gl.h>
#include "Upscaler.h"

enum UpscalerVersion : int
{
	None = 0,
	Simple = 1,
	One = 2,
	Two = 3,
	Count = 4 
};

enum ViewLayerType : int
{
	VLT_Color = 0,
	VLT_Depth = 1,
	VLT_MotionVectors = 2,
	VLT_Count = 3
};

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
	static UpscalerVersion upscalerVersion;
	static ViewLayerType viewLayerType;
	static bool bUseFXAA;
	static bool bUseDenoiseShader;
	static bool bUseMipMaps;
	static bool bUseRain;
	static bool bUseSnow;
	static bool bUseRainbow;
	static bool bRenderMeshes;
	static int WeatherTextureSize;
	static int WorleyTextureSize;
	static int PerlinWorleyTextureSize;
	static float WeatherScale;
	static float WorleyScale;
	static float PerlinWorleyScale;
	static float CloudType;
	static float RainIntensity;
	static float SnowIntensity;
	static bool bRenderSplashes;
	static bool bRenderRipples;
	static bool bRenderPuddles;
	static bool bRenderRainCone;
	static bool bUseSunrays;
	static int NumberOfBolts;
	static float ThunderstormIntensity;
	static bool bShowDebugInfo;
	std::unique_ptr<Upscaler> upscaler;
};
