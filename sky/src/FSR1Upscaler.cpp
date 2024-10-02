#include "FSR1Upscaler.h"
#include "Texture2D.h"
#include "TextureCounter.h"

void FSR1Upscaler::Prepare(int windowWidth, int windowHeight, int downscaleFactor)
{
	fsrPass0 = std::make_unique<Shader>("res/shaders/fsr-pass0.vert", "res/shaders/fsr-pass0.frag");
	fsrPass0->Bind();
	fsrPass0->SetUniform2f("SourceSize", (float)windowWidth / downscaleFactor, (float)windowHeight / downscaleFactor);
	fsrPass0->SetUniform2f("OutputSize", (float)windowWidth, (float)windowHeight);

	fsrPass1 = std::make_unique<Shader>("res/shaders/fsr-pass1.vert", "res/shaders/fsr-pass1.frag");
	fsrPass1->Bind();
	fsrPass1->SetUniform2f("OutputSize", (float)windowWidth, (float)windowHeight);
	Pass1FBO = std::make_unique<Framebuffer>(windowWidth, windowHeight);
	ResultFBO = std::make_unique<Framebuffer>(windowWidth, windowHeight);
}

Shader* FSR1Upscaler::Run1(Framebuffer* FBO, int FrameCount, float DeltaTime)
{
	Pass1FBO->Bind(TextureCounter::GetNextID());
	fsrPass0->Bind();
	FBO->GetTexture().Bind(TextureCounter::GetNextID());
	fsrPass0->SetUniform1i("Source",FBO->GetTexture().GetSlot());
	return fsrPass0.get();
}

void FSR1Upscaler::PostRun1()
{
	Pass1FBO->Unbind();
}

Shader* FSR1Upscaler::Run2(Framebuffer* FBO, int FrameCount, float DeltaTime)
{
	ResultFBO->Bind(TextureCounter::GetNextID());
	fsrPass1->Bind();
	Pass1FBO->GetTexture().Bind(TextureCounter::GetNextID());
	fsrPass1->SetUniform1i("Source", Pass1FBO->GetTexture().GetSlot());
	fsrPass1->SetUniform1i("FrameCount", FrameCount);
	return fsrPass1.get();
}

void FSR1Upscaler::PostRun2()
{
	ResultFBO->Unbind();
}

void FSR1Upscaler::Unbind()
{
	fsrPass0->Unbind();
	fsrPass1->Unbind();
}

Framebuffer* FSR1Upscaler::GetResultFBO() const
{
	return ResultFBO.get();
}
