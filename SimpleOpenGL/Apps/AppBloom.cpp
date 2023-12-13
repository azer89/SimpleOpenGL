#include "AppBloom.h"
#include "AppSettings.h"
#include "Shader.h"
#include "Model.h"
#include "Shape.h"
#include "Light.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include <iostream>

void RenderQuad();

int AppBloom::MainLoop()
{
	// Configure global opengl state
	glEnable(GL_DEPTH_TEST);

	Cube cube;

	Shader mainShader("Bloom//first_pass.vertex", "Bloom//first_pass.fragment");
	Shader shaderBlur("Bloom//blur.vertex", "Bloom//blur.fragment");
	Shader shaderFinal("Bloom//final.vertex", "Bloom//final.fragment");

	Shader lightShader("Misc//light_sphere.vertex", "Misc//light_sphere.fragment");
	Light light(glm::vec3(0.0f, 0.5f, 5.0f), glm::vec3(1.f));

	Model obj(AppSettings::ModelFolder + "Zaku//scene.gltf");
	auto modelRotation = acos(-1.f);
	mainShader.Use();

	// Set up Bloom pipeline
	const int numMipmaps = 1;
	
	// Configure (floating point) framebuffers
	unsigned int hdrFBO;
	glCreateFramebuffers(1, &hdrFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
	// Create 2 floating point color buffers (1 for normal rendering, other for brightness threshold values)
	unsigned int colorBuffers[2];
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
	unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
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
	unsigned int pingpongFBO[2];
	unsigned int pingpongColorbuffers[2];
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
	shaderBlur.Use();
	shaderBlur.SetInt("image", 0);
	shaderFinal.Use();
	shaderFinal.SetInt("scene", 0);
	shaderFinal.SetInt("bloomBlur", 1);

	// Parameters
	float exposure = 1.0f;
	int blurIter = 200;

	// Render loop
	while (!GLFWWindowShouldClose())
	{
		ProcessLoop(
			glm::vec4(0.0f, 0.0f, 0.0f, 1.0f),
			GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT
		);

		auto projection = camera->GetProjectionMatrix();
		auto view = camera->GetViewMatrix();

		// First pass
		glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		mainShader.Use();
		mainShader.SetMat4("projection", projection);
		mainShader.SetMat4("view", view);
		mainShader.SetVec3("viewPos", camera->Position);
		mainShader.SetVec3("lightPos", light.Position);
		
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
		model = glm::rotate(model, modelRotation, glm::vec3(0.0f, 1.0f, 0.0f));
		mainShader.SetMat4("model", model);
		obj.Draw(mainShader);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// Blur pass
		bool horizontal = true;
		shaderBlur.Use();
		for (unsigned int i = 0; i < blurIter; i++)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal]);
			shaderBlur.SetInt("horizontal", horizontal);
			glBindTextureUnit(0, 
				i == 0 ? 
				colorBuffers[1] : 
				pingpongColorbuffers[!horizontal]);  // bind texture of other framebuffer (or scene if first iteration)
			RenderQuad();
			horizontal = !horizontal;
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// 3. now render floating point color buffer to 2D quad and tonemap HDR colors to default framebuffer's (clamped) color range
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		shaderFinal.Use();
		glBindTextureUnit(0, colorBuffers[0]);
		glBindTextureUnit(1, pingpongColorbuffers[!horizontal]);
		shaderFinal.SetFloat("exposure", exposure);
		RenderQuad();

		// Cube light
		lightShader.Use();
		lightShader.SetMat4("projection", projection);
		lightShader.SetMat4("view", view);
		light.Render(lightShader);

		SwapBuffers();
	}

	mainShader.Delete();

	Terminate();

	return 0;
}

unsigned int quadVAO = 0;
unsigned int quadVBO;
void RenderQuad()
{
	if (quadVAO == 0)
	{
		// Quad
		constexpr float quadVertices[] = {
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

	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}