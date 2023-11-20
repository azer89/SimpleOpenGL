#include "AppBlinnPhong.h"

#include "Shader.h"
#include "Texture.h"
#include "AppSettings.h"
#include "XMLReader.h"

int AppBlinnPhong::MainLoop()
{
	// Configure global opengl state
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Light
	glm::vec3 lightPos(0.0f, 0.0f, 0.0f);

	// Shader programs
	Shader shader("BlinnPhong//blinn_phong.vertex", "BlinnPhong//blinn_phong.fragment");

	// Texture
	Texture texture;
	texture.CreateFromImageFile(AppSettings::TextureFolder + "wood.png");

	// Quad
	constexpr float vertices[] = {
		// positions            // normals            // texcoords
		 10.0f, -0.5f,  10.0f,  0.0f, 1.0f, 0.0f,  10.0f,  0.0f,
		-10.0f, -0.5f,  10.0f,  0.0f, 1.0f, 0.0f,   0.0f,  0.0f,
		-10.0f, -0.5f, -10.0f,  0.0f, 1.0f, 0.0f,   0.0f, 10.0f,

		 10.0f, -0.5f,  10.0f,  0.0f, 1.0f, 0.0f,  10.0f,  0.0f,
		-10.0f, -0.5f, -10.0f,  0.0f, 1.0f, 0.0f,   0.0f, 10.0f,
		 10.0f, -0.5f, -10.0f,  0.0f, 1.0f, 0.0f,  10.0f, 10.0f
	};

	unsigned int VBO, VAO; //EBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_STATIC_DRAW);

	// Position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// Normal attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	// Texture coord attribute
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	glBindVertexArray(0);

	// Texture
	shader.Use();
	shader.SetInt("texture1", 0);

	// Render loop
	while (!GLFWWindowShouldClose())
	{
		ProcessLoop(
			glm::vec4(0.2f, 0.3f, 0.3f, 1.0f), 
			GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT
		);

		shader.Use();
		shader.SetMat4("projection", camera->GetProjectionMatrix());
		shader.SetMat4("view", camera->GetViewMatrix());
		shader.SetVec3("viewPos", camera->Position);
		shader.SetVec3("lightPos", lightPos);

		// floor
		glBindVertexArray(VAO);
		glActiveTexture(GL_TEXTURE0);
		texture.Bind(GL_TEXTURE0);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		SwapBuffers();
	}

	// De-allocate all resources once they've outlived their purpose
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);

	// GLFW: terminate, clearing all previously allocated GLFW resources.
	Terminate();

	return 0;
}