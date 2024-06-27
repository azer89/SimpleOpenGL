#ifndef LIGHT
#define LIGHT

#include "Shader.h"
#include "Shape.h"

#include <memory>

#include "glm/glm.hpp"

class Light
{
public:
	Light(glm::vec3 position, glm::vec3 color, bool debugShow = true, float debugRadius = 0.1f);
	void Render(const Shader& shader) const;

public:
	glm::vec3 Position{};
	glm::vec3 Color{};

private:
	bool shouldRender{};
	float renderRadius{};

	std::unique_ptr<Quad> quad{};
};

#endif