#ifndef __TEXTURE_TYPE_H__
#define __TEXTURE_TYPE_H__

#include "assimp//material.h"

#include <unordered_map>
#include <string>

enum TextureType
{
	TEXTURE_NONE = 0,
	TEXTURE_DIFFUSE = 1,
	TEXTURE_NORMAL = 2,
	TEXTURE_METALNESS = 3,
	TEXTURE_ROUGHNESS = 4,
	TEXTURE_AO = 5,
};

namespace TextureMapper
{
	// Corresponds to yhe number of elements in TextureType
	constexpr unsigned int NUM_TEXTURE_TYPE = 5; 

	static std::vector<aiTextureType> aiTTypes = 
	{
		aiTextureType_DIFFUSE,
		aiTextureType_SPECULAR,
		aiTextureType_NORMALS,
		aiTextureType_METALNESS,
		aiTextureType_DIFFUSE_ROUGHNESS,
		aiTextureType_LIGHTMAP,
		aiTextureType_AMBIENT_OCCLUSION
		//aiTextureType_EMISSIVE
	};

	static std::unordered_map<aiTextureType, TextureType> assimpTextureToTextureType =
	{
		{aiTextureType_DIFFUSE, TEXTURE_DIFFUSE},
		{aiTextureType_SPECULAR, TEXTURE_METALNESS},
		{aiTextureType_NORMALS, TEXTURE_NORMAL},
		{aiTextureType_METALNESS, TEXTURE_METALNESS},
		{aiTextureType_DIFFUSE_ROUGHNESS, TEXTURE_ROUGHNESS},
		{aiTextureType_LIGHTMAP, TEXTURE_AO},
		{aiTextureType_AMBIENT_OCCLUSION, TEXTURE_AO}
		//aiTextureType_EMISSIVE
	};

	static std::unordered_map<TextureType, std::string> textureTypeToString =
	{
		{TEXTURE_DIFFUSE, "texture_diffuse"},
		{TEXTURE_NORMAL, "texture_normal"},
		{TEXTURE_METALNESS, "texture_metalness"},
		{TEXTURE_ROUGHNESS, "texture_roughness"},
		{TEXTURE_AO, "texture_ao"},
	};

	static TextureType GetTextureType(aiTextureType aiTType)
	{
		if (TextureMapper::assimpTextureToTextureType.find(aiTType) == TextureMapper::assimpTextureToTextureType.end())
		{
			return TEXTURE_NONE;
		}
		return TextureMapper::assimpTextureToTextureType[aiTType];
	}

	static std::string GetTextureString(TextureType tType)
	{
		if (TextureMapper::textureTypeToString.find(tType) == TextureMapper::textureTypeToString.end())
		{
			return "";
		}
		return TextureMapper::textureTypeToString[tType];
	}
};


#endif
