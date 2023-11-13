#include "AppSkyboxWithModel.h"
#include "Texture.h"
#include "Shader.h"
#include "Model.h"
#include "AppSettings.h"
#include "ShapeFactory.h"

int AppSkyboxWithModel::MainLoop()
{
	if (!IsGLFWWindowCreated() || !IsGLADLoaded())
	{
		return -1;
	}

	glEnable(GL_DEPTH_TEST);

	InitSkybox();

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

	Shader mainShader("Cubemap//blinn_phong_skybox.vertex", "Cubemap//blinn_phong_skybox.fragment");
	Shader skyboxShader("Cubemap//skybox.vertex", "Cubemap//skybox.fragment");

	Model obj(AppSettings::ModelFolder + "DamagedHelmet//DamagedHelmet.gltf");

	skyboxShader.Use();
	skyboxShader.SetInt("skybox", 0);

	glm::vec3 lightPos(0.0f, 1.5f, 1.5f);
	auto modelRotation = 0.0f;

	while (!GLFWWindowShouldClose())
	{
		ProcessTiming();
		ProcessInput();

		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::mat4 view = camera->GetViewMatrix();
		glm::mat4 projection = camera->GetProjectionMatrix();

		// glTF
		mainShader.Use();
		mainShader.SetMat4("projection", projection);
		mainShader.SetMat4("view", view);
		mainShader.SetVec3("viewPos", camera->Position);
		mainShader.SetVec3("lightPos", lightPos);

		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
		model = glm::rotate(model, glm::half_pi<float>(), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::rotate(model, modelRotation, glm::vec3(0.0f, 0.0f, 1.0f));
		modelRotation += deltaTime;
		mainShader.SetMat4("model", model);
		obj.Draw(mainShader);

		// Draw skybox
		glDepthFunc(GL_LEQUAL); // Change depth function so depth test passes when values are equal to depth buffer's content
		skyboxShader.Use();
		auto skyboxView = glm::mat4(glm::mat3(view)); // Remove translation from the view matrix
		skyboxShader.SetMat4("view", skyboxView);
		skyboxShader.SetMat4("projection", projection);
		glBindVertexArray(skyboxVAO);
		skyboxTexture.Bind(GL_TEXTURE0);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);
		glDepthFunc(GL_LESS); // Set depth function back to default
		
		SwapBuffers();
		PollEvents();
	}

	Terminate();

	return 0;
}

void AppSkyboxWithModel::InitSkybox()
{
	auto skyboxVertices = ShapeFactory::GenerateCubeVertices();

	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * skyboxVertices.size(), skyboxVertices.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
}