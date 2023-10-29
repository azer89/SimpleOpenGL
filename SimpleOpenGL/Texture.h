#ifndef __TEXTURE_H__
#define __TEXTURE_H__

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>

class Texture
{
private:
	unsigned int ID;
	GLenum textureIndex;

public:
	Texture(const std::string& imageFilename, GLenum textureIndex);
	void Bind();
};

#endif