#include "AppGeom.h"
#include "Shader.h"

int AppGeom::MainLoop()
{
	glEnable(GL_DEPTH_TEST);

	Shader shader(
		"GeometryShader//geom.vertex", 
		"GeometryShader//geom.fragment", 
		"GeometryShader//geom.geometry");

	constexpr float points[]{
		0.0f,  0.0f, 0.0f, 1.0f, 0.0f
	};
	unsigned int VBO, VAO;
	glGenBuffers(1, &VBO);
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(points), &points, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(2 * sizeof(float)));
	glBindVertexArray(0);

	while (!GLFWWindowShouldClose())
	{
		ProcessLoop(
			glm::vec4(0.2f, 0.3f, 0.3f, 1.0f),
			GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT
		);

		shader.Use();
		glBindVertexArray(VAO);
		glDrawArrays(GL_POINTS, 0, 4);

		SwapBuffers();
	}

	Terminate();

	return 0;
}