#include "PipelineDeferred.h"
#include "AppSettings.h"

#include "glad/glad.h"
#include <stdexcept>

PipelineDeferred::PipelineDeferred(
	const char* gBufferVertexShader,
	const char* gBufferFragmentShader,
	const char* lightingVertexShader,
	const char* lightingFragmentShader
)
{
	Init(gBufferVertexShader, gBufferFragmentShader, lightingVertexShader, lightingFragmentShader);
}

void PipelineDeferred::Init(
	const char* gBufferVertexShader,
	const char* gBufferFragmentShader,
	const char* lightingVertexShader,
	const char* lightingFragmentShader
)
{
	// Shaders
	gBufferShader = std::make_unique<Shader>(gBufferVertexShader, gBufferFragmentShader);
	lightingShader = std::make_unique<Shader>(lightingVertexShader, lightingFragmentShader);
	lightingShader->Use();
	lightingShader->SetInt("gPosition", 0);
	lightingShader->SetInt("gNormal", 1);
	lightingShader->SetInt("gAlbedoSpec", 2);

	// Quad
	constexpr float quadVertices[]{
		// Positions		// Texture Coords
		-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
		-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
		 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
		 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
	};

	unsigned int quadVBO;
	glCreateBuffers(1, &quadVBO);
	glNamedBufferStorage(quadVBO, sizeof(quadVertices), &quadVertices, GL_DYNAMIC_STORAGE_BIT);
	glCreateVertexArrays(1, &quadVAO);
	glVertexArrayVertexBuffer(quadVAO, 0, quadVBO, 0, 5 * sizeof(float));

	glEnableVertexArrayAttrib(quadVAO, 0);
	glEnableVertexArrayAttrib(quadVAO, 1);

	glVertexArrayAttribFormat(quadVAO, 0, 3, GL_FLOAT, GL_FALSE, 0);
	glVertexArrayAttribFormat(quadVAO, 1, 2, GL_FLOAT, GL_FALSE, 3 * sizeof(float));

	glVertexArrayAttribBinding(quadVAO, 0, 0);
	glVertexArrayAttribBinding(quadVAO, 1, 0);

	constexpr int numMipmaps = 1;

	// G-buffer
	glCreateFramebuffers(1, &gBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);

	// Position
	glCreateTextures(GL_TEXTURE_2D, 1, &gPosition);
	glTextureParameteri(gPosition, GL_TEXTURE_MAX_LEVEL, numMipmaps - 1);
	glTextureParameteri(gPosition, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureParameteri(gPosition, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTextureStorage2D(gPosition, numMipmaps, GL_RGBA32F, AppSettings::ScreenWidth, AppSettings::ScreenHeight);
	glNamedFramebufferTexture(gBuffer, GL_COLOR_ATTACHMENT0, gPosition, 0);
	
	// Normal
	glCreateTextures(GL_TEXTURE_2D, 1, &gNormal);
	glTextureParameteri(gNormal, GL_TEXTURE_MAX_LEVEL, numMipmaps - 1);
	glTextureParameteri(gNormal, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureParameteri(gNormal, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTextureStorage2D(gNormal, numMipmaps, GL_RGBA32F, AppSettings::ScreenWidth, AppSettings::ScreenHeight);
	glNamedFramebufferTexture(gBuffer, GL_COLOR_ATTACHMENT1, gNormal, 0);
	
	// Color + Specular
	glCreateTextures(GL_TEXTURE_2D, 1, &gAlbedoSpec);
	glTextureParameteri(gAlbedoSpec, GL_TEXTURE_MAX_LEVEL, numMipmaps - 1);
	glTextureParameteri(gAlbedoSpec, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureParameteri(gAlbedoSpec, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTextureStorage2D(gAlbedoSpec, numMipmaps, GL_RGBA8, AppSettings::ScreenWidth, AppSettings::ScreenHeight);
	glNamedFramebufferTexture(gBuffer, GL_COLOR_ATTACHMENT2, gAlbedoSpec, 0);
	
	constexpr uint32_t attachments[3]{ GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
	glNamedFramebufferDrawBuffers(gBuffer, 3, attachments);
	
	// Depth render buffer
	glCreateRenderbuffers(1, &rboDepth);
	glNamedRenderbufferStorage(rboDepth, GL_DEPTH_COMPONENT, AppSettings::ScreenWidth, AppSettings::ScreenHeight);
	glNamedFramebufferRenderbuffer(gBuffer, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);

	// Check if framebuffer is complete
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		throw std::runtime_error("Framebuffer not complete!\n");
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void PipelineDeferred::StartGeometryPass(const glm::mat4& projection, const glm::mat4& view) const
{
	// 1 Geometry pass
	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	gBufferShader->Use();
	gBufferShader->SetMat4("projection", projection);
	gBufferShader->SetMat4("view", view);
}

void PipelineDeferred::EndGeometryPass() const
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void PipelineDeferred::StartLightingPass(const std::vector<Light>& lights, const glm::vec3& cameraPosition) const
{
	// 2 Lighting Pass
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	lightingShader->Use();
	lightingShader->SetVec3("viewPos", cameraPosition);

	glBindTextureUnit(0, gPosition);
	glBindTextureUnit(1, gNormal);
	glBindTextureUnit(2, gAlbedoSpec);

	// Send light relevant uniforms
	constexpr float linear = 0.7f;
	constexpr float quadratic = 1.8f;
	lightingShader->SetFloat("linear", linear);
	lightingShader->SetFloat("quadratic", quadratic);
	for (size_t i = 0; i < lights.size(); ++i)
	{
		lightingShader->SetVec3("lights[" + std::to_string(i) + "].Position", lights[i].Position);
		lightingShader->SetVec3("lights[" + std::to_string(i) + "].Color", lights[i].Color);
	}
	// Render quad
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}

void PipelineDeferred::Blit() const
{
	// 3 Copy content of geometry's depth buffer to default framebuffer's depth buffer
	glBlitNamedFramebuffer(
		gBuffer, // Source
		0, // Destination
		0, 0, AppSettings::ScreenWidth, AppSettings::ScreenHeight, 0, 0, AppSettings::ScreenWidth, AppSettings::ScreenHeight, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}