#ifndef SHAPE
#define SHAPE

#include "glm/glm.hpp"

#include <vector>

class Shape
{
protected:
	unsigned int VAO{};
	unsigned int VBO{};

public:
	virtual void Draw() const = 0;
	unsigned int GetVAO() const
	{
		return VAO;
	}
};

class Sphere : Shape
{
public:
	Sphere();
	~Sphere();
	void Draw() const override;

private:
	unsigned int indexCount;
};

class Cube : Shape
{
public:
	Cube();
	~Cube();
	void Draw() const override;
};

class Quad : Shape
{
public:
	Quad();
	Quad(float rotation, glm::vec3 rotationAxis);
	~Quad();
	void Draw() const override;

private:
	std::vector<float> GetQuadData() const;
	void SetUpVAOVBO(const std::vector<float>& vertices);
};

#endif