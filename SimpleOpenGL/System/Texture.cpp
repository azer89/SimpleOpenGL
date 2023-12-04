#include "Texture.h"

#include "glad/glad.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STBI_MSC_SECURE_CRT
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <iostream>

int GetNumMipMapLevels2D(int w, int h)
{
	int levels = 1;
	while ((w | h) >> levels)
	{
		levels += 1;
	}
	return levels;
}

/// Draw a checkerboard on a pre-allocated square RGB image.
uint8_t* genDefaultCheckerboardImage(int* width, int* height)
{
	const int w = 128;
	const int h = 128;

	uint8_t* imgData = (uint8_t*)malloc(w * h * 3); // stbi_load() uses malloc(), so this is safe

	assert(imgData && w > 0 && h > 0);
	assert(w == h);

	if (!imgData || w <= 0 || h <= 0) return nullptr;
	if (w != h) return nullptr;

	for (int i = 0; i < w * h; i++)
	{
		const int row = i / w;
		const int col = i % w;
		imgData[i * 3 + 0] = imgData[i * 3 + 1] = imgData[i * 3 + 2] = 0xFF * ((row + col) % 2);
	}

	if (width) *width = w;
	if (height) *height = h;

	return imgData;
}

Texture::Texture() :
	id (GL_INVALID_VALUE),
	textureType(TextureType::TEXTURE_NONE)
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

unsigned int Texture::GetID()
{
	return id;
}

TextureType Texture::GetType()
{
	return textureType;
}

const char* Texture::GetName()
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
		// Non DSA
		/*
		glBindTexture(GL_TEXTURE_2D, id);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		// Wrapping
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		// Filtering
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		 */

		GLenum clamp = GL_REPEAT;
		int numMipmaps = GetNumMipMapLevels2D(width, height);
		int maxAnisotropy = 16;
		
		// DSA
		glCreateTextures(GL_TEXTURE_2D, 1, &id);
		
		glTextureParameteri(id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTextureParameteri(id, GL_TEXTURE_WRAP_S, clamp);
		glTextureParameteri(id, GL_TEXTURE_WRAP_T, clamp);

		// Allocate the memory and set the format
		glTextureStorage2D(id, numMipmaps, GL_RGBA8, width, height);
		// Upload the data
		glTextureSubImage2D(id, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateTextureMipmap(id);
		glTextureParameteri(id, GL_TEXTURE_MAX_LEVEL, numMipmaps - 1);
		glTextureParameteri(id, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		
		glTextureParameteri(id, GL_TEXTURE_MAX_ANISOTROPY, maxAnisotropy);
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
		// Non DSA
		/*glGenTextures(1, &id);
		glBindTexture(GL_TEXTURE_2D, id);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, data); // note how we specify the texture's data value to be float

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);*/

		// DSA
		const GLenum clamp = GL_CLAMP_TO_EDGE;
		const int numMipmaps = 1;
		int maxAnisotropy = 16;

		glCreateTextures(GL_TEXTURE_2D, 1, &id);

		glTextureParameteri(id, GL_TEXTURE_MAX_LEVEL, 0);
		glTextureParameteri(id, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTextureParameteri(id, GL_TEXTURE_WRAP_S, clamp);
		glTextureParameteri(id, GL_TEXTURE_WRAP_T, clamp);
		
		glTextureStorage2D(id, numMipmaps, GL_RGB16F, width, height);
		glTextureSubImage2D(id, 0, 0, 0, width, height, GL_RGB, GL_FLOAT, data);
		glTextureParameteri(id, GL_TEXTURE_MAX_ANISOTROPY, maxAnisotropy);
	}
	else
	{
		throw std::runtime_error("Failed to load HDR image " + fullFilePath);
	}

	stbi_image_free(data);
}

void Texture::CreateDepthMap(unsigned int width, unsigned int height)
{
	/*glGenTextures(1, &id);
	glBindTexture(GL_TEXTURE_2D, id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);*/

	// DSA
	const int numMipmaps = 1;

	glCreateTextures(GL_TEXTURE_2D, 1, &id);
	glTextureParameteri(id, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureParameteri(id, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTextureStorage2D(id, numMipmaps, GL_DEPTH_COMPONENT24, width, height);
	
	const GLfloat border[] = { 1.0, 1.0, 1.0, 1.0 };
	glTextureParameterfv(id, GL_TEXTURE_BORDER_COLOR, border);
	glTextureParameteri(id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTextureParameteri(id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	
}

void Texture::CreateCubeMap(const std::vector<std::string>& files, const std::string& directory)
{
	// Non DSA
	glGenTextures(1, &id);
	glBindTexture(GL_TEXTURE_CUBE_MAP, id);
	
	stbi_set_flip_vertically_on_load(false);
	int width, height;
	for (unsigned int i = 0; i < files.size(); ++i)
	{
		auto filePath = directory + files[i];
		unsigned char* data = stbi_load(filePath.c_str(), &width, &height, nullptr, STBI_rgb_alpha);
		if (data)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		}
		else
		{
			throw std::runtime_error("Cubemap texture failed to load: " + files[i]);
		}
		stbi_image_free(data);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	// DSA
	/*glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &id);

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
	glTextureParameteri(id, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);*/
}

void Texture::Bind(GLenum textureIndex)
{
	// Non DSA
	glActiveTexture(textureIndex);
	glBindTexture(GL_TEXTURE_2D, id);

	// DSA
	//glBindTextureUnit(textureIndex, id);
}