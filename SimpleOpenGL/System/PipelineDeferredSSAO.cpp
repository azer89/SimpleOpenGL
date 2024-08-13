#include "PipelineDeferredSSAO.h"
#include "AppSettings.h"
#include "Utility.h"

#include <vector>
#include <random>

PipelineDeferredSSAO::PipelineDeferredSSAO(
	const char* geomVertexShader,
	const char* geomFragmentShader,
	const char* lightVertexShader,
	const char* lightFragmentShader,
	const char* ssaoVertexShader,
	const char* ssaoFragmentShader,
	const char* blurVertexShader,
	const char* blurFragmentShader,
	int kernelSize,
	int noiseSize
)
{
	Init(
		geomVertexShader,
		geomFragmentShader,
		lightVertexShader,
		lightFragmentShader,
		ssaoVertexShader,
		ssaoFragmentShader,
		blurVertexShader,
		blurFragmentShader,
		kernelSize,
		noiseSize
	);
}

void PipelineDeferredSSAO::Init(
	const char* geomVertexShader,
	const char* geomFragmentShader,
	const char* lightVertexShader,
	const char* lightFragmentShader,
	const char* ssaoVertexShader,
	const char* ssaoFragmentShader,
	const char* blurVertexShader,
	const char* blurFragmentShader,
	int kernelSize,
	int noiseSize
)
{
	shaderGeometry = std::make_unique<Shader>(geomVertexShader, geomFragmentShader);
	shaderLighting = std::make_unique<Shader>(lightVertexShader, lightFragmentShader);
	shaderSSAO = std::make_unique<Shader>(ssaoVertexShader, ssaoFragmentShader);
	shaderBlur = std::make_unique<Shader>(blurVertexShader, blurFragmentShader);

	// G Buffer
	glCreateFramebuffers(1, &gBufferFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, gBufferFBO); // Needed because we are not fully DSA
	
	constexpr uint32_t numMipmaps = 1;
	
	// Position
	glCreateTextures(GL_TEXTURE_2D, 1, &gPositionTexture);
	glTextureParameteri(gPositionTexture, GL_TEXTURE_MAX_LEVEL, numMipmaps - 1);
	glTextureParameteri(gPositionTexture, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureParameteri(gPositionTexture, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTextureParameteri(gPositionTexture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(gPositionTexture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTextureStorage2D(gPositionTexture, numMipmaps, GL_RGBA16F, AppSettings::ScreenWidth, AppSettings::ScreenHeight);
	glNamedFramebufferTexture(gBufferFBO, GL_COLOR_ATTACHMENT0, gPositionTexture, 0);
	
	// Normal
	glCreateTextures(GL_TEXTURE_2D, 1, &gNormalTexture);
	glTextureParameteri(gNormalTexture, GL_TEXTURE_MAX_LEVEL, numMipmaps - 1);
	glTextureParameteri(gNormalTexture, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureParameteri(gNormalTexture, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTextureParameteri(gNormalTexture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(gNormalTexture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTextureStorage2D(gNormalTexture, numMipmaps, GL_RGBA16F, AppSettings::ScreenWidth, AppSettings::ScreenHeight);
	glNamedFramebufferTexture(gBufferFBO, GL_COLOR_ATTACHMENT1, gNormalTexture, 0);
	
	// Color + Specular
	glCreateTextures(GL_TEXTURE_2D, 1, &gAlbedoTexture);
	glTextureParameteri(gAlbedoTexture, GL_TEXTURE_MAX_LEVEL, numMipmaps - 1);
	glTextureParameteri(gAlbedoTexture, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureParameteri(gAlbedoTexture, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTextureParameteri(gAlbedoTexture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(gAlbedoTexture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTextureStorage2D(gAlbedoTexture, numMipmaps, GL_RGBA8, AppSettings::ScreenWidth, AppSettings::ScreenHeight);
	glNamedFramebufferTexture(gBufferFBO, GL_COLOR_ATTACHMENT2, gAlbedoTexture, 0);
	
	// Attachments
	constexpr uint32_t attachments[3]{ GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
	glNamedFramebufferDrawBuffers(gBufferFBO, 3, attachments);

	// Depth render buffer
	glGenRenderbuffers(1, &depthRBO);
	glBindRenderbuffer(GL_RENDERBUFFER, depthRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, AppSettings::ScreenWidth, AppSettings::ScreenHeight);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRBO);

	// Check frame buffer
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		throw std::runtime_error("Framebuffer not complete!\n");
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// SSAO Frame buffer
	glGenFramebuffers(1, &ssaoFBO);

	glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);

	// SSAO color buffer
	glGenTextures(1, &ssaoColorTexture);
	glBindTexture(GL_TEXTURE_2D, ssaoColorTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, AppSettings::ScreenWidth, AppSettings::ScreenHeight, 0, GL_RED, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoColorTexture, 0);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		throw std::runtime_error("SSAO Framebuffer not complete!\n");
	}

	// Blur frame buffer
	glGenFramebuffers(1, &ssaoBlurFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurFBO);
	glGenTextures(1, &ssaoBlurTexture);
	glBindTexture(GL_TEXTURE_2D, ssaoBlurTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, AppSettings::ScreenWidth, AppSettings::ScreenHeight, 0, GL_RED, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoBlurTexture, 0);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		throw std::runtime_error("SSAO Blur Framebuffer not complete!\n");
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Sample kernel
	const float kernelSizeF = static_cast<float>(kernelSize);
	for (int i = 0; i < kernelSize; ++i)
	{
		glm::vec3 sample(Utility::RandomNumber<float>(-1.0f, 1.0f),
			Utility::RandomNumber<float>(-1.0f, 1.0f),
			Utility::RandomNumber<float>()); // Half hemisphere
		sample = glm::normalize(sample);
		sample *= Utility::RandomNumber<float>();
		float scale = static_cast<float>(i) / kernelSizeF;

		// Scale samples s.t. they're more aligned to center of kernel
		scale = Utility::Lerp<float>(0.1f, 1.0f, scale * scale);
		sample *= scale;
		ssaoKernel.push_back(sample);
	}

	// Noise texture
	const int noiseSizeSq = noiseSize * noiseSize;
	std::vector<glm::vec3> ssaoNoise(noiseSizeSq);
	for (int i = 0; i < noiseSizeSq; ++i)
	{
		glm::vec3 noise(
			Utility::RandomNumber<float>() * 2.0 - 1.0,
			Utility::RandomNumber<float>() * 2.0 - 1.0,
			0.0f);
		ssaoNoise[i] = noise;
	}

	glCreateTextures(GL_TEXTURE_2D, 1, &noiseTexture);
	glTextureParameteri(noiseTexture, GL_TEXTURE_MAX_LEVEL, numMipmaps - 1);
	glTextureParameteri(noiseTexture, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureParameteri(noiseTexture, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTextureParameteri(noiseTexture, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTextureParameteri(noiseTexture, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTextureStorage2D(noiseTexture, numMipmaps, GL_RGB32F, noiseSize, noiseSize);
	glTextureSubImage2D(noiseTexture, 0, 0, 0, noiseSize, noiseSize, GL_RGB, GL_FLOAT, &ssaoNoise[0]);
	
	// Shader configuration
	shaderLighting->Use();
	shaderLighting->SetInt("gPosition", 0);
	shaderLighting->SetInt("gNormal", 1);
	shaderLighting->SetInt("gAlbedo", 2);
	shaderLighting->SetInt("ssao", 3);

	shaderSSAO->Use();
	shaderSSAO->SetInt("gPosition", 0);
	shaderSSAO->SetInt("gNormal", 1);
	shaderSSAO->SetInt("texNoise", 2);
	shaderSSAO->SetFloat("screen_width", static_cast<float>(AppSettings::ScreenWidth));
	shaderSSAO->SetFloat("screen_height", static_cast<float>(AppSettings::ScreenHeight));
	shaderSSAO->SetFloat("noise_size", static_cast<float>(noiseSize));

	shaderBlur->Use();
	shaderBlur->SetInt("ssaoInput", 0);

	// Plane
	constexpr float quadVertices[]{
		// Positions		// Texture Coords
		-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
		-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
		 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
		 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
	};
	unsigned int quadVBO = 0;
	glGenVertexArrays(1, &quadVAO);
	glGenBuffers(1, &quadVBO);
	glBindVertexArray(quadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
}

void PipelineDeferredSSAO::StartGeometryPass(const glm::mat4& projection, const glm::mat4& view)
{
	// 1 Geometry pass: render scene's geometry/color data into G buffer
	glBindFramebuffer(GL_FRAMEBUFFER, gBufferFBO);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	const glm::mat4 model = glm::mat4(1.0f);
	shaderGeometry->Use();
	shaderGeometry->SetMat4("projection", projection);
	shaderGeometry->SetMat4("view", view);
	shaderGeometry->SetMat4("model", model);
	shaderGeometry->SetFloat("invertedNormals", 1.0f);
}

void PipelineDeferredSSAO::EndGeometryPass()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void PipelineDeferredSSAO::StartSSAOPass
(
	const glm::mat4& projection,
	int kernelSize,
	float radius,
	float bias
)
{
	glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);
	glClear(GL_COLOR_BUFFER_BIT);
	shaderSSAO->Use();
	shaderSSAO->SetInt("kernelSize", kernelSize);
	shaderSSAO->SetFloat("radius", radius);
	shaderSSAO->SetFloat("bias", bias);
	// Send kernel + rotation 
	for (int i = 0; i < kernelSize; ++i)
	{
		shaderSSAO->SetVec3("samples[" + std::to_string(i) + "]", ssaoKernel[i]);
	}
	shaderSSAO->SetMat4("projection", projection);
	glBindTextureUnit(0, gPositionTexture);
	glBindTextureUnit(1, gNormalTexture);
	glBindTextureUnit(2, noiseTexture);
	RenderQuad();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void PipelineDeferredSSAO::StartBlurPass()
{
	glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurFBO);
	glClear(GL_COLOR_BUFFER_BIT);
	shaderBlur->Use();
	glBindTextureUnit(0, ssaoColorTexture);
	RenderQuad();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void PipelineDeferredSSAO::StartLightingPass(const std::vector<Light>& lights, 
	const glm::mat4& cameraview,
	const glm::vec3& cameraPosition)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	shaderLighting->Use();
	for (unsigned int i = 0; i < lights.size(); ++i)
	{
		glm::vec3 lightPosView = glm::vec3(cameraview * glm::vec4(lights[i].Position, 1.0));
		shaderLighting->SetVec3("lights[" + std::to_string(i) + "].Position", lightPosView);
		shaderLighting->SetVec3("lights[" + std::to_string(i) + "].Color", lights[i].Color);
	}

	// Attenuation parameters
	constexpr float linear = 2.9f;
	constexpr float quadratic = 3.8f;
	shaderLighting->SetFloat("linear", linear);
	shaderLighting->SetFloat("quadratic", quadratic);
	shaderLighting->SetVec3("viewPos", cameraPosition);
	glBindTextureUnit(0, gPositionTexture);
	glBindTextureUnit(1, gNormalTexture);
	glBindTextureUnit(2, gAlbedoTexture);
	glBindTextureUnit(3, ssaoBlurTexture);
	RenderQuad();
}

void PipelineDeferredSSAO::Blit()
{
	glBindFramebuffer(GL_READ_FRAMEBUFFER, gBufferFBO);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glBlitFramebuffer(0, 0, AppSettings::ScreenWidth, AppSettings::ScreenHeight, 0, 0, AppSettings::ScreenWidth, AppSettings::ScreenHeight, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void PipelineDeferredSSAO::RenderQuad()
{
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}