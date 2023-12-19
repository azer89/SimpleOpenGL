#include "AppDeferred.h"
#include "AppSettings.h"
#include "PipelineDeferred.h"
#include "Utility.h"

#include <memory>

int AppDeferred::MainLoop()
{
	glEnable(GL_DEPTH_TEST);
	
	PipelineDeferred pipeline(
		"Deferred//deferred_g_buffer.vertex", "Deferred//deferred_g_buffer.fragment",
		"Deferred//deferred_lighting.vertex", "Deferred//deferred_lighting.fragment"
	);

	InitLights();
	InitScene();

	while (!GLFWWindowShouldClose())
	{
		ProcessLoop(
			glm::vec4(0.0f, 0.0f, 0.0f, 1.0f),
			GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT
		);

		glm::mat4 projection = camera->GetProjectionMatrix();
		glm::mat4 view = camera->GetViewMatrix();

		// 1 Geometry pass
		pipeline.StartGeometryPass(projection, view);
		const Shader* geomShaderPtr = pipeline.GetGeometryShader();
		RenderScene(*geomShaderPtr);
		pipeline.EndGeometryPass();

		// 2 Lighting Pass
		UpdateLightPositions();
		pipeline.StartLightingPass(lights, camera->Position);

		// 3 Copy content of geometry's depth buffer to default framebuffer's depth buffer
		pipeline.Blit();

		// Render lights
		RenderLights();

		SwapBuffers();
	}

	Terminate();

	return 0;
}

void AppDeferred::InitLights()
{
	lightSphereShader = std::make_unique<Shader>("Misc//light_sphere.vertex", "Misc//light_sphere.fragment");
	
	float pi2 = glm::two_pi<float>();

	constexpr unsigned int NR_LIGHTS = 64;
	for (unsigned int i = 0; i < NR_LIGHTS; ++i)
	{
		float yPos = Utility::RandomNumber<float>(0.15f, 1.0f);
		float radius = Utility::RandomNumber<float>(3.0f, 8.0f);
		float rad = Utility::RandomNumber<float>(0.0f, pi2);
		float xPos = glm::cos(rad);

		glm::vec3 position(
			glm::cos(rad) * radius,
			yPos,
			glm::sin(rad) * radius
		);

		glm::vec3 color(
			Utility::RandomNumber<float>(0.5f, 1.0f),
			Utility::RandomNumber<float>(0.5f, 1.0f),
			Utility::RandomNumber<float>(0.5f, 1.0f)
		);

		lightAngles.push_back(rad);
		lightRadii.push_back(radius);
		lights.emplace_back(position, color, true, 0.1f);
	}
}

void AppDeferred::UpdateLightPositions()
{
	for (unsigned int i = 0; i < lights.size(); ++i)
	{
		float step = deltaTime;

		float yPos = lights[i].Position.y;
		lightAngles[i] += step;

		lights[i].Position = glm::vec3(
			glm::cos(lightAngles[i]) * lightRadii[i],
			yPos,
			glm::sin(lightAngles[i]) * lightRadii[i]
		);
	}
}

void AppDeferred::InitScene()
{
	// Fox
	foxModel = std::make_unique<Model>(AppSettings::ModelFolder + "Fox//Fox.gltf");

	// Grass texture
	grassTexture = std::make_unique<Texture>();
	grassTexture->CreateFromImageFile(AppSettings::TextureFolder + "grass.png");

	// Grass plane
	constexpr float halfWidth = 50.0f;
	constexpr float planeVertices[] = {
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
	for (Light& light : lights)
	{
		light.Render(*lightSphereShader);
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
	grassTexture->Bind(0);
	glBindVertexArray(planeVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
}