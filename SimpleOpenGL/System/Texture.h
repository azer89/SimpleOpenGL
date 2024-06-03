#ifndef TEXTURE
#define TEXTURE

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
	~Texture();
	void CreateFromImageFile(const std::string& fullFilePath, bool flipVertical = false);
	void CreateFromHDRFile(const std::string& fullFilePath);
	void CreateDepthMap(unsigned int width, unsigned int height);
	void CreateCubeMap(const std::vector<std::string>& files, const std::string& directory);

	void Bind(unsigned int textureUnit);

	[[nodiscard]] unsigned int GetID();
	[[nodiscard]] TextureType GetType();
	[[nodiscard]] const char* GetName();
};

#endif