#include "AppSkyboxWithCube.h"
#include "Texture.h"
#include "Shader.h"
#include "AppSettings.h"
#include "ShapeFactory.h"

int AppSkyboxWithCube::MainLoop()
{
	if (!IsGLFWWindowCreated() || !IsGLADLoaded())
	{
		return -1;
	}

	glEnable(GL_DEPTH_TEST);

	InitCube();
	InitSkybox();

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
	//cubeShader.SetInt("texture1", 0);
	cubeShader.SetInt("skybox", 0);
	cubeShader.SetInt("texture1", 1);

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

		// Draw Cube
		cubeShader.Use();
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::scale(model, glm::vec3(0.5f));
		cubeShader.SetMat4("model", model);
		cubeShader.SetMat4("view", view);
		cubeShader.SetMat4("projection", projection);
		cubeShader.SetVec3("cameraPos", camera->Position);
		glBindVertexArray(cubeVAO);
		skyboxTexture.Bind(GL_TEXTURE0);
		cubeTexture.Bind(GL_TEXTURE1);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);

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

void AppSkyboxWithCube::InitCube()
{
	auto cubeVertices = ShapeFactory::GenerateCubeVertices();

	glGenVertexArrays(1, &cubeVAO);
	glGenBuffers(1, &cubeVBO);
	glBindVertexArray(cubeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * cubeVertices.size(), cubeVertices.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
}

void AppSkyboxWithCube::InitSkybox()
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