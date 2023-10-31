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
	//Texture(const std::string& imageFilename, GLenum textureIndex);
	void CreateFromImageFile(const std::string& imageFilename);
	void CreateDepthMap(unsigned int width, unsigned int height);
	void Bind(GLenum textureIndex);

	unsigned int GetID()
	{
		return ID;
	}
};

#endif