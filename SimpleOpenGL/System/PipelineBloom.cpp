#include "PipelineBloom.h"
#include "AppSettings.h"

#include <iostream>

PipelineBloom::PipelineBloom(unsigned blurIteration_) :
	blurIteration(blurIteration_)
{
	mainShader = std::make_unique<Shader>("Bloom//first_pass.vertex", "Bloom//first_pass.fragment");
	shaderBlur = std::make_unique<Shader>("Bloom//blur.vertex", "Bloom//blur.fragment");
	shaderFinal = std::make_unique<Shader>("Bloom//final.vertex", "Bloom//final.fragment");

	// Quad
	InitQuad();

	// Set up Bloom pipeline
	const int numMipmaps = 1;

	// Configure (floating point) framebuffers
	glCreateFramebuffers(1, &hdrFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
	// Create 2 floating point color buffers (1 for normal rendering, other for brightness threshold values)
	glCreateTextures(GL_TEXTURE_2D, 2, colorBuffers);
	for (unsigned int i = 0; i < 2; i++)
	{
		glTextureParameteri(colorBuffers[i], GL_TEXTURE_MAX_LEVEL, numMipmaps - 1);
		glTextureParameteri(colorBuffers[i], GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(colorBuffers[i], GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTextureParameteri(colorBuffers[i], GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTextureParameteri(colorBuffers[i], GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTextureStorage2D(colorBuffers[i], numMipmaps, GL_RGBA16F, AppSettings::ScreenWidth, AppSettings::ScreenHeight);
		// Attach texture to framebuffer
		glNamedFramebufferTexture(hdrFBO, GL_COLOR_ATTACHMENT0 + i, colorBuffers[i], 0);
	}

	// Tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
	unsigned int attachments[2]{ GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	glNamedFramebufferDrawBuffers(hdrFBO, 2, attachments);
	if (glCheckNamedFramebufferStatus(hdrFBO, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cerr << "Framebuffer not complete!\n";
	}

	// Create and attach depth buffer (renderbuffer)
	unsigned int rboDepth;
	glCreateRenderbuffers(1, &rboDepth);
	glNamedRenderbufferStorage(rboDepth, GL_DEPTH_COMPONENT, AppSettings::ScreenWidth, AppSettings::ScreenHeight);
	glNamedFramebufferRenderbuffer(hdrFBO, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Ping-pong-framebuffer for blurring
	glCreateFramebuffers(2, pingpongFBO);
	glCreateTextures(GL_TEXTURE_2D, 2, pingpongColorbuffers);
	for (unsigned int i = 0; i < 2; i++)
	{
		glTextureParameteri(pingpongColorbuffers[i], GL_TEXTURE_MAX_LEVEL, numMipmaps - 1);
		glTextureParameteri(pingpongColorbuffers[i], GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(pingpongColorbuffers[i], GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTextureParameteri(pingpongColorbuffers[i], GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // we clamp to the edge as the blur filter would otherwise sample repeated texture values!
		glTextureParameteri(pingpongColorbuffers[i], GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTextureStorage2D(pingpongColorbuffers[i], numMipmaps, GL_RGBA16F, AppSettings::ScreenWidth, AppSettings::ScreenHeight);

		glNamedFramebufferTexture(pingpongFBO[i], GL_COLOR_ATTACHMENT0, pingpongColorbuffers[i], 0);

		// Check if framebuffers are complete
		if (glCheckNamedFramebufferStatus(pingpongFBO[i], GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			std::cerr << "Framebuffer not complete!\n";
		}
	}

	// Shader
	shaderBlur->Use();
	shaderBlur->SetInt("image", 0);
	shaderFinal->Use();
	shaderFinal->SetInt("scene", 0);
	shaderFinal->SetInt("bloomBlur", 1);
}

// 1
void PipelineBloom::StartFirstPass(
	const glm::mat4& projection,
	const glm::mat4& view,
	const glm::vec3& cameraPosition,
	const glm::vec3& lightPosition
)
{
	// First pass
	glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	mainShader->Use();
	mainShader->SetMat4("projection", projection);
	mainShader->SetMat4("view", view);
	mainShader->SetVec3("viewPos", cameraPosition);
	mainShader->SetVec3("lightPos", lightPosition);
}

// 2
void PipelineBloom::EndFirstPass()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// 3
void PipelineBloom::StartBlurPass()
{
	// Blur pass
	horizontal = true;
	shaderBlur->Use();
	for (unsigned int i = 0; i < blurIteration; i++)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal]);
		shaderBlur->SetInt("horizontal", horizontal);
		glBindTextureUnit(0,
			i == 0 ?
			colorBuffers[1] :
			pingpongColorbuffers[!horizontal]);  // bind texture of other framebuffer (or scene if first iteration)
		RenderQuad();
		horizontal = !horizontal;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// 4
void PipelineBloom::RenderComposite()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	shaderFinal->Use();
	glBindTextureUnit(0, colorBuffers[0]);
	glBindTextureUnit(1, pingpongColorbuffers[!horizontal]);
	RenderQuad();
}

void PipelineBloom::InitQuad()
{
	// Quad
	constexpr float quadVertices[]{
		// Positions		// Texture Coords
		-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
		-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
		 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
		 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
	};

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
}

void PipelineBloom::RenderQuad()
{
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}