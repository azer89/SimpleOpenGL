#ifndef __TEXTURE_H__
#define __TEXTURE_H__

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>

class Texture
{
private:
	unsigned int id;
	std::string textureType;
	std::string textureName;

public:
	Texture();
	Texture(std::string textureType, std::string textureName);
	void CreateFromImageFile(const std::string& fullFilePath, bool flipVertical = false);
	void CreateDepthMap(unsigned int width, unsigned int height);
	void Bind(GLenum textureIndex);

	unsigned int GetID();
	std::string GetType();
	std::string GetName();
};

#endif