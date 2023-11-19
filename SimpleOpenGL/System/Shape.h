#ifndef __SHAPE_H__
#define __SHAPE_H__

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
	~Quad();
	void Draw() override;
};

#endif