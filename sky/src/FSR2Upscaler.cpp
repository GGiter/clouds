#include "FSR2Upscaler.h"
#include "TextureCounter.h"

void FSR2Upscaler::Prepare(int windowWidth, int windowHeight, int downscaleFactor)
{
	renderWidth = windowWidth / downscaleFactor;
	renderHeight = windowHeight / downscaleFactor;
		FfxFsr2ContextDescription contextDesc{
		.flags = FFX_FSR2_ENABLE_DEBUG_CHECKING | FFX_FSR2_ENABLE_AUTO_EXPOSURE | FFX_FSR2_ENABLE_HIGH_DYNAMIC_RANGE |
				FFX_FSR2_ALLOW_NULL_DEVICE_AND_COMMAND_LIST,
		.maxRenderSize = {(uint32_t)renderWidth, (uint32_t)renderHeight},
		.displaySize = {(uint32_t)windowWidth, (uint32_t)windowHeight},
		.fpMessage =
		[](FfxFsr2MsgType type, const wchar_t* message)
		{
		char cstr[256] = {};
		wcstombs_s(nullptr, cstr, sizeof(cstr), message, sizeof(cstr));
		cstr[255] = '\0';
		printf("FSR 2 message (type=%d): %s\n", type, cstr);
		},
	};
	fsr2ScratchMemory = std::make_unique<char[]>(ffxFsr2GetScratchMemorySizeGL());
	ffxFsr2GetInterfaceGL(&contextDesc.callbacks, fsr2ScratchMemory.get(), ffxFsr2GetScratchMemorySizeGL(), glfwGetProcAddress);
	ffxFsr2ContextCreate(&fsr2Context, &contextDesc);

	fboUpscaled = std::make_unique<Framebuffer>(windowWidth, windowHeight);
	this->windowWidth = windowWidth;
	this->windowHeight = windowHeight;
}

Shader* FSR2Upscaler::Run1(Framebuffer* FBO, int FrameCount, float DeltaTime)
{
	//FSR 2
	float jitterX{};
    float jitterY{};
    ffxFsr2GetJitterOffset(&jitterX, &jitterY, FrameCount, ffxFsr2GetJitterPhaseCount(renderWidth, windowWidth));

	FBO->Bind(TextureCounter::GetNextID());
	fboUpscaled->Bind(TextureCounter::GetNextID());
	DepthTexture->Bind(TextureCounter::GetNextID());
	MotionVectorsTexture->Bind(TextureCounter::GetNextID());
    FfxFsr2DispatchDescription dispatchDesc{
        .color = ffxGetTextureResourceGL(FBO->GetTexture().GetID(), renderWidth, renderHeight, GL_RGBA32F),
		.depth = ffxGetTextureResourceGL(DepthTexture->GetID(), renderWidth, renderHeight, GL_DEPTH_COMPONENT32F),
        .motionVectors = ffxGetTextureResourceGL(MotionVectorsTexture->GetID(), renderWidth, renderHeight, GL_RG16F),
        .exposure = {},
        .reactive = {},
        .transparencyAndComposition = {},
        .output =
        ffxGetTextureResourceGL(fboUpscaled->GetTexture().GetID(), windowWidth, windowHeight, GL_RGBA32F),
        .jitterOffset = {0, 0},
        .motionVectorScale = {float(renderWidth), float(renderHeight)},
        .renderSize = {renderWidth, renderHeight},
        .enableSharpening = fsr2Sharpness != 0,
        .sharpness = fsr2Sharpness,
        .frameTimeDelta = static_cast<float>(DeltaTime * 1000.0),
        .preExposure = 1,
        .reset = true,
		.cameraNear = cameraNear,
        .cameraFar = cameraFar,
        .cameraFovAngleVertical = glm::radians(90.f),
        .viewSpaceToMetersFactor = 1,
        .deviceDepthNegativeOneToOne = false,
    };
	if (auto err = ffxFsr2ContextDispatch(&fsr2Context, &dispatchDesc); err != FFX_OK)
    {
        printf("FSR 2 error: %d\n", err);
    }
	FBO->Unbind();

	return nullptr;
}

void FSR2Upscaler::PostRun1()
{
}

Shader* FSR2Upscaler::Run2(Framebuffer* FBO, int FrameCount, float DeltaTime)
{
	return nullptr;
}

void FSR2Upscaler::PostRun2()
{
}

void FSR2Upscaler::Unbind()
{
	ffxFsr2ContextDestroy(&fsr2Context);
}

Framebuffer* FSR2Upscaler::GetResultFBO() const
{
	return fboUpscaled.get();
}
