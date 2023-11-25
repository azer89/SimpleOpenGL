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
	const char* blurFragmentShader
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
		blurFragmentShader
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
	const char* blurFragmentShader
)
{
	shaderGeometry = std::make_unique<Shader>(geomVertexShader, geomFragmentShader);
	shaderLighting = std::make_unique<Shader>(lightVertexShader, lightFragmentShader);
	shaderSSAO = std::make_unique<Shader>(ssaoVertexShader, ssaoFragmentShader);
	shaderBlur = std::make_unique<Shader>(blurVertexShader, blurFragmentShader);

	// G Buffer
	glGenFramebuffers(1, &gBufferFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, gBufferFBO);

	// Position
	glGenTextures(1, &gPositionTexture);
	glBindTexture(GL_TEXTURE_2D, gPositionTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, AppSettings::ScreenWidth, AppSettings::ScreenHeight, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPositionTexture, 0);

	// Normal
	glGenTextures(1, &gNormalTexture);
	glBindTexture(GL_TEXTURE_2D, gNormalTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, AppSettings::ScreenWidth, AppSettings::ScreenHeight, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormalTexture, 0);

	// Color + Specular
	glGenTextures(1, &gAlbedoTexture);
	glBindTexture(GL_TEXTURE_2D, gAlbedoTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, AppSettings::ScreenWidth, AppSettings::ScreenHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedoTexture, 0);

	// Attachments
	unsigned int attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
	glDrawBuffers(3, attachments);

	// Depth render buffer
	glGenRenderbuffers(1, &depthRBO);
	glBindRenderbuffer(GL_RENDERBUFFER, depthRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, AppSettings::ScreenWidth, AppSettings::ScreenHeight);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRBO);

	// Check frame buffer
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cerr << "Framebuffer not complete!" << std::endl;
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
		std::cerr << "SSAO Framebuffer not complete!" << std::endl;
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
		std::cerr << "SSAO Blur Framebuffer not complete!" << std::endl;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Sample kernel
	for (unsigned int i = 0; i < 64; ++i)
	{
		glm::vec3 sample(Utility::RandomNumber<float>(-1.0f, 1.0f),
			Utility::RandomNumber<float>(-1.0f, 1.0f),
			Utility::RandomNumber<float>()); // Half hemisphere
		sample = glm::normalize(sample);
		sample *= Utility::RandomNumber<float>();
		float scale = float(i) / 64.0f;

		// Scale samples s.t. they're more aligned to center of kernel
		scale = Utility::Lerp<float>(0.1f, 1.0f, scale * scale);
		sample *= scale;
		ssaoKernel.push_back(sample);
	}

	// Noise texture
	std::vector<glm::vec3> ssaoNoise;
	for (unsigned int i = 0; i < 16; ++i)
	{
		glm::vec3 noise(
			Utility::RandomNumber<float>() * 2.0 - 1.0,
			Utility::RandomNumber<float>() * 2.0 - 1.0, 0.0f); // rotate around z-axis (in tangent space)
		ssaoNoise.push_back(noise);
	}
	glGenTextures(1, &noiseTexture);
	glBindTexture(GL_TEXTURE_2D, noiseTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 4, 4, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

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

	shaderBlur->Use();
	shaderBlur->SetInt("ssaoInput", 0);

	// Plane
	constexpr float quadVertices[] = {
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
	glm::mat4 model = glm::mat4(1.0f);
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

void PipelineDeferredSSAO::StartSSAOPass(const glm::mat4& projection)
{
	glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);
	glClear(GL_COLOR_BUFFER_BIT);
	shaderSSAO->Use();
	// Send kernel + rotation 
	for (unsigned int i = 0; i < 64; ++i)
	{
		shaderSSAO->SetVec3("samples[" + std::to_string(i) + "]", ssaoKernel[i]);
	}
	shaderSSAO->SetMat4("projection", projection);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gPositionTexture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, gNormalTexture);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, noiseTexture);
	RenderQuad();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void PipelineDeferredSSAO::StartBlurPass()
{
	glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurFBO);
	glClear(GL_COLOR_BUFFER_BIT);
	shaderBlur->Use();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, ssaoColorTexture);
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
	const float linear = 2.9f;
	const float quadratic = 3.8f;
	shaderLighting->SetFloat("linear", linear);
	shaderLighting->SetFloat("quadratic", quadratic);
	shaderLighting->SetVec3("viewPos", cameraPosition);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gPositionTexture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, gNormalTexture);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, gAlbedoTexture);
	glActiveTexture(GL_TEXTURE3); // add extra SSAO texture to lighting pass
	glBindTexture(GL_TEXTURE_2D, ssaoBlurTexture);
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