#pragma once
#include <GLAD/glad.h>
#include <GLFW/glfw3.h>
#include <ffx_fsr2.h>
#include <memory>
#include <gl/ffx_fsr2_gl.h>
#include "Upscaler.h"

class PerspectiveCamera;
class Scene;
class CubemapFramebuffer;
class VertexArray;
class IndexBuffer;

enum UpscalerVersion : int
{
	None = 0,
	Simple = 1,
	One = 2,
	Two = 3,
	Count = 4 
};

class Application
{
public:
	int Run();

	static bool bUseProcedural;
	static bool LoadOnlyDiffuseTextures;
	static int MaxNumberOfMeshes;
	static int windowHeight;
	static int windowWidth;
	static int downscaleFactor;
private:
	void Render();
	void GenerateTextures();
	void UpdatePreviousCubemapPass(Scene& scene, CubemapFramebuffer& cubemapPass, CubemapFramebuffer& previousCubemapPass, const glm::vec3& initialCameraPosition, Shader& passThroughCubemapShader, VertexArray& vaCube, IndexBuffer& ib, bool bShowDebugInfo);
	void UpdateCubemapPass(Scene& scene, CubemapFramebuffer& cubemapPass, const glm::vec3& initialCameraPosition, Shader& shader, VertexArray& vaCube, IndexBuffer& ib, bool bShowDebugInfo, Shader& meshShader, std::unordered_map<std::string, Shader*>& shaderMap);
	void SetCameraViewForFace(PerspectiveCamera& camera, unsigned int face);
	void ConfigureMeshShader(Scene& scene, const PerspectiveCamera& cubemapCamera, Shader& meshShader, std::unordered_map<std::string, Shader*>& shaderMap);
        void UpdateInterpolatedCubemap(
            Scene &scene, CubemapFramebuffer &cubemapPass,
            CubemapFramebuffer &previousCubemapPass,
            CubemapFramebuffer &interpolatedCubemapPass,
            const glm::vec3 &initialCameraPosition, float timePassed,
            float CubemapUpdateRate, Shader &lerpCubemapShader,
            VertexArray &vaCube, IndexBuffer &ib, bool bShowDebugInfo);
	static void WindowSizeCallback(GLFWwindow* window, int width, int height);
	static bool bNeedsReload;
	static UpscalerVersion upscalerVersion;
	static bool bUseFXAA;
	static bool bUseDenoiseShader;
	static bool bUseMipMaps;
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
	static bool bRenderRainCone;
	static bool bUseRipples;
	static int NumberOfBolts;
	static float ThunderstormIntensity;
	static bool bShowDebugInfo;
	static bool bRenderPuddles;
	static bool bRenderWetSurfaces;
	static float ShadowIntensity;
	static float MaxSnowDisplacement;
	static std::string scenePath;
	std::unique_ptr<Upscaler> upscaler;
};
