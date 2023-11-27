#ifndef __SHAPE_H__
#define __SHAPE_H__

#include "glm/glm.hpp"
#include "glm/ext/scalar_constants.hpp"
#include <glm/ext/matrix_transform.hpp>
#include "glad/glad.h"

#include <vector>

class Shape
{
protected:
	unsigned int VAO;
	unsigned int VBO;

public:
	virtual void Draw() = 0;
	unsigned int GetVAO()
	{
		return VAO;
	}
};

class Sphere : Shape
{
public:
	Sphere();
	~Sphere();
	void Draw() override;

private:
	unsigned int indexCount;
};

class Cube : Shape
{
public:
	Cube();
	~Cube();
	void Draw() override;
};

class Quad : Shape
{
public:
	Quad();
	Quad(float rotation, glm::vec3 rotationAxis);
	~Quad();
	void Draw() override;

private:
	std::vector<float> GetQuadData();
	void SetUpVAOVBO(const std::vector<float>& vertices);
};

#endif