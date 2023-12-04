#include "AppSkyboxWithCube.h"
#include "Texture.h"
#include "Shader.h"
#include "AppSettings.h"
#include "Shape.h"

int AppSkyboxWithCube::MainLoop()
{
	glEnable(GL_DEPTH_TEST);

	Cube cube;

	Texture cubeTexture;
	cubeTexture.CreateFromImageFile(AppSettings::TextureFolder + "neco_coneco.jpg", true);

	Texture skyboxTexture;
	std::vector<std::string> files
	{
		"right.png",
		"left.png",
		"top.png",
		"bottom.png",
		"front.png",
		"back.png"
	};
	skyboxTexture.CreateCubeMap(files, AppSettings::TextureFolder + "skybox_blue_space//");

	Shader cubeShader("Cubemap//cube.vertex", "Cubemap//cube.fragment");
	Shader mainShader("Cubemap//blinn_phong_skybox.vertex", "Cubemap//blinn_phong_skybox.fragment");
	Shader skyboxShader("Cubemap//skybox.vertex", "Cubemap//skybox.fragment");

	cubeShader.Use();
	cubeShader.SetInt("skybox", 0);
	cubeShader.SetInt("texture1", 1);

	skyboxShader.Use();
	skyboxShader.SetInt("skybox", 0);

	glm::vec3 lightPos(0.0f, 1.5f, 1.5f);
	auto modelRotation = 0.0f;

	while (!GLFWWindowShouldClose())
	{
		ProcessLoop(
			glm::vec4(0.2f, 0.3f, 0.3f, 1.0f),
			GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT
		);

		glm::mat4 view = camera->GetViewMatrix();
		glm::mat4 projection = camera->GetProjectionMatrix();

		// Draw Cube
		cubeShader.Use();
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::scale(model, glm::vec3(0.5f));
		cubeShader.SetMat4("model", model);
		cubeShader.SetMat4("view", view);
		cubeShader.SetMat4("projection", projection);
		cubeShader.SetVec3("cameraPos", camera->Position);
		skyboxTexture.BindDSA(0);
		cubeTexture.BindDSA(1);
		cube.Draw();
		glBindVertexArray(0);

		// Draw skybox
		glDepthFunc(GL_LEQUAL); // Change depth function so depth test passes when values are equal to depth buffer's content
		skyboxShader.Use();
		auto skyboxView = glm::mat4(glm::mat3(view)); // Remove translation from the view matrix
		skyboxShader.SetMat4("view", skyboxView);
		skyboxShader.SetMat4("projection", projection);
		skyboxTexture.BindDSA(0);
		cube.Draw();
		glBindVertexArray(0);
		glDepthFunc(GL_LESS); // Set depth function back to default
		
		SwapBuffers();
	}

	Terminate();

	return 0;
}