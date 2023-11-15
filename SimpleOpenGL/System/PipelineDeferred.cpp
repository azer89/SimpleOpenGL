#include "PipelineDeferred.h"
#include "AppSettings.h"

#include <glad/glad.h>

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
	float quadVertices[] = {
		// Positions		// Texture Coords
		-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
		-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
		 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
		 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
	};

	unsigned int quadVBO;
	glGenVertexArrays(1, &quadVAO);
	glGenBuffers(1, &quadVBO);
	glBindVertexArray(quadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

	// G-buffer
	glGenFramebuffers(1, &gBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);

	// Position
	glGenTextures(1, &gPosition);
	glBindTexture(GL_TEXTURE_2D, gPosition);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, AppSettings::ScreenWidth, AppSettings::ScreenHeight, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0); // Attach texture to framebuffer

	// Normal
	glGenTextures(1, &gNormal);
	glBindTexture(GL_TEXTURE_2D, gNormal);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, AppSettings::ScreenWidth, AppSettings::ScreenHeight, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0); // Attach texture to framebuffer

	// Color + Specular
	glGenTextures(1, &gAlbedoSpec);
	glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, AppSettings::ScreenWidth, AppSettings::ScreenHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedoSpec, 0); // Attach texture to framebuffer

	unsigned int attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
	glDrawBuffers(3, attachments);

	// Render buffer
	glGenRenderbuffers(1, &rboDepth);
	glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, AppSettings::ScreenWidth, AppSettings::ScreenHeight);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);

	// Check if framebuffer is complete
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cerr << "Framebuffer not complete!" << '\n';
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void PipelineDeferred::StartGeometryPass(const glm::mat4& projection, const glm::mat4& view)
{
	// 1 Geometry pass
	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	gBufferShader->Use();
	gBufferShader->SetMat4("projection", projection);
	gBufferShader->SetMat4("view", view);
}

void PipelineDeferred::EndGeometryPass() 
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void PipelineDeferred::StartLightingPass(const std::vector<Light>& lights, const glm::vec3& cameraPosition)
{
	// 2 Lighting Pass
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	lightingShader->Use();
	lightingShader->SetVec3("viewPos", cameraPosition);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gPosition);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, gNormal);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
	// Send light relevant uniforms
	const float linear = 0.7f;
	const float quadratic = 1.8f;
	lightingShader->SetFloat("linear", linear);
	lightingShader->SetFloat("quadratic", quadratic);
	for (unsigned int i = 0; i < lights.size(); i++)
	{
		lightingShader->SetVec3("lights[" + std::to_string(i) + "].Position", lights[i].Position);
		lightingShader->SetVec3("lights[" + std::to_string(i) + "].Color", lights[i].Color);
	}
	// Render quad
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}

void PipelineDeferred::Blit() 
{
	// 3 Copy content of geometry's depth buffer to default framebuffer's depth buffer
	glBindFramebuffer(GL_READ_FRAMEBUFFER, gBuffer);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); // write to default framebuffer
	// Blit to default framebuffer. Note that this may or may not work as the internal formats of both the FBO and default framebuffer have to match.
	// the internal formats are implementation defined. This works on all of my systems, but if it doesn't on yours you'll likely have to write to the 		
	// depth buffer in another shader stage (or somehow see to match the default framebuffer's internal format with the FBO's internal format).
	glBlitFramebuffer(0, 0, AppSettings::ScreenWidth, AppSettings::ScreenHeight, 0, 0, AppSettings::ScreenWidth, AppSettings::ScreenHeight, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}