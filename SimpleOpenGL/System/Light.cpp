#include "Light.h"
#include "ShapeFactory.h"

#include <glm/ext/matrix_transform.hpp>

Light::Light(glm::vec3 position, glm::vec3 color, bool debugShow, float debugRadius) :
	Position(position),
	Color(color),
	shouldRender(debugShow),
	renderRadius(debugRadius)
{
	if (shouldRender)
	{
		auto vertices = ShapeFactory::GenerateQuadVertices();

		glGenBuffers(1, &VBO);
		glGenVertexArrays(1, &VAO);
		glBindVertexArray(VAO);

		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices.size(), vertices.data(), GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
		glBindVertexArray(0);
	}
}

void Light::Render(const Shader& shader)
{
	if (!shouldRender)
	{
		return;
	}

	shader.Use();
	shader.SetFloat("radius", renderRadius);

	//glm::mat4 model(1.0f);
	//model = glm::translate(model, Position);

	//shader.SetMat4("model", model);
	shader.SetVec3("lightColor", Color);
	shader.SetVec3("lightPosition", Position);

	glBindVertexArray(VAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
}