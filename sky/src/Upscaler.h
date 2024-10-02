#pragma once

#include "Shader.h"
#include "Framebuffer.h"
#include <memory>
#include <functional>

class Upscaler
{
public:
	virtual void Prepare(int windowWidth, int windowHeight, int downscaleFactor) = 0;
	virtual Shader* Run1(Framebuffer* FBO, int FrameCount, float DeltaTime) = 0;
	virtual void PostRun1() = 0;
	virtual Shader* Run2(Framebuffer* FBO, int FrameCount, float DeltaTime) = 0;
	virtual void PostRun2() = 0;
	virtual void Unbind() = 0;
	virtual Framebuffer* GetResultFBO() const = 0;

};

