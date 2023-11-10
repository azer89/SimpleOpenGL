#include "AppDeferred.h"
#include "AppSettings.h"
#include "ShapeFactory.h"
#include "GBuffer.h"
#include "UsefulStuff.h"

#include <memory>

int AppDeferred::MainLoop()
{
	if (!IsGLFWWindowCreated() || !IsGLADLoaded())
	{
		return -1;
	}

	glEnable(GL_DEPTH_TEST);
	
	GBuffer gBuffer;
	InitLights();
	InitScene();

	while (!GLFWWindowShouldClose())
	{
		ProcessTiming();
		ProcessInput();

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::mat4 projection = camera->GetProjectionMatrix();
		glm::mat4 view = camera->GetViewMatrix();

		// 1 Geometry pass
		gBuffer.StartGeometryPass(projection, view);
		Shader* geomShaderPtr = gBuffer.GetGeometryShader();
		RenderScene(*geomShaderPtr);
		gBuffer.EndGeometryPass();

		// 2 Lighting Pass
		gBuffer.StartLightingPass(lights, camera->Position);

		// 3 Copy content of geometry's depth buffer to default framebuffer's depth buffer
		gBuffer.Blit();

		// Render lights
		RenderLights();

		SwapBuffers();
		PollEvents();
	}

	Terminate();

	return 0;
}

void AppDeferred::InitLights()
{
	lightSphereShader = std::make_unique<Shader>("light_sphere.vertex", "light_sphere.fragment");

	const unsigned int NR_LIGHTS = 64;
	srand(time(NULL));
	for (unsigned int i = 0; i < NR_LIGHTS; i++)
	{
		glm::vec3 position(
			UsefulStuff::RandomNumber<float>(-6.0f, 6.0f),
			UsefulStuff::RandomNumber<float>(0.15f, 1.0f),
			UsefulStuff::RandomNumber<float>(-6.0f, 6.0f)
		);

		glm::vec3 color(
			UsefulStuff::RandomNumber<float>(0.5f, 1.0f),
			UsefulStuff::RandomNumber<float>(0.5f, 1.0f),
			UsefulStuff::RandomNumber<float>(0.5f, 1.0f)
		);

		lights.emplace_back(position, color, true, 0.1f);
	}

	lightSphereShader->Use();
	lightSphereShader->SetFloat("radius", 0.2f);
}

void AppDeferred::UpdateLightPositions()
{

}

void AppDeferred::InitScene()
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

void AppDeferred::RenderLights()
{
	lightSphereShader->Use();
	lightSphereShader->SetMat4("projection", camera->GetProjectionMatrix());
	lightSphereShader->SetMat4("view", camera->GetViewMatrix());
	for (unsigned int i = 0; i < lights.size(); i++)
	{
		lights[i].Render(*lightSphereShader);
	}
}

void AppDeferred::RenderScene(const Shader& shader)
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