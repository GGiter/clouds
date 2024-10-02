#include "SimpleUpscaler.h"
#include "TextureCounter.h"

void SimpleUpscaler::Prepare(int windowWidth, int windowHeight, int downscaleFactor)
{
	upscaleShader = std::make_unique<Shader>("res/shaders/upscale.vert", "res/shaders/upscale.frag");
	upscaleShader->Bind();
	upscaleShader->SetUniform2f("u_Resolution", (float)windowWidth, (float)windowHeight);
	upscaleShader->SetUniform1f("u_DownscaleFactor", (float)downscaleFactor);
}

Shader* SimpleUpscaler::Run1(Framebuffer* FBO, int FrameCount, float DeltaTime)
{
	upscaleShader->Bind();
	FBO->GetTexture().Bind(TextureCounter::GetNextID());
	upscaleShader->SetUniform1i("u_Fbo", FBO->GetTexture().GetSlot());
	return upscaleShader.get();
}

void SimpleUpscaler::PostRun1()
{
}

Shader* SimpleUpscaler::Run2(Framebuffer* FBO, int FrameCount, float DeltaTime)
{
	return nullptr;
}

void SimpleUpscaler::PostRun2()
{
}

void SimpleUpscaler::Unbind()
{
	upscaleShader->Unbind();
}
