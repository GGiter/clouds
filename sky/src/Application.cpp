#include "Application.h"
#include "Vertex.h"
#include "Renderer.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "VertexArray.h"
#include "VertexBufferLayout.h"
#include "Shader.h"
#include "Texture2D.h"
#include "Texture3D.h"
#include "Framebuffer.h"
#include "PerspectiveCamera.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/string_cast.hpp"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include "Window.h"
#include "Input.h"
#include "OpenVDBReader.h"
#include "CameraController.h"
#include "NoiseTextureGenerator.h"
#include "TextureSaver.h"
#include <iostream>
#include "FreeImage.h"
#include <filesystem>
#include "SimpleUpscaler.h"
#include "FSR1Upscaler.h"
#include "FSR2Upscaler.h"
#include "TextureCounter.h"
#include "Model.h"
#include "WaterSplashParticle.h"
#include "WaterSplashParticleManager.h"
#include "RainParticle.h"
#include "RainParticleManager.h"
#include "SnowParticle.h"
#include "SnowParticleManager.h"
#include "GameWorld.h"
#include "Cubemap.h"
#include "Scene.h"
#include <thread>
#include <chrono>
#pragma optimize("", off)
#define ENFORCE_SLOW_FRAMERATE_HACK 0
/* Configuration properties */
bool Application::bNeedsReload = false;
int Application::windowWidth = 1920;
int Application::windowHeight = 1080;
int Application::downscaleFactor = 4;
bool Application::bUseFXAA = true;
UpscalerVersion Application::upscalerVersion = UpscalerVersion::Two;
ViewLayerType Application::viewLayerType = ViewLayerType::VLT_Color;
bool Application::bUseProcedural = true;
bool Application::bUseDenoiseShader = false;
bool Application::bUseMipMaps = true;
bool Application::bRenderMeshes = true;
bool Application::bUseRain = false;
bool Application::bUseSnow = false;
bool Application::bUseRainbow = false;
int Application::WeatherTextureSize = 256;
int Application::WorleyTextureSize = 64;
int Application::PerlinWorleyTextureSize = 256;
float Application::WeatherScale = 0.00004f; //0.05f old version;
float Application::WorleyScale = 0.0003f;
float Application::PerlinWorleyScale = 0.0003f; //0.3f old version;
float Application::CloudType = 0.5f;
float Application::RainIntensity = 0.0f;
float Application::SnowIntensity = 0.0f;
bool Application::bRenderSplashes = true;
bool Application::bRenderRipples = true;
bool Application::bRenderPuddles = true;
bool Application::bRenderRainCone = false;
bool Application::bUseSunrays = false;
int Application::NumberOfBolts = 3;
float Application::ThunderstormIntensity = 0.0f;
bool Application::bShowDebugInfo = false;

/* Constants */
constexpr const char* weatherTextureName = "perlin.bmp";
constexpr const char* worleyTextureName = "worley.bmp";
constexpr const char* perlinWorleyTextureName = "perlinWorley.bmp";
constexpr const std::string debugOutputFileName = "debugOutput.txt";
const std::string fileName = "\\clouds.png";

/* Callback called when window resizes */
void Application::WindowSizeCallback(GLFWwindow* window, int width, int height)
{
    windowWidth = width;
    windowHeight = height;
	bNeedsReload = true;
}

int Application::Run()
{
    /* Initialize the library */
    if (!glfwInit())
        return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    /* Create a windowed mode window and its OpenGL context */
	glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_FALSE);
    Window::window = glfwCreateWindow(windowWidth, windowHeight, "Clouds", NULL, NULL);
    if (!Window::window)
    {
        glfwTerminate();
        return -1;
    }

    glfwSetWindowSizeCallback(Window::window, WindowSizeCallback);

    /*Initialize input callbacks*/
    Input::s_Instance = new Input(Window::window);

    /* Make the window's context current */
    glfwMakeContextCurrent(Window::window);

    glfwSwapInterval(1);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

    std::cout << glGetString(GL_VERSION) << std::endl;
    Render();

    glfwTerminate();
    return 0;
}

/* Main rendering method */
void Application::Render()
{
	uint32_t renderWidth = windowWidth / downscaleFactor;
	uint32_t renderHeight = windowHeight / downscaleFactor;

	if(upscalerVersion == UpscalerVersion::Simple)
	{
		upscaler = std::make_unique<SimpleUpscaler>();
	}
	else if(upscalerVersion == UpscalerVersion::One)
	{
		upscaler = std::make_unique<FSR1Upscaler>();
	}
	else if(upscalerVersion == UpscalerVersion::Two)
	{
		upscaler = std::make_unique<FSR2Upscaler>();
	}

	if(upscaler)
		upscaler->Prepare(windowWidth, windowHeight, downscaleFactor);

	srand(static_cast<unsigned int>(time(NULL)));

	std::vector<VertexSimplified> upscalePositions = {
      {glm::vec3(-1.f, -1.f, 0.0f), glm::vec2(0.0f, 0.0f)},
      {glm::vec3(1.f, -1.f, 0.0f), glm::vec2(1.0f, 0.0f)},
	  {glm::vec3(1.f, 1.f, 0.0f), glm::vec2(1.0f, 1.0f)},
      {glm::vec3(-1.f, 1.f, 0.0f), glm::vec2(0.0f, 1.0f)}
	};

	std::vector<unsigned int> upscaleIndices = {0, 1, 2, 2, 3, 0};
   
	/* CUBE DATA */
	std::vector<VertexSimplified> positions = {
             {glm::vec3(-1.0, -1.0, 1.0), glm::vec2(0.0f, 0.0f)},  {glm::vec3(1.0,  -1.0, 1.0), glm::vec2(1.0f, 0.0f)},  {glm::vec3(1.0,  1.0,  1.0), glm::vec2(1.0f, 1.0f)},
             {glm::vec3(-1.0, -1.0, 1.0), glm::vec2(0.0f, 0.0f)},  {glm::vec3(1.0,  1.0,  1.0), glm::vec2(1.0f, 1.0f)},  {glm::vec3(-1.0, 1.0,  1.0), glm::vec2(0.0f, 1.0f)},

             {glm::vec3( 1.0,  -1.0, 1.0), glm::vec2(0.0f, 0.0f)},  {glm::vec3(1.0,  -1.0, -1.0), glm::vec2(1.0f, 0.0f)}, {glm::vec3(1.0,  1.0,  -1.0), glm::vec2(1.0f, 1.0f)},
             {glm::vec3(1.0,  -1.0, 1.0), glm::vec2(0.0f, 0.0f)},  {glm::vec3(1.0,  1.0,  -1.0), glm::vec2(1.0f, 1.0f)}, {glm::vec3(1.0,  1.0,  1.0), glm::vec2(0.0f, 1.0f)},

             {glm::vec3(1.0,  -1.0, -1.0), glm::vec2(0.0f, 0.0f)}, {glm::vec3(-1.0, -1.0, -1.0), glm::vec2(1.0f, 0.0f)}, {glm::vec3(-1.0, 1.0,  -1.0), glm::vec2(1.0f, 1.0f)},
             {glm::vec3(1.0,  -1.0, -1.0), glm::vec2(0.0f, 0.0f)}, {glm::vec3(-1.0, 1.0,  -1.0), glm::vec2(1.0f, 1.0f)}, {glm::vec3(1.0,  1.0,  -1.0), glm::vec2(0.0f, 1.0f)},

             {glm::vec3(-1.0, -1.0, -1.0), glm::vec2(0.0f, 0.0f)}, {glm::vec3(-1.0, -1.0, 1.0), glm::vec2(1.0f, 0.0f)},  {glm::vec3(-1.0, 1.0,  1.0), glm::vec2(1.0f, 1.0f)},
             {glm::vec3(-1.0, -1.0, -1.0), glm::vec2(0.0f, 0.0f)}, {glm::vec3(-1.0, 1.0,  1.0), glm::vec2(1.0f, 1.0f)},  {glm::vec3(-1.0, 1.0,  -1.0), glm::vec2(0.0f, 1.0f)},

             {glm::vec3(-1.0, 1.0,  1.0), glm::vec2(0.0f, 0.0f)}, {glm::vec3( 1.0,  1.0,  1.0), glm::vec2(1.0f, 0.0f)},  {glm::vec3(1.0,  1.0,  -1.0), glm::vec2(1.0f, 1.0f)},
             {glm::vec3(-1.0, 1.0,  1.0), glm::vec2(0.0f, 0.0f)},  {glm::vec3(1.0,  1.0,  -1.0), glm::vec2(1.0f, 1.0f)}, {glm::vec3(-1.0, 1.0,  -1.0), glm::vec2(0.0f, 1.0f)},

             {glm::vec3(-1.0, -1.0, 1.0), glm::vec2(0.0f, 0.0f)},  {glm::vec3(-1.0, -1.0, -1.0), glm::vec2(1.0f, 0.0f)}, {glm::vec3(1.0,  -1.0, -1.0), glm::vec2(1.0f, 1.0f)},
             {glm::vec3(-1.0, -1.0, 1.0), glm::vec2(0.0f, 0.0f)}, {glm::vec3( 1.0,  -1.0, -1.0), glm::vec2(1.0f, 1.0f)}, {glm::vec3(1.0,  -1.0, 1.0), glm::vec2(0.0f, 1.0f)}};

	 auto copyPositions = positions;
	 auto copyPositions2 = positions;

	if(!bUseProcedural)
	{
		for (auto &vertex : positions) {
			vertex.position *= 0.25f;    
			vertex.position.x += 0.5f;
			vertex.position.y -= 0.10f;
		}

		/* Modfiy these values if you want other clouds to have different locations */
		for (auto &vertex : copyPositions) {
			vertex.position *= 0.25f;    
			vertex.position.x += 0.5f;
			vertex.position.y -= 0.10f;
		}

		for (auto &vertex : copyPositions2) {
			vertex.position *= 0.25f;    
			vertex.position.x += 0.5f;
			vertex.position.y -= 0.10f;
		}
	}


	std::vector<unsigned int> indices;
        for (unsigned int i = 0; i < positions.size(); ++i) {
          indices.push_back(i);
		}
    /* CUBE DATA */
                                                                       
	GLCall(glViewport(0, 0, windowWidth, windowHeight));

    GLCall(glDepthFunc(GL_LESS));

	GLCall(glEnable(GL_BLEND));
	GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

	VertexArray vaCube;
	VertexBuffer vbCube((unsigned int*)positions.data(), (unsigned int)positions.size() * 5 * sizeof(float));
    VertexArray vaUpscale;
    VertexBuffer vbUpscale((unsigned int*)upscalePositions.data(), (unsigned int)upscalePositions.size() * 5 * sizeof(float));
    VertexArray vaCube2;
    VertexBuffer vbCube2((unsigned int*)copyPositions.data(), (unsigned int)copyPositions.size() * 5 * sizeof(float));
	VertexArray vaCube3;
    VertexBuffer vbCube3((unsigned int*)copyPositions2.data(), (unsigned int)copyPositions2.size() * 5 * sizeof(float));

	VertexBufferLayout layout;
	layout.Push<float>(3);
	layout.Push<float>(2);
	vaCube.AddBuffer(vbCube, layout);
    vaCube2.AddBuffer(vbCube2, layout);
    vaCube3.AddBuffer(vbCube3, layout);
    vaUpscale.AddBuffer(vbUpscale, layout);
    IndexBuffer ibPost((unsigned int*)upscaleIndices.data(), (unsigned int)upscaleIndices.size());
	IndexBuffer ib((unsigned int*)indices.data(), (unsigned int)indices.size());


	std::string vertexFilePath = "res/shaders/OpenVDB.vert";
	std::string fragmentFilePath = "res/shaders/OpenVDB.frag";

	if(bUseProcedural)
	{
		vertexFilePath = "res/shaders/Procedural.vert";
		fragmentFilePath = "res/shaders/Procedural.frag";
	}

	/* Setup shaders */
	Shader shader(vertexFilePath, fragmentFilePath);

	Shader fxaaShader("res/shaders/fxaa.vert", "res/shaders/fxaa.frag");
	fxaaShader.Bind();
	fxaaShader.SetUniform2f("SourceSize", (float)windowWidth, (float)windowHeight);

	Shader passThroughShader("res/shaders/passThrough.vert", "res/shaders/passThrough.frag");

	Shader waterRippleShader("res/shaders/water-ripple.vert", "res/shaders/water-ripple.frag");
	waterRippleShader.Bind();
	waterRippleShader.SetUniform2f("SourceSize", (float)windowWidth, (float)windowHeight);

	Shader denoiseShader("res/shaders/denoise.vert", "res/shaders/denoise.frag");
	denoiseShader.Bind();
	denoiseShader.SetUniform2f("SourceSize", (float)windowWidth, (float)windowHeight);
	denoiseShader.SetUniform1f("uSigma", 3.5f);
	denoiseShader.SetUniform1f("uThreshold", .180f);
	denoiseShader.SetUniform1f("uKSigma", 1.4f);

	Shader depthShader("res/shaders/depth.vert", "res/shaders/depth.frag");
	Shader meshMarker("res/shaders/meshMarker.vert", "res/shaders/meshMarker.frag");
	Shader velocityShader("res/shaders/velocity.vert", "res/shaders/velocity.frag");
	Shader framebufferDrawShader("res/shaders/framebuffer-draw.vert", "res/shaders/framebuffer-draw.frag");
	Shader meshShader("res/shaders/mesh.vert", "res/shaders/mesh.frag", "res/shaders/mesh.geom");
	Shader rainShader("res/shaders/rain.vert", "res/shaders/rain.frag");
	Shader glassShader("res/shaders/glass.vert", "res/shaders/glass.frag");

	shader.Bind();

	Texture2D waterSplashTexture;
	waterSplashTexture.Load("water_splash.png", 512, 512, false);

	Texture2D weatherTexture;
	Texture3D worleyTexture;
	Texture3D perlinWorleyDataTexture;
	glm::vec3 windVector(0,0,0);
	glm::vec3 particleVelocity = glm::vec3(0,0,0.3f);

	if(bUseProcedural)
	{
		weatherTexture.Load(weatherTextureName, WeatherTextureSize, WeatherTextureSize, false);
		perlinWorleyDataTexture.Load(perlinWorleyTextureName, PerlinWorleyTextureSize, PerlinWorleyTextureSize, PerlinWorleyTextureSize, false, false);
		worleyTexture.Load(worleyTextureName, WorleyTextureSize, WorleyTextureSize, WorleyTextureSize, false, false);

		weatherTexture.Bind(TextureCounter::GetNextID());
		shader.SetUniform1i("u_Weather", weatherTexture.GetSlot());

		worleyTexture.Bind(TextureCounter::GetNextID());
		shader.SetUniform1i("u_Worley", worleyTexture.GetSlot());

		perlinWorleyDataTexture.Bind(TextureCounter::GetNextID());
		shader.SetUniform1i("u_PerlinWorley", perlinWorleyDataTexture.GetSlot());
	}

	Texture2D waterRippleTexture;
	waterRippleTexture.Load("res/textures/waterRippleTexture.png", 384, 384, false);

	Texture2D snowflakeTexture;
	snowflakeTexture.Load("res/textures/snowflakeTexture.png", 384, 384, false);

	Texture2D snowAlbedoTexture;
	snowAlbedoTexture.Load("res/textures/snow/snowAlbedo.png");

	Texture2D snowNormalTexture1;
	snowNormalTexture1.Load("res/textures/snow/snowNormal1.jpg");

	Texture2D snowNormalTexture2;
	snowNormalTexture2.Load("res/textures/snow/snowNormal2.png");

	Texture2D iceAlbedoTexture;
	iceAlbedoTexture.Load("res/textures/water/iceAlbedo.jpg");

	Texture2D glassNoiseTexture;
	glassNoiseTexture.Load("res/textures/glassNoise.png");

	Texture3D texture3D;
	if(!bUseProcedural)
	{	
		std::vector<float> volumeData;
		OpenVDBReader vdbReader;
		glm::ivec3 volumeDimensions;
		vdbReader.Read("res/vdb/cloud.vdb", volumeData, volumeDimensions);

		texture3D.Load(volumeData, volumeDimensions.x, volumeDimensions.y, volumeDimensions.z, false, true);
		texture3D.Bind(TextureCounter::GetNextID());
		shader.SetUniform1i("u_Texture3D", texture3D.GetSlot());
	}

	Scene scene("res/scene.json");
	std::unordered_map<std::string, Shader*> shaderMap;

	vaCube3.Unbind();
    vbCube2.Unbind();
    vaUpscale.Unbind();
    vbUpscale.Unbind();
	vaCube.Unbind();
	vbCube.Unbind();
	ib.Unbind();
	shader.Unbind();

	scene.GetModel("crate")->meshes[0]->InitCollision(glm::vec3(0,0,0), glm::vec3(0.1f, 0.1f, 0.1f));
	scene.GetModel("ground")->meshes[0]->InitCollision(glm::vec3(0,0,0), glm::vec3(0.5f * 25, 0.5f * 25, 0.5f * 25));
	scene.GetModel("window")->meshes[0]->InitCollision(glm::vec3(0,0,0), glm::vec3(0.5f * 25, 0.5f * 25, 0.5f * 25));
	scene.GetModel("sphere")->meshes[0]->InitCollision(glm::vec3(0,0,0), glm::vec3(0.5f * 25, 0.5f * 25, 0.5f * 25));
	GameWorld::WorldExtent = glm::vec3(250.f, 250.f, 250.f); 
	Shader waterSplashShader("res/shaders/water-splash.vert", "res/shaders/water-splash.frag");
	WaterSplashParticleManager waterSplashParticleManager(1000);
	waterSplashParticleManager.SpawnParticles();

	Shader rainParticleShader("res/shaders/rain-particle.vert", "res/shaders/rain-particle.frag");
	RainParticleManager rainParticleManager(5000, particleVelocity);
	rainParticleManager.SpawnParticles();

	Shader snowParticleShader("res/shaders/snow-particle.vert", "res/shaders/snow-particle.frag");
	SnowParticleManager snowParticleManager(5000, particleVelocity);
	snowParticleManager.SpawnParticles();

	PerspectiveCamera perspectiveCamera = PerspectiveCamera(70.f, (float)windowHeight, (float)windowWidth);
	perspectiveCamera.SetPosition(glm::vec3(0.0f, 0.0f, -0.7f));

	glm::mat4 lightProjection = glm::ortho(-3.0f, 3.0f, -3.0f, 3.0f, -5.f, 5.f); //change size if shadows are not captured
	Shader shadowMappingDepthShader("res/shaders/shadow-mapping-depth.vert", "res/shaders/shadow-mapping-depth.frag");

	if(!bRenderMeshes)
	{
		if(bUseProcedural)
		{
			perspectiveCamera.SetPosition(glm::vec3(-0.2f, 143.284f, 1016.471f));
			perspectiveCamera.SetRotationX(30.670f);
			perspectiveCamera.SetRotationY(-1.750f);
		}
		else
		{
			perspectiveCamera.SetPosition(glm::vec3(0.29f, 0.1f, -1.f));
		}
	}

	PerspectiveCamera prevPerspectiveCamera = perspectiveCamera;

	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(Window::window, true);
	const char* glsl_version = "#version 460";
	ImGui_ImplOpenGL3_Init(glsl_version);

	CameraController cameraController;

	/* Setup framebuffers */
	CubemapFramebuffer cubemapPass(renderWidth, renderWidth);
    Framebuffer fboDownscaledCloudPass(renderWidth, renderHeight);
	Framebuffer fboFXAAPass(windowWidth, windowHeight);
	Framebuffer waterRippleTexturePass(windowWidth, windowHeight);
	DepthFramebuffer fboDepth(renderWidth, renderHeight);
	Framebuffer fboDepthMeshes(windowWidth, windowWidth);
	MotionVectorsFramebuffer fboMotionVectors(renderWidth, renderHeight);
	DepthFramebuffer fboShadowMap(windowWidth, windowHeight);
	Framebuffer* lastFBO = nullptr;

	float deltaTime = 0.0f;
	float lastFrame = 0.0f;
	float timePassed = 0.0f;
	float rainTimePassed = 0.0f;
	float snowAccumulation = 0.0f;
	float thunderstromTimePassed = 0.0f;
	float TimeMultiplier = 1.0f;

	int FrameCount = 0;
	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(Window::window) && !bNeedsReload)
	{
		prevPerspectiveCamera = perspectiveCamera;
		scene.GetRenderer().Clear();

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		float currentFrame = (float)glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        timePassed = currentFrame;
		lastFrame = currentFrame;

		
		depthShader.Bind();
		depthShader.SetUniformMat4f("model", perspectiveCamera.GetViewProjectionMatrix());
		depthShader.SetUniformMat4f("depthMVP", perspectiveCamera.GetViewProjectionMatrix());

		fboDepth.Bind(TextureCounter::GetNextID());
		scene.GetRenderer().Draw(vaUpscale, ibPost, depthShader, bShowDebugInfo);
		fboDepth.Unbind();

		velocityShader.Bind();
		velocityShader.SetUniformMat4f("uModelViewProjectionMat", perspectiveCamera.GetViewProjectionMatrix());
		velocityShader.SetUniformMat4f("uPrevModelViewProjectionMat", prevPerspectiveCamera.GetViewProjectionMatrix());

		fboMotionVectors.Bind(TextureCounter::GetNextID());
		scene.GetRenderer().Draw(vaUpscale, ibPost, velocityShader, bShowDebugInfo);
		fboMotionVectors.Unbind();

		if(RainIntensity > 0.0f)
		{
			rainTimePassed += deltaTime * 0.03;
			if(rainTimePassed > 1.0)
				rainTimePassed = 1.0;
		}
		else
		{
			rainTimePassed -= deltaTime * 0.03;
			if(rainTimePassed < 0.0)
				rainTimePassed = 0.0;
		}

		if(ThunderstormIntensity > 0.0f)
		{
			thunderstromTimePassed += deltaTime * 0.3;
			if(thunderstromTimePassed > 1.0)
				thunderstromTimePassed = 1.0;
		}
		else
		{
			thunderstromTimePassed -= deltaTime * 0.03;
			if(thunderstromTimePassed < 0.0)
				thunderstromTimePassed = 0.0;
		}

		if(SnowIntensity > 0.0f)
		{
			snowAccumulation += deltaTime * 0.03;
			if(snowAccumulation > 1.0)
				snowAccumulation = 1.0;
		}
		else
		{
			snowAccumulation -= deltaTime * 0.03;
			if(snowAccumulation < 0.0)
				snowAccumulation = 0.0;
		}
		
		if(viewLayerType == ViewLayerType::VLT_Color)
		{
			/* CLOUDS PASS */
			if(upscaler || bUseFXAA || bUseDenoiseShader)
			{
				fboDownscaledCloudPass.Bind(TextureCounter::GetNextID());
				scene.GetRenderer().UpdateViewport((float)windowWidth / downscaleFactor, (float)windowHeight / downscaleFactor);
			}
	
			shader.Bind();
			shader.SetUniform1f("u_RotationX", perspectiveCamera.GetRotationX());
			shader.SetUniform1f("u_RotationY", perspectiveCamera.GetRotationY());
			shader.SetUniform1f("u_RotationZ", perspectiveCamera.GetRotationZ());
			shader.SetUniformVec3f("u_Translation", perspectiveCamera.GetPosition());
			shader.SetUniform1i("u_UseMipMaps", (int)bUseMipMaps);
			shader.SetUniform1i("u_UseRain", (int)bUseRain);
			shader.SetUniform1i("u_UseSnow", (int)bUseSnow);
			shader.SetUniform1i("u_UseRainbow", (int)bUseRainbow);
			shader.SetUniform1i("u_UseSunrays", (int)bUseSunrays);
			shader.SetUniform1i("u_NumberOfBolts", NumberOfBolts);
			shader.SetUniformMat4f("u_MVPM", perspectiveCamera.GetViewProjectionMatrix());
			shader.SetUniform2f("u_resolution", windowWidth, windowHeight);
			shader.SetUniform1f("u_downscale", downscaleFactor);
			shader.SetUniform1f("u_aspect", windowWidth / windowHeight);
			shader.SetUniform1f("u_thunderstorm", ThunderstormIntensity);
			shader.SetUniform1f("u_thunderstormTimePassed", thunderstromTimePassed);

			fboDepthMeshes.GetTexture().Bind(TextureCounter::GetNextID());
			shader.SetUniform1i("u_depthTexture", fboDepthMeshes.GetTexture().GetSlot());

	
			glm::vec3 sunPosition;
			if(bUseProcedural)
			{

				const float PI = 3.141592653589793238462643383279502884197169;
				const float theta = PI * ( -0.23+0.25*sin(timePassed * 0.01 * TimeMultiplier));
				const float phi = 2 * PI * (-0.25);
				const float sunposx = cos( phi );
				const float sunposy = sin( phi ) * sin( theta );
				const float sunposz = sin( phi ) * cos( theta );

				shader.SetUniform1f("u_Time", timePassed * TimeMultiplier);
				shader.SetUniform1f("u_WeatherScale", WeatherScale);
				shader.SetUniform1f("u_WorleyScale", WorleyScale);
				shader.SetUniform1f("u_PerlinWorleyScale", PerlinWorleyScale);
				shader.SetUniform1f("u_CloudType", CloudType);
				shader.SetUniformVec3f("sunPosition", glm::vec3(sunposx, sunposy, sunposz));
				meshShader.Bind();
				meshShader.SetUniformVec3f("u_lightPosition", glm::vec3(sunposx, sunposy, sunposz));
				shader.Bind();
				sunPosition = glm::vec3(sunposx, sunposy, sunposz);
			}
			glm::mat4 lightView = glm::lookAt(sunPosition, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
			glm::mat4 lightSpaceMatrix = lightProjection * lightView;

			scene.GetRenderer().Draw(vaCube, ib, shader, bShowDebugInfo);
			if(!bUseProcedural)
			{
				//Disable rendering two or more clouds by default, decomment these lines if needed
				scene.GetRenderer().Draw(vaCube2, ib, shader, bShowDebugInfo);
				scene.GetRenderer().Draw(vaCube3, ib, shader, bShowDebugInfo);
			}
			
			if(upscaler || bUseFXAA || bUseDenoiseShader)
			{
				fboDownscaledCloudPass.Unbind();
				lastFBO = &fboDownscaledCloudPass;
			}
			fboDepthMeshes.GetTexture().Unbind();
			scene.GetRenderer().UpdateViewport((float)windowWidth, (float)windowHeight);

			/* UPSCALE PASS */
			if(upscaler)
			{
				if(FSR2Upscaler* fsrUpscaler = dynamic_cast<FSR2Upscaler*>(upscaler.get()))
				{
					fsrUpscaler->MotionVectorsTexture = &fboMotionVectors.GetTexture();
					fsrUpscaler->DepthTexture = &fboDepth.GetTexture();
				}

				if(Shader* upscalerShader = upscaler->Run1(&fboDownscaledCloudPass, FrameCount, deltaTime))
				{
					scene.GetRenderer().Draw(vaUpscale, ibPost, *upscalerShader, bShowDebugInfo);
					upscaler->PostRun1();
				}
			
				if(Shader* upscalerShader = upscaler->Run2(&fboDownscaledCloudPass, FrameCount, deltaTime))
				{
					scene.GetRenderer().Draw(vaUpscale, ibPost, *upscalerShader, bShowDebugInfo);
					upscaler->PostRun2();
				}
			
				if(lastFBO)
					lastFBO->Unbind();
				lastFBO = upscaler->GetResultFBO();
			}

			/* FXAA PASS */
			if(bUseFXAA)
			{	
				if(bUseDenoiseShader)
				{
					fboFXAAPass.Bind(TextureCounter::GetNextID());
				}

				fxaaShader.Bind();
				int FrameBufferID = 0;
				if(lastFBO)
				{
					lastFBO->GetTexture().Bind(TextureCounter::GetNextID());
					FrameBufferID = lastFBO->GetTexture().GetSlot();
				}
				else 
				{
					fboDownscaledCloudPass.GetTexture().Bind(TextureCounter::GetNextID());
					FrameBufferID = fboDownscaledCloudPass.GetTexture().GetSlot();
				}
	
				fxaaShader.SetUniform1i("Source", FrameBufferID);
				scene.GetRenderer().Draw(vaUpscale, ibPost, fxaaShader, bShowDebugInfo);

				if(bUseDenoiseShader)
				{
					fboFXAAPass.Unbind();
					lastFBO = &fboFXAAPass;
				}
			}
			else
			{
				if(bUseDenoiseShader)
				{
					fboFXAAPass.Bind(TextureCounter::GetNextID());
				}

				passThroughShader.Bind();
				int FrameBufferID = 0;
				if(lastFBO)
				{
					lastFBO->GetTexture().Bind(TextureCounter::GetNextID());
					FrameBufferID = lastFBO->GetTexture().GetSlot();
				}
				else 
				{
					fboDownscaledCloudPass.GetTexture().Bind(TextureCounter::GetNextID());
					FrameBufferID = fboDownscaledCloudPass.GetTexture().GetSlot();
				}
	
				fxaaShader.SetUniform1i("Source", FrameBufferID);
				passThroughShader.SetUniform1i("Source", FrameBufferID); // Bind texture to shader uniform

				scene.GetRenderer().Draw(vaUpscale, ibPost, passThroughShader, bShowDebugInfo);

				if(bUseDenoiseShader)
				{
					fboFXAAPass.Unbind();
					lastFBO = &fboFXAAPass;
				}
			}

			/* DENOISE PASS */
			if(bUseDenoiseShader && lastFBO)
			{
				denoiseShader.Bind();
				lastFBO->GetTexture().Bind(TextureCounter::GetNextID());
				denoiseShader.SetUniform1i("Source", lastFBO->GetTexture().GetSlot());
				scene.GetRenderer().Draw(vaUpscale, ibPost, denoiseShader, bShowDebugInfo);
			}
	
			if(lastFBO)
				lastFBO->GetTexture().Unbind();

			if(bRenderMeshes)
			{
				GLCall(glEnable(GL_DEPTH_TEST));
				/*Render shadows*/			
				fboShadowMap.Bind(TextureCounter::GetNextID());
				GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
				//GLCall(glCullFace(GL_FRONT));
				shadowMappingDepthShader.Bind();
				shadowMappingDepthShader.SetUniformMat4f("lightSpaceMatrix", lightSpaceMatrix);
				shaderMap.clear();
				shaderMap["crate"] = &shadowMappingDepthShader;
				shaderMap["window"] = &shadowMappingDepthShader;
				shaderMap["ground"] = &shadowMappingDepthShader;
				shaderMap["sphere"] = &shadowMappingDepthShader;
				scene.Draw(shaderMap);
				fboShadowMap.Unbind();
				//GLCall(glCullFace(GL_BACK));
				GLCall(glClear(GL_DEPTH_BUFFER_BIT));

				GLCall(glEnable(GL_DEPTH_TEST));
				/*Render depth buffer*/
				depthShader.Bind();
				depthShader.SetUniformMat4f("depthMVP", perspectiveCamera.GetViewProjectionMatrix());
				fboDepthMeshes.Bind(TextureCounter::GetNextID());
				GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
				shaderMap.clear();
				shaderMap["crate"] = &depthShader;
				shaderMap["window"] = &depthShader;
				shaderMap["ground"] = &depthShader;
				shaderMap["sphere"] = &depthShader;
				scene.Draw(shaderMap);
				fboDepthMeshes.Unbind();
				depthShader.Unbind();
				GLCall(glClear(GL_DEPTH_BUFFER_BIT));

	
				/*Render Cube*/
				shaderMap.clear();
				shaderMap["ground"] = &meshShader;

				meshShader.Bind();
				meshShader.SetUniformMat4f("lightSpaceMatrix", lightSpaceMatrix);
				fboShadowMap.GetTexture().Bind(TextureCounter::GetNextID()); //Bind shadow map
				meshShader.SetUniform1i("shadowMap", fboShadowMap.GetTexture().GetSlot());
				meshShader.SetUniform1f("u_useCubemap", (float)(FrameCount != 0));
				cubemapPass.GetCubemap().Bind(TextureCounter::GetNextID());
				meshShader.SetUniform1i("cubemap", cubemapPass.GetCubemap().GetSlot());
				meshShader.SetUniformVec3f("u_eyePosition", perspectiveCamera.GetPosition());
				meshShader.SetUniformMat4f("u_MVPM", perspectiveCamera.GetViewProjectionMatrix());
				meshShader.SetUniform1f("u_rainIntensity", RainIntensity);
				meshShader.SetUniform1f("u_rainTimePassed", rainTimePassed);
				meshShader.SetUniform1f("snowAccumulation", snowAccumulation);
				meshShader.SetUniform1f("maxDisplacement", 0.05f);
				snowAlbedoTexture.Bind(TextureCounter::GetNextID());
				meshShader.SetUniform1i("snowAlbedoTexture", snowAlbedoTexture.GetSlot());
				iceAlbedoTexture.Bind(TextureCounter::GetNextID());
				meshShader.SetUniform1i("iceAlbedoTexture", iceAlbedoTexture.GetSlot());
				snowNormalTexture1.Bind(TextureCounter::GetNextID());
				meshShader.SetUniform1i("snowNormal1Texture", snowNormalTexture1.GetSlot());
				snowNormalTexture2.Bind(TextureCounter::GetNextID());
				meshShader.SetUniform1i("snowNormal2Texture", snowNormalTexture2.GetSlot());
				meshShader.SetUniform1i("u_FrameCount", FrameCount);

				meshShader.SetUniformMat4f("view", perspectiveCamera.GetViewMatrix());
				waterRippleTexturePass.GetTexture().Bind(TextureCounter::GetNextID());
				meshShader.SetUniform1i("RippleTexture", waterRippleTexturePass.GetTexture().GetSlot());
				scene.Draw(shaderMap);
				waterRippleTexturePass.GetTexture().Unbind();

				shaderMap.clear();
				shaderMap["crate"] = &meshShader;
				shaderMap["sphere"] = &meshShader;

				meshShader.SetUniform1f("u_rainIntensity", 0.0f);
				meshShader.SetUniform1f("u_rainTimePassed", 0.0f);
				meshShader.SetUniform1f("u_useCubemap",  0.0f);
				scene.Draw(shaderMap);
				snowAlbedoTexture.Unbind();
				iceAlbedoTexture.Unbind();
				snowNormalTexture1.Unbind();
				snowNormalTexture2.Unbind();

				/*Glass drops */
				shaderMap.clear();
				shaderMap["window"] = &glassShader;
				shaderMap["sphere"] = &meshShader;

				glassShader.Bind();
				glassShader.SetUniform1f("u_useCubemap",  0.0f);
				glassShader.SetUniform1i("cubemap", cubemapPass.GetCubemap().GetSlot());
				glassShader.SetUniformVec3f("u_eyePosition", perspectiveCamera.GetPosition());
				glassShader.SetUniformMat4f("u_MVPM", perspectiveCamera.GetViewProjectionMatrix());
				glassShader.SetUniform1f("u_rainIntensity", RainIntensity);
				glassShader.SetUniform1f("u_Time", timePassed * TimeMultiplier);
				glassShader.SetUniformMat4f("view", perspectiveCamera.GetViewMatrix());
				glassShader.SetUniform1f("snowAccumulation", snowAccumulation);
				glassNoiseTexture.Bind(TextureCounter::GetNextID());
				glassShader.SetUniform1i("glassNoiseTexture", glassNoiseTexture.GetSlot());
				scene.Draw(shaderMap);
				glassNoiseTexture.Unbind();

				waterSplashShader.Bind();
				waterSplashTexture.Bind(TextureCounter::GetNextID());
				waterSplashShader.SetUniform1i("texture_diffuse1", waterSplashTexture.GetSlot());
				waterSplashParticleManager.SetRainIntensity(RainIntensity);
				waterSplashParticleManager.Update(deltaTime);
				waterSplashParticleManager.Draw(scene.GetRenderer(), perspectiveCamera.GetViewProjectionMatrix(), &waterSplashShader);

				/*Rain particles*/

				rainParticleShader.Bind();
				rainParticleManager.SetRainIntensity(RainIntensity);
				rainParticleManager.Update(deltaTime);
				rainParticleManager.Draw(scene.GetRenderer(), perspectiveCamera.GetViewProjectionMatrix(), &rainParticleShader);

				/*Snow particles*/

				snowParticleShader.Bind();
				snowflakeTexture.Bind(TextureCounter::GetNextID());
				snowParticleShader.SetUniform1i("snowflakeTexture", snowflakeTexture.GetSlot());
				snowParticleManager.SetSnowIntensity(SnowIntensity);
				snowParticleManager.Update(deltaTime);
				snowParticleManager.Draw(scene.GetRenderer(), perspectiveCamera.GetViewProjectionMatrix(), &snowParticleShader);
				snowflakeTexture.Unbind();

				GLCall(glDisable(GL_DEPTH_TEST));
				GLCall(glClear(GL_DEPTH_BUFFER_BIT));
				/* generate water ripple texture */
				waterRippleTexturePass.Bind(TextureCounter::GetNextID());
				waterRippleTexture.Bind(TextureCounter::GetNextID());
				waterRippleShader.Bind();
				waterRippleShader.SetUniform1f("Time", timePassed * TimeMultiplier);
				waterRippleShader.SetUniform1i("RippleTexture", waterRippleTexture.GetSlot());
				waterRippleShader.SetUniform1f("u_rainIntensity", RainIntensity);
				scene.GetRenderer().Draw(vaUpscale, ibPost, waterRippleShader, bShowDebugInfo);
				waterRippleTexture.Unbind();
				waterRippleTexturePass.Unbind();


				/* rain shader cone post process */
				if(bRenderRainCone)
				{
					rainShader.Bind();
					glm::mat4 coneModelTransform = glm::mat4(1.0f);
					coneModelTransform = glm::translate(coneModelTransform, -perspectiveCamera.GetPosition() + glm::vec3(0,3.0, 0.0));
					coneModelTransform = glm::scale(coneModelTransform, glm::vec3(10.0f, 10.0f, 10.0f));

					shaderMap.clear();
					shaderMap["cone"] = &rainShader;

					rainShader.SetUniform1f("u_Time", timePassed * TimeMultiplier);
					rainShader.SetUniform2f("u_resolution", windowWidth, windowHeight);
					rainShader.SetUniformMat4f("u_MVPM", perspectiveCamera.GetViewProjectionMatrix());
					rainShader.SetUniformMat4f("model", coneModelTransform);
					fboDepthMeshes.GetTexture().Bind(TextureCounter::GetNextID());
					rainShader.SetUniform1i("u_depthTexture", fboDepthMeshes.GetTexture().GetSlot());
					scene.Draw(shaderMap);
					fboDepthMeshes.GetTexture().Unbind();
				}

				if(FrameCount == 0) //GIGA HACK to have cubemap to refactor, it's okayinh at 10 at frame, but maybe some kind of blending between updates would be better
				{
					scene.GetRenderer().UpdateViewport((float)windowWidth / (downscaleFactor), (float)windowWidth / (downscaleFactor));
					cubemapPass.Bind(TextureCounter::GetNextID());
					PerspectiveCamera cubemapCamera = PerspectiveCamera(glm::radians(90.f), 1.0f, 1.0f); 

					glm::vec3 position = perspectiveCamera.GetPosition();
					//position.y = 1.0f;

					cubemapCamera.SetPosition(position);

					for(unsigned int face = 0; face < 6; ++face)
					{
						switch (face)
						{
						case 0: //POSITIVE X
							cubemapCamera.SetViewMatrix(glm::lookAt(cubemapCamera.GetPosition(), cubemapCamera.GetPosition() + glm::vec3(1,0,0), glm::vec3(0,-1,0)));
							break;
						case 1: //NEGATIVE X
							cubemapCamera.SetViewMatrix(glm::lookAt(cubemapCamera.GetPosition(), cubemapCamera.GetPosition() + glm::vec3(-1,0,0), glm::vec3(0,-1,0)));
							break;
						case 2: //POSITIVE Y
							cubemapCamera.SetViewMatrix(glm::lookAt(cubemapCamera.GetPosition(), cubemapCamera.GetPosition() + glm::vec3(0,1,0), glm::vec3(0,0,1.f)));
							break;
						case 3: //NEGATIVE Y
							cubemapCamera.SetViewMatrix(glm::lookAt(cubemapCamera.GetPosition(), cubemapCamera.GetPosition() + glm::vec3(0,-1,0), glm::vec3(0,0,-1.0f)));
							break;
						case 4: //POSITIVE Z
							cubemapCamera.SetViewMatrix(glm::lookAt(cubemapCamera.GetPosition(), cubemapCamera.GetPosition() + glm::vec3(0,0,1), glm::vec3(0,-1,0)));
							break;
						case 5: //NEGATIVE Z
							cubemapCamera.SetViewMatrix(glm::lookAt(cubemapCamera.GetPosition(), cubemapCamera.GetPosition() + glm::vec3(0,0,-1), glm::vec3(0,-1,0)));
							break;
						default:
							break;
						}
						cubemapPass.DrawToFace(face);
						shader.Bind();
						shader.SetUniformMat4f("u_MVPM", cubemapCamera.GetViewProjectionMatrix());
						scene.GetRenderer().Draw(vaCube, ib, shader, bShowDebugInfo);
						GLCall(glEnable(GL_DEPTH_TEST));
						shaderMap.clear();
						shaderMap["ground"] = &meshShader;
						shaderMap["crate"] = &meshShader;
						shaderMap["window"] = &meshShader;
						shaderMap["sphere"] = &meshShader;

						meshShader.Bind();
						meshShader.SetUniformMat4f("u_MVPM", cubemapCamera.GetViewProjectionMatrix());
						scene.Draw(shaderMap);
						GLCall(glDisable(GL_DEPTH_TEST));
					}
					cubemapPass.Unbind();


					scene.GetRenderer().UpdateViewport((float)windowWidth, (float)windowHeight);
				}
			}

		}
		//Depth texture
		else if(viewLayerType == ViewLayerType::VLT_Depth)
		{
			framebufferDrawShader.Bind();
			fboDepth.Bind(TextureCounter::GetNextID());
			framebufferDrawShader.SetUniform1i("u_Fbo", fboDepth.GetTexture().GetSlot());
			scene.GetRenderer().Draw(vaUpscale, ibPost, framebufferDrawShader);
			fboDepth.Unbind();
			framebufferDrawShader.Unbind();
		}
		//MotionVectors texture
		else
		{
			framebufferDrawShader.Bind();
			fboMotionVectors.Bind(TextureCounter::GetNextID());
			framebufferDrawShader.SetUniform1i("u_Fbo", fboMotionVectors.GetTexture().GetSlot());
			scene.GetRenderer().Draw(vaUpscale, ibPost, framebufferDrawShader);
			fboMotionVectors.Unbind();
			framebufferDrawShader.Unbind();
		}

		FrameCount++;

		/* IMGUI SETUP */
		{
			cameraController.m_translation = perspectiveCamera.GetPosition();
			cameraController.m_rotationX = perspectiveCamera.GetRotationX();
			cameraController.m_rotationY = perspectiveCamera.GetRotationY();
			cameraController.m_rotationZ = perspectiveCamera.GetRotationZ();
			ImGui::SliderFloat3("Translation", &cameraController.m_translation.x, -(float)10000, (float)10000);
			ImGui::SliderFloat("RotationX", &cameraController.m_rotationX, 0.0f, 360.f);
			ImGui::SliderFloat("RotationY", &cameraController.m_rotationY, 0.0f, 360.f);
			ImGui::SliderFloat("RotationZ", &cameraController.m_rotationZ, 0.0f, 360.f);
			ImGui::SliderFloat("RotationSpeed", &cameraController.m_cameraRotationSpeed, 0.0f, 2.f);
			ImGui::SliderFloat("ZoomSpeed", &cameraController.m_cameraZoomSpeed, 0.0f, 2.f);
			ImGui::SliderFloat("MoveSpeed", &cameraController.m_cameraMoveSpeed, 0.0f, 2.f);
			ImGui::SliderFloat("CloudType", &CloudType, 0.0f, 1.f);
			ImGui::SliderFloat("TimeMultiplier", &TimeMultiplier, 0.1f, 10.f);
			ImGui::SliderFloat("WeatherScale", &WeatherScale, 0.00001f, 0.0001f);
			ImGui::SliderFloat("WorleyScale", &WorleyScale, 0.0001f, 0.001f);
			ImGui::SliderFloat("PerlinWorleyScale", &PerlinWorleyScale, 0.0001f, 0.001f);
			if(ImGui::SliderInt("DownscaleFactor", &downscaleFactor, 1, 16))
			{
				bNeedsReload = true;
			}

			{
				const char* element_names[UpscalerVersion::Count] = { "None","Simple", "One", "Two"};
				int current_element = upscalerVersion;
				const char* current_element_name = (current_element >= 0 && current_element < UpscalerVersion::Count) ? element_names[current_element] : "Unknown";
				if( ImGui::SliderInt("Upscaler Type", &current_element, 0, UpscalerVersion::Count - 1, current_element_name) ||
				   ImGui::Checkbox("Use FXAA", &bUseFXAA) ||
				   ImGui::Checkbox("Use denoise shader", &bUseDenoiseShader) || 
				   ImGui::Checkbox("Use Mip Maps", &bUseMipMaps))
				{
					upscalerVersion = (UpscalerVersion)current_element;
					bNeedsReload = true;
				}
				ImGui::Checkbox("Use Rain", &bUseRain);
				ImGui::Checkbox("Use Snow", &bUseSnow);
				ImGui::Checkbox("Use Rainbow", &bUseRainbow);
				ImGui::Checkbox("Render Meshes", &bRenderMeshes);
				ImGui::Checkbox("Render Splashes", &bRenderSplashes);
				ImGui::Checkbox("Render Ripples", &bRenderRipples);
				ImGui::Checkbox("Render Puddles", &bRenderPuddles);
				ImGui::Checkbox("Render Rain Cone", &bRenderRainCone);
				ImGui::Checkbox("Use Sun Rays", &bUseSunrays);
				ImGui::SliderInt("Number of Bolts", &NumberOfBolts, 1, 12);
				ImGui::SliderFloat("Rain Intensity", &RainIntensity, 0.0f, 1.0f);
				ImGui::SliderFloat("Snow Intensity", &SnowIntensity, 0.0f, 1.0f);
				ImGui::SliderFloat("Thunderstorm", &ThunderstormIntensity, 0.0f, 1.0f);
				ImGui::Checkbox("Show Debug Info", &bShowDebugInfo);
			}

			ImGui::SliderFloat3("WindVector", &windVector.x, -1.f, 1.f);
			ImGui::SliderFloat3("InitialParticleVelocity", &particleVelocity.x, -1.f, 1.f);
			rainParticleManager.SetWind(windVector);
			snowParticleManager.SetWind(windVector);
			snowParticleManager.SetInitialParticleVelocity(particleVelocity);
			snowParticleManager.SetInitialParticleVelocity(particleVelocity);
			scene.SetDebugInfo(bShowDebugInfo, debugOutputFileName);

			{
				const char* element_names[ViewLayerType::VLT_Count] = { "Color", "Depth", "Motion Vectors"};
				int current_element = viewLayerType;
				const char* current_element_name = (current_element >= 0 && current_element < ViewLayerType::VLT_Count) ? element_names[current_element] : "Unknown";
				if( ImGui::SliderInt("View layer Type", &current_element, 0, ViewLayerType::VLT_Count - 1, current_element_name))
				{
					viewLayerType = (ViewLayerType)current_element;
					bNeedsReload = true;
				}
			}


			if(ImGui::Button("Generate textures", ImVec2(200.f, 20.f)))
			{
				GenerateTextures();
			}

			if(!bUseProcedural)
			{
				if(ImGui::Button("Switch to procedural", ImVec2(200.f, 20.f)))
				{
					bUseProcedural = true;
					bNeedsReload = true;
				}
			}
			else 
			{
				if(ImGui::Button("Switch to OpenVDB", ImVec2(200.f, 20.f)))
				{
					bUseProcedural = false;
					bNeedsReload = true;
				}
			}

			/* Saving to file */
			if(ImGui::Button("Save to file", ImVec2(200.f, 20.f)))
			{
				BYTE* pixels = new BYTE[3 * windowWidth * windowHeight];

				glReadPixels(0, 0, windowWidth, windowHeight, GL_RGB, GL_UNSIGNED_BYTE, pixels);

				FIBITMAP* image = FreeImage_ConvertFromRawBits(pixels, windowWidth, windowHeight, 3 * windowWidth, 24, 0xFF0000, 0x00FF00, 0x0000FF, false);
				const std::string path = std::filesystem::current_path().string() + fileName;
				FreeImage_Save(FIF_PNG, image, path.c_str(), 0);
				
				FreeImage_Unload(image);
				delete [] pixels;
			}

			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			ImGui::End();

			cameraController.update();
			perspectiveCamera.SetPosition(cameraController.m_translation);
			perspectiveCamera.SetRotationX(cameraController.m_rotationX);
			perspectiveCamera.SetRotationY(cameraController.m_rotationY);
			perspectiveCamera.SetRotationZ(cameraController.m_rotationZ);

			//in opengl Y is up
			waterSplashParticleManager.SetCenterPosition(glm::vec3(cameraController.m_translation.x, cameraController.m_translation.z, 0.0f));
			rainParticleManager.SetCenterPosition(glm::vec3(cameraController.m_translation.x, cameraController.m_translation.z, 0.0f));
		}

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		ImGui::EndFrame();

		/* Swap front and back buffers */
		glfwSwapBuffers(Window::window);

		/* Poll for and process events */
		glfwPollEvents();

		GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
#if ENFORCE_SLOW_FRAMERATE_HACK
		std::this_thread::sleep_for(0.2 * std::chrono::seconds(1));
#endif
	}
	
	vaCube3.Unbind();
    vbCube3.Unbind();
	vaCube2.Unbind();
    vbCube2.Unbind();
    vaUpscale.Unbind();
    vbUpscale.Unbind();
	vaCube.Unbind();
	vbCube.Unbind();
	ib.Unbind();
	shader.Unbind();
	fxaaShader.Unbind();
	passThroughShader.Unbind();
	meshShader.Unbind();
	glassShader.Unbind();
	rainShader.Unbind();
	waterSplashShader.Unbind();
	waterRippleShader.Unbind();

	upscaler->Unbind();
	upscaler = nullptr;

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	scene.SaveStatisticsToFile();
	scene.Uninitialize();

	if (bNeedsReload)
	{
		downscaleFactor = (upscalerVersion != UpscalerVersion::None) ? 4 : 1;
		bNeedsReload = false;
		Render();
	}
}

/* Method used to generate textures used for procedural clouds */
void Application::GenerateTextures()
{
	NoiseTextureGenerator noiseTextureGenerator;

	std::vector<float> perlinData;
	noiseTextureGenerator.GeneratePerlinTexture2D(WeatherTextureSize, WeatherTextureSize,perlinData);	
	TextureSaver::SaveTextureToFile(perlinData, weatherTextureName, WeatherTextureSize, WeatherTextureSize, 4);

	std::vector<float> worleyData;
	noiseTextureGenerator.GenerateWorleyTexture3D(WorleyTextureSize, WorleyTextureSize, WorleyTextureSize, worleyData);
	TextureSaver::SaveTextureToFile(worleyData, worleyTextureName, WorleyTextureSize * WorleyTextureSize, WorleyTextureSize, 4);

	std::vector<float> perlinWorleyData;
	noiseTextureGenerator.GeneratePerlinWorleyTexture3D(PerlinWorleyTextureSize, PerlinWorleyTextureSize, PerlinWorleyTextureSize, perlinWorleyData);	
	TextureSaver::SaveTextureToFile(perlinWorleyData, perlinWorleyTextureName, PerlinWorleyTextureSize * PerlinWorleyTextureSize, PerlinWorleyTextureSize, 4);
}
