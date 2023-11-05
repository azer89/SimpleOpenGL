#ifndef __TEXTURE_H__
#define __TEXTURE_H__

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>

class Texture
{
private:
	unsigned int ID;

public:
	Texture();
	void CreateFromImageFile(const std::string& fullFilePath);
	void CreateDepthMap(unsigned int width, unsigned int height);
	void Bind(GLenum textureIndex);

	unsigned int GetID()
	{
		return ID;
	}
};

#endif