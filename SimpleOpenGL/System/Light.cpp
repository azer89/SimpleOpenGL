#include "Light.h"

#include <glm/ext/matrix_transform.hpp>

Light::Light(glm::vec3 position, glm::vec3 color, bool debugShow, float debugRadius) :
	Position(position),
	Color(color),
	shouldRender(debugShow),
	renderRadius(debugRadius)
{
	if (shouldRender)
	{
		quad = std::make_unique<Quad>();
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
	shader.SetVec3("lightColor", Color);
	shader.SetVec3("lightPosition", Position);

	quad->Draw();
}