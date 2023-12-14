#include "AppEdge.h"
#include "Shader.h"
#include "Model.h"
#include "Light.h"
#include "Texture.h"
#include "AppSettings.h"

int AppEdge::MainLoop()
{
	// Configure global opengl state
	glEnable(GL_DEPTH_TEST);

	Shader depthShader("Edge//depth.vertex", "Edge//depth.fragment");

	Shader mainShader("Edge//composite.vertex", "Edge//composite.fragment");
	Model obj(AppSettings::ModelFolder + "Dragon//Dragon.obj");
	auto modelRotation = glm::radians(180.f);

	//glm::vec3 lightPos(0.0f, 0.5f, 5.0f);
	Shader lightShader("Misc//light_sphere.vertex", "Misc//light_sphere.fragment");
	Light light(glm::vec3(0.0f, 0.5f, 5.0f), glm::vec3(1.0f));

	// Depth
	Texture depthTexture;
	depthTexture.CreateDepthMap(AppSettings::ScreenWidth, AppSettings::ScreenHeight);

	// Depth FBO
	unsigned int depthFBO;
	glCreateFramebuffers(1, &depthFBO);
	glNamedFramebufferTexture(depthFBO, GL_DEPTH_ATTACHMENT, depthTexture.GetID(), 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

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
		glBindFramebuffer(GL_FRAMEBUFFER, depthFBO);
		glClear(GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		depthShader.SetMat4("model", model);
		obj.Draw(depthShader);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		// Main shader
		mainShader.Use();
		mainShader.SetMat4("projection", projection);
		mainShader.SetMat4("view", view);
		mainShader.SetVec3("viewPos", camera->Position);
		mainShader.SetVec3("lightPos", light.Position);
		mainShader.SetMat4("model", model);
		mainShader.SetInt("texture_depth1", 1);
		depthTexture.BindDSA(1);
		mainShader.SetFloat("near_plane", NEAR_PLANE);
		mainShader.SetFloat("far_plane", FAR_PLANE);
		mainShader.SetFloat("screen_width", AppSettings::ScreenWidth);
		mainShader.SetFloat("screen_height", AppSettings::ScreenHeight);
		obj.Draw(mainShader);

		// Light
		lightShader.Use();
		lightShader.SetMat4("projection", projection);
		lightShader.SetMat4("view", view);
		light.Render(lightShader);

		SwapBuffers();
	}

	Terminate();

	return 0;
}