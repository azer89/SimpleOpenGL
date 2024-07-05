#include "Texture.h"
#include "Utility.h"

#include "glad/glad.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STBI_MSC_SECURE_CRT
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <iostream>

Texture::Texture() :
	id (GL_INVALID_VALUE),
	textureType(TextureType::NONE)
{
}

Texture::Texture(const std::string& fullFilePath)
{
	CreateFromImageFile(fullFilePath);
}

Texture::Texture(TextureType texType, const std::string& texName) :
	textureType(texType),
	textureName(texName),
	id(GL_INVALID_VALUE)
{
}

Texture::~Texture()
{
}

unsigned int Texture::GetID() const
{
	return id;
}

TextureType Texture::GetType() const
{
	return textureType;
}

const char* Texture::GetName() const
{
	return textureName.c_str();
}

void Texture::CreateFromImageFile(const std::string& fullFilePath, bool flipVertical)
{
	int width;
	int height;
	stbi_set_flip_vertically_on_load(flipVertical);
	uint8_t* data = stbi_load(fullFilePath.c_str(), &width, &height, nullptr, STBI_rgb_alpha);
	if (data)
	{
		GLenum clamp = GL_REPEAT;
		int numMipmaps = Utility::NumMipmap(width, height);
		int maxAnisotropy = 16;
		
		// DSA
		glCreateTextures(GL_TEXTURE_2D, 1, &id);
		
		glTextureParameteri(id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTextureParameteri(id, GL_TEXTURE_WRAP_S, clamp);
		glTextureParameteri(id, GL_TEXTURE_WRAP_T, clamp);
		glTextureParameteri(id, GL_TEXTURE_MAX_LEVEL, numMipmaps - 1);
		glTextureParameteri(id, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTextureParameteri(id, GL_TEXTURE_MAX_ANISOTROPY, maxAnisotropy);

		// Allocate the memory and set the format
		glTextureStorage2D(id, numMipmaps, GL_RGBA8, width, height);
		// Upload the data
		glTextureSubImage2D(id, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);

		// Mipmap
		glGenerateTextureMipmap(id);
	}
	else
	{
		throw std::runtime_error("Failed to load texture" + fullFilePath);
	}
	stbi_image_free(data);
}

void Texture::CreateFromHDRFile(const std::string& fullFilePath)
{
	stbi_set_flip_vertically_on_load(true);
	int width, height, nrComponents;
	float* data = stbi_loadf(fullFilePath.c_str(), &width, &height, &nrComponents, 0);
	if (data)
	{
		// DSA
		const int numMipmaps = Utility::NumMipmap(width, height);;

		glCreateTextures(GL_TEXTURE_2D, 1, &id);
		glTextureStorage2D(id, numMipmaps, GL_RGB32F, width, height);
		glTextureSubImage2D(id, 0, 0, 0, width, height, GL_RGB, GL_FLOAT, data);

		glTextureParameteri(id, GL_TEXTURE_MAX_LEVEL, numMipmaps - 1);
		glTextureParameteri(id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTextureParameteri(id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTextureParameteri(id, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glGenerateTextureMipmap(id);
	}
	else
	{
		throw std::runtime_error("Failed to load HDR image " + fullFilePath);
	}

	stbi_image_free(data);
}

void Texture::CreateDepthMap(unsigned int width, unsigned int height)
{
	// DSA
	glCreateTextures(GL_TEXTURE_2D, 1, &id);

	const int numMipmaps = 1;
	glTextureParameteri(id, GL_TEXTURE_MAX_LEVEL, numMipmaps - 1);
	
	glTextureParameteri(id, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureParameteri(id, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTextureStorage2D(id, numMipmaps, GL_DEPTH_COMPONENT24, width, height);
	
	const GLfloat border[]{ 1.0, 1.0, 1.0, 1.0 };
	glTextureParameterfv(id, GL_TEXTURE_BORDER_COLOR, border);
	glTextureParameteri(id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTextureParameteri(id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
}

void Texture::CreateCubeMap(const std::vector<std::string>& files, const std::string& directory)
{
	// DSA
	glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &id);

	const int numMipmaps = 1;
	glTextureParameteri(id, GL_TEXTURE_MAX_LEVEL, numMipmaps - 1);

	stbi_set_flip_vertically_on_load(false);
	int width, height;
	for (int i = 0; i < files.size(); ++i)
	{
		auto filePath = directory + files[i];
		uint8_t* data = stbi_load(filePath.c_str(), &width, &height, nullptr, STBI_rgb_alpha);

		if (i == 0)
		{
			// Allocate the memory and set the format
			glTextureStorage2D(id, 1, GL_RGBA8, width, height);
		}
		
		if (data)
		{
			// Upload the data
			glTextureSubImage3D(id, 0, 0, 0, i, width, height, 1, GL_RGBA, GL_UNSIGNED_BYTE, data);
		}
		else
		{
			throw std::runtime_error("Cubemap texture failed to load: " + files[i]);
		}
		stbi_image_free(data);
	}
	glTextureParameteri(id, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTextureParameteri(id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTextureParameteri(id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTextureParameteri(id, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
}

void Texture::Bind(unsigned int textureUnit)
{
	// DSA
	glBindTextureUnit(textureUnit, id);
}