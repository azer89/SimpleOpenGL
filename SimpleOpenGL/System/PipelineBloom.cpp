#include "PipelineBloom.h"
#include "AppSettings.h"

#include <iostream>

PipelineBloom::PipelineBloom(uint32_t blurIteration_) :
	blurIteration_(blurIteration_)
{
	mainShader_ = std::make_unique<Shader>("Bloom//first_pass.vertex", "Bloom//first_pass.fragment");
	shaderBlur_ = std::make_unique<Shader>("Bloom//blur.vertex", "Bloom//blur.fragment");
	shaderFinal_ = std::make_unique<Shader>("Bloom//final.vertex", "Bloom//final.fragment");

	// Quad
	InitQuad();

	// Set up Bloom pipeline
	constexpr int numMipmaps = 1;

	// Configure (floating point) framebuffers
	glCreateFramebuffers(1, &hdrFBO_);
	glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO_);
	// Create 2 floating point color buffers (1 for normal rendering, other for brightness threshold values)
	glCreateTextures(GL_TEXTURE_2D, 2, colorBuffers_.data());
	for (uint32_t i = 0; i < 2; i++)
	{
		glTextureParameteri(colorBuffers_[i], GL_TEXTURE_MAX_LEVEL, numMipmaps - 1);
		glTextureParameteri(colorBuffers_[i], GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(colorBuffers_[i], GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTextureParameteri(colorBuffers_[i], GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTextureParameteri(colorBuffers_[i], GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTextureStorage2D(colorBuffers_[i], numMipmaps, GL_RGBA16F, AppSettings::ScreenWidth, AppSettings::ScreenHeight);
		// Attach texture to framebuffer
		glNamedFramebufferTexture(hdrFBO_, GL_COLOR_ATTACHMENT0 + i, colorBuffers_[i], 0);
	}

	// Tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
	constexpr uint32_t attachments[2]{ GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	glNamedFramebufferDrawBuffers(hdrFBO_, 2, attachments);
	if (glCheckNamedFramebufferStatus(hdrFBO_, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cerr << "Framebuffer not complete!\n";
	}

	// Create and attach depth buffer (renderbuffer)
	uint32_t rboDepth{};
	glCreateRenderbuffers(1, &rboDepth);
	glNamedRenderbufferStorage(rboDepth, GL_DEPTH_COMPONENT, AppSettings::ScreenWidth, AppSettings::ScreenHeight);
	glNamedFramebufferRenderbuffer(hdrFBO_, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Ping-pong-framebuffer for blurring
	glCreateFramebuffers(2, pingpongFBO_.data());
	glCreateTextures(GL_TEXTURE_2D, 2, pingpongColorbuffers_.data());
	for (uint32_t i = 0; i < 2; i++)
	{
		glTextureParameteri(pingpongColorbuffers_[i], GL_TEXTURE_MAX_LEVEL, numMipmaps - 1);
		glTextureParameteri(pingpongColorbuffers_[i], GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(pingpongColorbuffers_[i], GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTextureParameteri(pingpongColorbuffers_[i], GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // we clamp to the edge as the blur filter would otherwise sample repeated texture values!
		glTextureParameteri(pingpongColorbuffers_[i], GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTextureStorage2D(pingpongColorbuffers_[i], numMipmaps, GL_RGBA16F, AppSettings::ScreenWidth, AppSettings::ScreenHeight);

		glNamedFramebufferTexture(pingpongFBO_[i], GL_COLOR_ATTACHMENT0, pingpongColorbuffers_[i], 0);

		// Check if framebuffers are complete
		if (glCheckNamedFramebufferStatus(pingpongFBO_[i], GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			std::cerr << "Framebuffer not complete!\n";
		}
	}

	// Shader
	shaderBlur_->Use();
	shaderBlur_->SetInt("image", 0);
	shaderFinal_->Use();
	shaderFinal_->SetInt("scene", 0);
	shaderFinal_->SetInt("bloomBlur", 1);
}

// 1
void PipelineBloom::StartFirstPass(
	const glm::mat4& projection,
	const glm::mat4& view,
	const glm::vec3& cameraPosition,
	const glm::vec3& lightPosition
) const
{
	// First pass
	glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO_);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	mainShader_->Use();
	mainShader_->SetMat4("projection", projection);
	mainShader_->SetMat4("view", view);
	mainShader_->SetVec3("viewPos", cameraPosition);
	mainShader_->SetVec3("lightPos", lightPosition);
}

// 2
void PipelineBloom::EndFirstPass() const
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// 3
void PipelineBloom::StartBlurPass()
{
	// Blur pass
	horizontal_ = true;
	shaderBlur_->Use();
	for (uint32_t i = 0; i < blurIteration_; i++)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO_[horizontal_]);
		shaderBlur_->SetInt("horizontal", horizontal_);
		glBindTextureUnit(0,
			i == 0 ?
			colorBuffers_[1] :
			pingpongColorbuffers_[!horizontal_]);  // bind texture of other framebuffer (or scene if first iteration)
		RenderQuad();
		horizontal_ = !horizontal_;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// 4
void PipelineBloom::RenderComposite() const
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	shaderFinal_->Use();
	glBindTextureUnit(0, colorBuffers_[0]);
	glBindTextureUnit(1, pingpongColorbuffers_[!horizontal_]);
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

	glCreateBuffers(1, &quadVBO_);
	glNamedBufferStorage(quadVBO_, sizeof(quadVertices), &quadVertices, GL_DYNAMIC_STORAGE_BIT);
	glCreateVertexArrays(1, &quadVAO_);
	glVertexArrayVertexBuffer(quadVAO_, 0, quadVBO_, 0, 5 * sizeof(float));

	glEnableVertexArrayAttrib(quadVAO_, 0);
	glEnableVertexArrayAttrib(quadVAO_, 1);

	glVertexArrayAttribFormat(quadVAO_, 0, 3, GL_FLOAT, GL_FALSE, 0);
	glVertexArrayAttribFormat(quadVAO_, 1, 2, GL_FLOAT, GL_FALSE, 3 * sizeof(float));

	glVertexArrayAttribBinding(quadVAO_, 0, 0);
	glVertexArrayAttribBinding(quadVAO_, 1, 0);
}

void PipelineBloom::RenderQuad() const
{
	glBindVertexArray(quadVAO_);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}