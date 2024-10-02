#pragma once
#include "Upscaler.h"
#include <ffx_fsr2.h>
#include <GLAD/glad.h>
#include <GLFW/glfw3.h>
#include <gl/ffx_fsr2_gl.h>

class FSR2Upscaler :
    public Upscaler
{
public:
	virtual void Prepare(int windowWidth, int windowHeight, int downscaleFactor);
	virtual Shader* Run1(Framebuffer* FBO, int FrameCount, float DeltaTime);
	virtual void PostRun1();
	virtual Shader* Run2(Framebuffer* FBO, int FrameCount, float DeltaTime);
	virtual void PostRun2();
	virtual void Unbind();
	virtual Framebuffer* GetResultFBO() const;
	Texture2D* MotionVectorsTexture = nullptr;
	Texture2D* DepthTexture = nullptr;
protected:
	FfxFsr2Context fsr2Context;
	std::unique_ptr<char[]> fsr2ScratchMemory;
	std::unique_ptr<Framebuffer> fboUpscaled;
	uint32_t renderWidth, renderHeight;
	int windowWidth, windowHeight;
	float fsr2Sharpness = 0.5;
	float cameraNear = 0.1f;
	float cameraFar = 100.0f;
};
