#ifndef __TEXTURE_H__
#define __TEXTURE_H__

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>
#include <vector>

class Texture
{
private:
	unsigned int id;

	// These two are only for loading glTF, otherwise they are empty
	std::string textureType;
	std::string textureName;

public:
	Texture();
	Texture(std::string textureType, std::string textureName);
	void CreateFromImageFile(const std::string& fullFilePath, bool flipVertical = false);
	void CreateDepthMap(unsigned int width, unsigned int height);
	void CreateCubeMap(const std::vector<std::string>& files, const std::string& directory);
	void Bind(GLenum textureIndex);

	unsigned int GetID();
	const char* GetType();
	const char* GetName();
};

#endif