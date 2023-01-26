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

/* Configuration properties */
bool Application::bNeedsReload = false;
int Application::windowWidth = 1920;
int Application::windowHeight = 1080;
int Application::downscaleFactor = 4;
bool Application::bUseFXAA = true;
bool Application::bUseFSR = true;
bool Application::bUseUpscaleShader = false;
bool Application::bUseProcedural = true;
bool Application::bUseDenoiseShader = true;
bool Application::bUseMipMaps = true;
int Application::WeatherTextureSize = 128;
int Application::WorleyTextureSize = 32;
int Application::PerlinWorleyTextureSize = 128;
float Application::WeatherScale = 0.05f;
float Application::WorleyScale = 0.004f;
float Application::PerlinWorleyScale = 0.3f;
float Application::CloudType = 0.2f;

/* Constants */
constexpr const char* weatherTextureName = "perlin.bmp";
constexpr const char* worleyTextureName = "worley.bmp";
constexpr const char* perlinWorleyTextureName = "perlinWorley.bmp";
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

    if (glewInit() != GLEW_OK)
        std::cout << "Error!" << std::endl;

    std::cout << glGetString(GL_VERSION) << std::endl;

    Render();

    glfwTerminate();
    return 0;
}

/* Main rendering method */
void Application::Render()
{
	srand(static_cast<unsigned int>(time(NULL)));

	std::vector<Vertex> upscalePositions = {
      {glm::vec3(-1.f, -1.f, 0.0f), glm::vec2(0.0f, 0.0f)},
      {glm::vec3(1.f, -1.f, 0.0f), glm::vec2(1.0f, 0.0f)},
	  {glm::vec3(1.f, 1.f, 0.0f), glm::vec2(1.0f, 1.0f)},
      {glm::vec3(-1.f, 1.f, 0.0f), glm::vec2(0.0f, 1.0f)}
	};

	std::vector<unsigned int> upscaleIndices = {0, 1, 2, 2, 3, 0};
   
	/* CUBE DATA */
	std::vector<Vertex> positions = {
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
		//	vertex.position *= 0.25f;
		//	vertex.position.x -= 0.3f;
		//	vertex.position.y -= 0.10f;
		}

		for (auto &vertex : copyPositions2) {
			vertex.position *= 0.25f;    
			vertex.position.x += 0.5f;
			vertex.position.y -= 0.10f;
		//	vertex.position *= 0.25f;
		//	vertex.position.x += 0.1f;
		//	vertex.position.y -= 0.10f;
		}
	}


	std::vector<unsigned int> indices;
        for (unsigned int i = 0; i < positions.size(); ++i) {
          indices.push_back(i);
		}
    /* CUBE DATA */
                                                                       
	GLCall(glViewport(0, 0, windowWidth, windowHeight));

	GLCall(glEnable(GL_DEPTH_TEST));
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

	Shader upscaleShader("res/shaders/upscale.vert", "res/shaders/upscale.frag");
	upscaleShader.Bind();
	upscaleShader.SetUniform2f("u_Resolution", (float)windowWidth, (float)windowHeight);
	upscaleShader.SetUniform1f("u_DownscaleFactor", (float)downscaleFactor);

	Shader fsrPass0("res/shaders/fsr-pass0.vert", "res/shaders/fsr-pass0.frag");
	fsrPass0.Bind();
	fsrPass0.SetUniform2f("SourceSize", (float)windowWidth / downscaleFactor, (float)windowHeight / downscaleFactor);
	fsrPass0.SetUniform2f("OutputSize", (float)windowWidth, (float)windowHeight);

	Shader fsrPass1("res/shaders/fsr-pass1.vert", "res/shaders/fsr-pass1.frag");
	fsrPass1.Bind();
	fsrPass1.SetUniform2f("OutputSize", (float)windowWidth, (float)windowHeight);

	Shader fxaaShader("res/shaders/fxaa.vert", "res/shaders/fxaa.frag");
	fxaaShader.Bind();
	fxaaShader.SetUniform2f("SourceSize", (float)windowWidth, (float)windowHeight);

	Shader denoiseShader("res/shaders/denoise.vert", "res/shaders/denoise.frag");
	denoiseShader.Bind();
	denoiseShader.SetUniform2f("SourceSize", (float)windowWidth, (float)windowHeight);
	denoiseShader.SetUniform1f("uSigma", 3.5f);
	denoiseShader.SetUniform1f("uThreshold", .180f);
	denoiseShader.SetUniform1f("uKSigma", 1.4f);

	shader.Bind();

	Texture2D weatherTexture;
	Texture3D worleyTexture;
	Texture3D perlinWorleyDataTexture;

	if(bUseProcedural)
	{
		weatherTexture.Load(weatherTextureName, WeatherTextureSize, WeatherTextureSize);
		perlinWorleyDataTexture.Load(perlinWorleyTextureName, PerlinWorleyTextureSize, PerlinWorleyTextureSize, PerlinWorleyTextureSize, false, false);
		worleyTexture.Load(worleyTextureName, WorleyTextureSize, WorleyTextureSize, WorleyTextureSize, false, false);

		weatherTexture.Bind(5);
		shader.SetUniform1i("u_Weather", 5);

		worleyTexture.Bind(6);
		shader.SetUniform1i("u_Worley", 6);

		perlinWorleyDataTexture.Bind(7);
		shader.SetUniform1i("u_PerlinWorley", 7);
	}

	Texture3D texture3D;
	if(!bUseProcedural)
	{	
		std::vector<float> volumeData;
		OpenVDBReader vdbReader;
		glm::ivec3 volumeDimensions;
		vdbReader.Read("res/vdb/cloud.vdb", volumeData, volumeDimensions);

		texture3D.Load(volumeData, volumeDimensions.x, volumeDimensions.y, volumeDimensions.z, false, true);
		texture3D.Bind(1);
		shader.SetUniform1i("u_Texture3D", 1);
	}

	vaCube3.Unbind();
    vbCube2.Unbind();
    vaUpscale.Unbind();
    vbUpscale.Unbind();
	vaCube.Unbind();
	vbCube.Unbind();
	ib.Unbind();
	shader.Unbind();

	Renderer renderer;
	PerspectiveCamera orthographicCamera = PerspectiveCamera(90.f, (float)windowHeight, (float)windowWidth);

	if(bUseProcedural)
	{
		orthographicCamera.SetPosition(glm::vec3(-0.63f, -1.46f, -2.f));
		orthographicCamera.SetRotationY(124.f);
		orthographicCamera.SetRotationZ(90.f);
	}
	else
	{
		orthographicCamera.SetPosition(glm::vec3(0.29f, 0.1f, -1.f));
	}

	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(Window::window, true);
	const char* glsl_version = "#version 460";
	ImGui_ImplOpenGL3_Init(glsl_version);

	CameraController cameraController;

	/* Setup framebuffers */
    Framebuffer fboDownscaled(windowWidth / downscaleFactor, windowHeight / downscaleFactor);
    Framebuffer fboFSRPass1(windowWidth, windowHeight);
	Framebuffer fboNormalScale(windowWidth, windowHeight);
	Framebuffer fboFXAAPass(windowWidth, windowHeight);
	Framebuffer* lastFBO = nullptr;

	float deltaTime = 0.0f;
	float lastFrame = 0.0f;
	float timePassed = 0.0f;

	int FrameCount = 0;

	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(Window::window) && !bNeedsReload)
	{
		renderer.Clear();

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		float currentFrame = (float)glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        timePassed = currentFrame;
		lastFrame = currentFrame;

        /* CLOUDS PASS */
		if(bUseFSR || bUseUpscaleShader || bUseFXAA || bUseDenoiseShader)
		{
			fboDownscaled.Bind(3);
			renderer.UpdateViewport((float)windowWidth / downscaleFactor, (float)windowHeight / downscaleFactor);
		}
	
		shader.Bind();
		shader.SetUniform1f("u_RotationX", orthographicCamera.GetRotationX());
		shader.SetUniform1f("u_RotationY", orthographicCamera.GetRotationY());
		shader.SetUniform1f("u_RotationZ", orthographicCamera.GetRotationZ());
		shader.SetUniformVec3f("u_Translation", orthographicCamera.GetPosition());
		shader.SetUniform1i("u_UseMipMaps", (int)bUseMipMaps);

		if(bUseProcedural)
		{
			shader.SetUniform1f("u_Time", timePassed);
			shader.SetUniform1f("u_WeatherScale", WeatherScale);
			shader.SetUniform1f("u_WorleyScale", WorleyScale);
			shader.SetUniform1f("u_PerlinWorleyScale", PerlinWorleyScale);
			shader.SetUniform1f("u_CloudType", CloudType);
		}

		renderer.Draw(vaCube, ib, shader);
		if(!bUseProcedural)
		{
			//Disable rendering two or more clouds by default, decomment these lines if needed
			//renderer.Draw(vaCube2, ib, shader);
			//renderer.Draw(vaCube3, ib, shader);
		}
		if(bUseFSR || bUseUpscaleShader || bUseFXAA || bUseDenoiseShader)
		{
			fboDownscaled.Unbind();
			lastFBO = &fboDownscaled;
		}
			
		renderer.UpdateViewport((float)windowWidth, (float)windowHeight);

		/* FSR PASS */
		if(bUseFSR)
		{
			/* FSR PASS 0 */
			fboFSRPass1.Bind(8);
			fsrPass0.Bind();
			fboDownscaled.GetTexture().Bind(3);
			fsrPass0.SetUniform1i("Source", 3);
			renderer.Draw(vaUpscale, ibPost, fsrPass0);
			fboFSRPass1.Unbind();
			lastFBO = &fboFSRPass1;

			/* FSR PASS 1 */
			if(bUseFXAA)
			{
				fboNormalScale.Bind(9);
			}

			fsrPass1.Bind();
			fboFSRPass1.GetTexture().Bind(8);
			fsrPass1.SetUniform1i("Source", 8);
			fsrPass1.SetUniform1i("FrameCount", FrameCount);
			renderer.Draw(vaUpscale, ibPost, fsrPass1);

			if(bUseFXAA)
			{
				fboNormalScale.Unbind();
				lastFBO = &fboNormalScale;
			}
		}

		/* UPSCALE PASS */
		if(bUseUpscaleShader && !bUseFSR)
		{
			upscaleShader.Bind();
			fboDownscaled.GetTexture().Bind(3);
			upscaleShader.SetUniform1i("u_Fbo", 3);
			renderer.Draw(vaUpscale, ibPost, upscaleShader);	
		}
	
		/* FXAA PASS */
		if(bUseFXAA)
		{	
			if(bUseDenoiseShader)
			{
				fboFXAAPass.Bind(10);
			}

			fxaaShader.Bind();
			if(bUseFSR)
			{
				fboNormalScale.GetTexture().Bind(9);
			}
			else 
			{
				fboDownscaled.GetTexture().Bind(9);
			}
	
			fxaaShader.SetUniform1i("Source", 9);
			renderer.Draw(vaUpscale, ibPost, fxaaShader);

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
			lastFBO->GetTexture().Bind(10);
			denoiseShader.SetUniform1i("Source", 10);
			renderer.Draw(vaUpscale, ibPost, denoiseShader);
		}

		FrameCount++;

		/* IMGUI SETUP */
		{
			cameraController.m_translation = orthographicCamera.GetPosition();
			cameraController.m_rotationX = orthographicCamera.GetRotationX();
			cameraController.m_rotationY = orthographicCamera.GetRotationY();
			cameraController.m_rotationZ = orthographicCamera.GetRotationZ();
			ImGui::SliderFloat3("Translation", &cameraController.m_translation.x, 0.0f, (float)windowWidth);
			ImGui::SliderFloat("RotationX", &cameraController.m_rotationX, 0.0f, 360.f);
			ImGui::SliderFloat("RotationY", &cameraController.m_rotationY, 0.0f, 360.f);
			ImGui::SliderFloat("RotationZ", &cameraController.m_rotationZ, 0.0f, 360.f);
			ImGui::SliderFloat("RotationSpeed", &cameraController.m_cameraRotationSpeed, 0.0f, 2.f);
			ImGui::SliderFloat("ZoomSpeed", &cameraController.m_cameraZoomSpeed, 0.0f, 2.f);
			ImGui::SliderFloat("MoveSpeed", &cameraController.m_cameraMoveSpeed, 0.0f, 2.f);
			if(ImGui::SliderInt("DownscaleFactor", &downscaleFactor, 1, 16))
			{
				bNeedsReload = true;
			}

			if(ImGui::Checkbox("Use FSR", &bUseFSR) ||
			   ImGui::Checkbox("Use FXAA", &bUseFXAA) ||
			   ImGui::Checkbox("Use upscale shader", &bUseUpscaleShader) || 
			   ImGui::Checkbox("Use denoise shader", &bUseDenoiseShader) || 
			   ImGui::Checkbox("Use Mip Maps", &bUseMipMaps))
			{
				bNeedsReload = true;
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
			orthographicCamera.SetPosition(cameraController.m_translation);
			orthographicCamera.SetRotationX(cameraController.m_rotationX);
			orthographicCamera.SetRotationY(cameraController.m_rotationY);
			orthographicCamera.SetRotationZ(cameraController.m_rotationZ);
		}

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		ImGui::EndFrame();

		/* Swap front and back buffers */
		glfwSwapBuffers(Window::window);

		/* Poll for and process events */
		glfwPollEvents();

		GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
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
	upscaleShader.Unbind();
	fsrPass0.Unbind();
	fsrPass1.Unbind();
	fxaaShader.Unbind();

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	if (bNeedsReload)
	{
		if(bUseFSR)
		{
			bUseUpscaleShader = false;
		}
		downscaleFactor = (bUseFSR || bUseUpscaleShader) ? 3 : 1;
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
