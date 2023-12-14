#include "AppEdge.h"
#include "Shader.h"
#include "Model.h"
#include "Light.h"
#include "Texture.h"
#include "AppSettings.h"
#include <stdexcept>

unsigned int quadVAO;
unsigned int quadVBO;
void InitQuad();

void RenderQuad();

int AppEdge::MainLoop()
{
	// Configure global opengl state
	glEnable(GL_DEPTH_TEST);

	Shader depthShader("Edge//depth.vertex", "Edge//depth.fragment");

	InitQuad();

	Shader compositeShader("Edge//composite.vertex", "Edge//composite.fragment");
	compositeShader.Use();
	compositeShader.SetInt("texture_depth1", 0);
	compositeShader.SetFloat("near_plane", NEAR_PLANE);
	compositeShader.SetFloat("far_plane", FAR_PLANE);
	compositeShader.SetFloat("screen_width", AppSettings::ScreenWidth);
	compositeShader.SetFloat("screen_height", AppSettings::ScreenHeight);

	Model obj(AppSettings::ModelFolder + "Dragon/Dragon.obj");
	auto modelRotation = glm::radians(180.f);

	//glm::vec3 lightPos(0.0f, 0.5f, 5.0f);
	Shader lightShader("Misc//light_sphere.vertex", "Misc//light_sphere.fragment");
	Light light(glm::vec3(0.0f, 0.5f, 5.0f), glm::vec3(1.0f));

	// Depth FBO
	unsigned int gBufferFBO;
	glCreateFramebuffers(1, &gBufferFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, gBufferFBO);

	const int numMipmaps = 1;

	// Position
	unsigned int gPositionTexture;
	glCreateTextures(GL_TEXTURE_2D, 1, &gPositionTexture);
	glTextureParameteri(gPositionTexture, GL_TEXTURE_MAX_LEVEL, numMipmaps - 1);
	glTextureParameteri(gPositionTexture, GL_TEXTURE_MIN_FILTER, GL_NEAREST); 
	glTextureParameteri(gPositionTexture, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTextureParameteri(gPositionTexture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(gPositionTexture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTextureStorage2D(gPositionTexture, numMipmaps, GL_RGBA16F, AppSettings::ScreenWidth, AppSettings::ScreenHeight);
	glNamedFramebufferTexture(gBufferFBO, GL_COLOR_ATTACHMENT0, gPositionTexture, 0);

	unsigned int attachments[1] = { GL_COLOR_ATTACHMENT0};
	glNamedFramebufferDrawBuffers(gBufferFBO, 1, attachments);

	// Depth render buffer
	unsigned int depthRBO;
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

	// Uniform
	float edgeThreshold = 0.01;

	// Render loop
	while (!GLFWWindowShouldClose())
	{
		ProcessLoop(
			glm::vec4(0.2f, 0.3f, 0.3f, 1.0f),
			GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT
		);

		auto projection = camera->GetProjectionMatrix();
		auto view = camera->GetViewMatrix();

		// Model matrix
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
		model = glm::rotate(model, modelRotation, glm::vec3(0.0f, 1.0f, 0.0f));
		//modelRotation += deltaTime * 0.2f;

		// Render depth
		depthShader.Use();
		depthShader.SetMat4("projection", projection);
		depthShader.SetMat4("view", view);
		depthShader.SetMat4("model", model);
		glBindFramebuffer(GL_FRAMEBUFFER, gBufferFBO);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		obj.Draw(depthShader);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		// Main shader
		compositeShader.Use();
		compositeShader.SetFloat("edgeThreshold", edgeThreshold);
		glBindTextureUnit(0, gPositionTexture);
		RenderQuad();

		// Light
		lightShader.Use();
		lightShader.SetMat4("projection", projection);
		lightShader.SetMat4("view", view);
		light.Render(lightShader);

		if (showImgui)
		{
			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();
			ImGui::SetNextWindowSize(ImVec2(500, 200));

			ImGui::Begin("Edge Detection");

			ImGui::SliderFloat("Threshold", &edgeThreshold, 0.001f, 1.0f);

			ImGui::End();

			ImGui::Render();
			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		}

		SwapBuffers();
	}

	Terminate();

	return 0;
}

void InitQuad()
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

void RenderQuad()
{
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}