#include "AppSSAO.h"
#include "AppSettings.h"

int AppSSAO::MainLoop()
{
	if (!IsGLFWWindowCreated() || !IsGLADLoaded())
	{
		return -1;
	}

	glEnable(GL_DEPTH_TEST);

	while (!GLFWWindowShouldClose())
	{
		ProcessTiming();
		ProcessInput();

		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		SwapBuffers();
		PollEvents();
	}

	Terminate();

	return 0;
}

void AppSSAO::InitScene()
{
	// Fox
	foxModel = std::make_unique<Model>(AppSettings::ModelFolder + "Fox//Fox.gltf");

	// Grass texture
	grassTexture = std::make_unique<Texture>();
	grassTexture->CreateFromImageFile(AppSettings::TextureFolder + "grass.png");

	// Grass plane
	float halfWidth = 50.0f;
	float planeVertices[] = {
		// Positions				   // Normals			// Texcoords
		 halfWidth, 0.0f,  halfWidth,  0.0f, 1.0f, 0.0f,  25.0f,  0.0f,
		-halfWidth, 0.0f,  halfWidth,  0.0f, 1.0f, 0.0f,   0.0f,  0.0f,
		-halfWidth, 0.0f, -halfWidth,  0.0f, 1.0f, 0.0f,   0.0f, 25.0f,

		 halfWidth, 0.0f,  halfWidth,  0.0f, 1.0f, 0.0f,  25.0f,  0.0f,
		-halfWidth, 0.0f, -halfWidth,  0.0f, 1.0f, 0.0f,   0.0f, 25.0f,
		 halfWidth, 0.0f, -halfWidth,  0.0f, 1.0f, 0.0f,  25.0f, 25.0f
	};
	unsigned int planeVBO;
	glGenVertexArrays(1, &planeVAO);
	glGenBuffers(1, &planeVBO);
	glBindVertexArray(planeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glBindVertexArray(0);
}

void AppSSAO::RenderScene(const Shader& shader)
{
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(2.0f, 0.0f, 1.0f));
	model = glm::scale(model, glm::vec3(0.025f));
	shader.SetMat4("model", model);
	foxModel->Draw(shader);

	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.0f, 0.0f, 2.0f));
	model = glm::scale(model, glm::vec3(0.02f));
	shader.SetMat4("model", model);
	foxModel->Draw(shader);

	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(-1.5f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(0.03f));
	shader.SetMat4("model", model);
	foxModel->Draw(shader);

	model = glm::mat4(1.0f);
	shader.SetMat4("model", model);
	grassTexture->Bind(GL_TEXTURE0);
	glBindVertexArray(planeVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
}