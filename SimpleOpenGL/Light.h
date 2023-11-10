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
	Light(glm::vec3 position, glm::vec3 color);

private:
	std::unique_ptr<Shader> shader;

};

#endif