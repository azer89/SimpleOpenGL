#include "AppSSAO.h"
#include "AppSettings.h"
#include "UsefulStuff.h"
#include "PipelineDeferredSSAO.h"

#include <random>

const unsigned int NR_LIGHTS = 200;

int AppSSAO::MainLoop()
{
	if (!IsGLFWWindowCreated() || !IsGLADLoaded())
	{
		return -1;
	}

	glEnable(GL_DEPTH_TEST);

	InitScene();
	InitLights();

	PipelineDeferredSSAO pipeline(
		"SSAO//geometry.vertex", "SSAO//geometry.fragment",
		"SSAO//ssao.vertex", "SSAO//lighting.fragment",
		"SSAO//ssao.vertex", "SSAO//ssao.fragment",
		"SSAO//ssao.vertex", "SSAO//blur.fragment"
	);

	// Game loop
	while (!GLFWWindowShouldClose())
	{
		ProcessTiming();
		ProcessInput();

		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::mat4 projection = camera->GetProjectionMatrix();
		glm::mat4 view = camera->GetViewMatrix();

		// 1 Geometry pass: render scene's geometry/color data into G buffer
		pipeline.StartGeometryPass(projection, view);
		RenderScene(*(pipeline.GetGeometryShader()));
		pipeline.EndGeometryPass();

		// 2 SSAO
		pipeline.StartSSAOPass(projection);

		// 3 Blur SSAO texture to remove noise
		pipeline.StartBlurPass();

		// 4 lighting pass: traditional deferred Blinn-Phong lighting with added screen-space ambient occlusion
		pipeline.StartLightingPass(lights, view, camera->Position);

		// 5 Blit
		pipeline.Blit();

		RenderLights();

		SwapBuffers();
		PollEvents();
	}

	Terminate();

	return 0;
}

void AppSSAO::InitScene()
{
	sponzaModel = std::make_unique<Model>(AppSettings::ModelFolder + "Sponza//Sponza.gltf");
}

void AppSSAO::RenderScene(const Shader& shader)
{
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(10.0f, 0.0f, 1.0f));
	model = glm::scale(model, glm::vec3(0.02f));
	shader.SetMat4("model", model);
	sponzaModel->Draw(shader);
}

unsigned int quadVAO = 0;
void AppSSAO::RenderQuad()
{
	if (quadVAO == 0)
	{
		float quadVertices[] = {
			// Positions		// Texture Coords
			-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
			 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
			 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		};
		// setup plane VAO
		unsigned int quadVBO = 0;
		glGenVertexArrays(1, &quadVAO);
		glGenBuffers(1, &quadVBO);
		glBindVertexArray(quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	}
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}

void AppSSAO::InitLights()
{
	lightSphereShader = std::make_unique<Shader>("Misc//light_sphere.vertex", "Misc//light_sphere.fragment");

	float pi2 = glm::two_pi<float>();

	for (unsigned int i = 0; i < NR_LIGHTS; i++)
	{
		float yPos = UsefulStuff::RandomNumber<float>(0.15f, 10.0f);
		float radius = UsefulStuff::RandomNumber<float>(0.0f, 20.0f);
		float rad = UsefulStuff::RandomNumber<float>(0.0f, pi2);
		float xPos = glm::cos(rad);

		glm::vec3 position(
			glm::cos(rad) * radius,
			yPos,
			glm::sin(rad) * radius
		);

		glm::vec3 color(
			UsefulStuff::RandomNumber<float>(0.2f, 0.5f),
			UsefulStuff::RandomNumber<float>(0.2f, 0.5f),
			UsefulStuff::RandomNumber<float>(0.5f, 1.0f)
		);

		lightAngles.push_back(rad);
		lightRadii.push_back(radius);
		lights.emplace_back(position, color, true, 0.1f);
	}

	lightSphereShader->Use();
	lightSphereShader->SetFloat("radius", 0.2f);
}

void AppSSAO::RenderLights()
{
	lightSphereShader->Use();
	lightSphereShader->SetMat4("projection", camera->GetProjectionMatrix());
	lightSphereShader->SetMat4("view", camera->GetViewMatrix());
	for (unsigned int i = 0; i < lights.size(); i++)
	{
		lights[i].Render(*lightSphereShader);
	}
}