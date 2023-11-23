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
	TEXTURE_EMISSIVE = 6,
};

namespace TextureMapper
{
	// Corresponds to yhe number of elements in TextureType
	constexpr unsigned int NUM_TEXTURE_TYPE = 6; 

	// This vector is a priority list
	static std::vector<aiTextureType> aiTTypeSearchOrder = 
	{
		// Diffuse
		aiTextureType_DIFFUSE,

		// Specular
		aiTextureType_SPECULAR,
		aiTextureType_METALNESS,
		
		// Normal
		aiTextureType_NORMALS,
		
		// Roughness shininess
		aiTextureType_DIFFUSE_ROUGHNESS,
		aiTextureType_SHININESS,
		
		// AO
		aiTextureType_AMBIENT_OCCLUSION,
		aiTextureType_LIGHTMAP,

		// Emissive
		aiTextureType_EMISSIVE
	};

	static std::unordered_map<aiTextureType, TextureType> assimpTextureToTextureType =
	{
		// Diffuse
		{aiTextureType_DIFFUSE, TEXTURE_DIFFUSE},

		// Specular
		{aiTextureType_SPECULAR, TEXTURE_METALNESS},
		{aiTextureType_METALNESS, TEXTURE_METALNESS},
		
		// Normal
		{aiTextureType_NORMALS, TEXTURE_NORMAL},
		
		// Roughness shininess
		{aiTextureType_DIFFUSE_ROUGHNESS, TEXTURE_ROUGHNESS},
		{aiTextureType_SHININESS, TEXTURE_ROUGHNESS},
		
		// AO
		{aiTextureType_AMBIENT_OCCLUSION, TEXTURE_AO},
		{aiTextureType_LIGHTMAP, TEXTURE_AO},

		// Emissive
		{aiTextureType_EMISSIVE, TEXTURE_EMISSIVE}
	};

	static std::unordered_map<TextureType, std::string> textureTypeToString =
	{
		{TEXTURE_DIFFUSE, "texture_diffuse"},
		{TEXTURE_NORMAL, "texture_normal"},
		{TEXTURE_METALNESS, "texture_metalness"},
		{TEXTURE_ROUGHNESS, "texture_roughness"},
		{TEXTURE_AO, "texture_ao"},
		{TEXTURE_EMISSIVE, "texture_emissive"},
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
