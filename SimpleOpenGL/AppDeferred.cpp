#include "AppDeferred.h"
#include "AppSettings.h"
#include "ShapeFactory.h"
#include "GBuffer.h"

#include <memory>

int AppDeferred::MainLoop()
{
	if (!IsGLFWWindowCreated() || !IsGLADLoaded())
	{
		return -1;
	}

	glEnable(GL_DEPTH_TEST);

	Shader lightSphereShader("light_sphere.vertex", "light_sphere.fragment");

	Texture grassTexture;
	grassTexture.CreateFromImageFile(AppSettings::TextureFolder + "grass.png");

	GBuffer gBuffer;

	InitLights();
	InitScene();

	lightSphereShader.Use();
	lightSphereShader.SetFloat("radius", 0.2f);

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
		grassTexture.Bind(GL_TEXTURE0);
		RenderPlane(*geomShaderPtr);
		RenderFoxes(*geomShaderPtr);
		gBuffer.EndGeometryPass();

		// 2 Lighting Pass
		gBuffer.StartLightingPass(lights, camera->Position);

		// 3 Copy content of geometry's depth buffer to default framebuffer's depth buffer
		gBuffer.Blit();

		// Render lights on top of scene
		lightSphereShader.Use();
		lightSphereShader.SetMat4("projection", projection);
		lightSphereShader.SetMat4("view", view);
		for (unsigned int i = 0; i < lights.size(); i++)
		{
			lights[i].Render(lightSphereShader);
		}

		SwapBuffers();
		PollEvents();
	}

	Terminate();

	return 0;
}

void AppDeferred::InitLights()
{
	const unsigned int NR_LIGHTS = 64;
	srand(time(NULL));
	for (unsigned int i = 0; i < NR_LIGHTS; i++)
	{
		glm::vec3 position(
			static_cast<float>(((rand() % 100) / 100.0) * 12.0 - 6.0),
			static_cast<float>(((rand() % 100) / 100.0) * 1.0 + 0.15),
			static_cast<float>(((rand() % 100) / 100.0) * 12.0 - 6.0)
		);
		glm::vec3 color(
			static_cast<float>(((rand() % 100) / 200.0f) + 0.5), // Between 0.5 and 1.0
			static_cast<float>(((rand() % 100) / 200.0f) + 0.5), // Between 0.5 and 1.0
			static_cast<float>(((rand() % 100) / 200.0f) + 0.5) // Between 0.5 and 1.0
		);
		lights.emplace_back(position, color, true, 0.1f);
	}
}

void AppDeferred::InitScene()
{
	// Fox
	foxModel = std::make_unique<Model>(AppSettings::ModelFolder + "Fox//Fox.gltf");

	float halfWidth = 50.0f;

	// Plane
	float planeVertices[] = {
		// Positions				   // Normals			// Texcoords
		 halfWidth, 0.0f,  halfWidth,  0.0f, 1.0f, 0.0f,  25.0f,  0.0f,
		-halfWidth, 0.0f,  halfWidth,  0.0f, 1.0f, 0.0f,   0.0f,  0.0f,
		-halfWidth, 0.0f, -halfWidth,  0.0f, 1.0f, 0.0f,   0.0f, 25.0f,

		 halfWidth, 0.0f,  halfWidth,  0.0f, 1.0f, 0.0f,  25.0f,  0.0f,
		-halfWidth, 0.0f, -halfWidth,  0.0f, 1.0f, 0.0f,   0.0f, 25.0f,
		 halfWidth, 0.0f, -halfWidth,  0.0f, 1.0f, 0.0f,  25.0f, 25.0f
	};
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

void AppDeferred::RenderPlane(const Shader& shader)
{
	glm::mat4 model = glm::mat4(1.0f);
	shader.SetMat4("model", model);
	glBindVertexArray(planeVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
}

void AppDeferred::RenderFoxes(const Shader& shader)
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
}