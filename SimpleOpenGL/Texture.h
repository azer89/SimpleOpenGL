#ifndef __TEXTURE_H__
#define __TEXTURE_H__

#include <glad/glad.h>
#include <glm/glm.hpp>

class Texture
{
private:
	unsigned int ID;
	GLenum textureIndex;

public:
	Texture(const char* imageFilePath, GLenum textureIndex);
	void Bind();
};

#endif