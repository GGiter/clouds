#pragma once
#include "Upscaler.h"

class SimpleUpscaler :
    public Upscaler
{
public:
    virtual void Prepare(int windowWidth, int windowHeight, int downscaleFactor);
	virtual Shader* Run1(Framebuffer* FBO, int FrameCount, float DeltaTime);
	virtual void PostRun1();
	virtual Shader* Run2(Framebuffer* FBO, int FrameCount, float DeltaTime);
	virtual void PostRun2();
	virtual void Unbind();
	virtual Framebuffer* GetResultFBO() const { return nullptr; }
protected:
	std::unique_ptr<Shader> upscaleShader;
};

