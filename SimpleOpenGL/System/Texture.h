#ifndef __TEXTURE_H__
#define __TEXTURE_H__

#include "glad/glad.h"

#include <string>
#include <vector>

#include "TextureMapper.h"

class Texture
{
private:
	unsigned int id;

	// These two are only for loading glTF, otherwise they are empty
	TextureType textureType;
	std::string textureName;

public:
	Texture();
	Texture(const std::string& fullFilePath);
	Texture(TextureType textureType, const std::string& textureName);
	void CreateFromImageFile(const std::string& fullFilePath, bool flipVertical = false);
	void CreateFromHDRFile(const std::string& fullFilePath);
	void CreateDepthMap(unsigned int width, unsigned int height);
	void CreateCubeMap(const std::vector<std::string>& files, const std::string& directory);
	void Bind(GLenum textureIndex);

	unsigned int GetID();
	TextureType GetType();
	const char* GetName();
};

#endif