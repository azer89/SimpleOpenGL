#ifndef __LIGHT_H__
#define __LIGHT_H__

#include "Shader.h"

#include <memory>
#include <glm/glm.hpp>

class Light
{
public:
	glm::vec3 Position;
	glm::vec3 Color;

public:
	Light(glm::vec3 position, glm::vec3 color, bool debugShow = true, float debugRadius = 0.2f);
	void Render(const Shader& shader);

private:
	bool shouldRender;
	float renderRadius;

	unsigned int VAO = 0;
	unsigned int VBO = 0;
};

#endif