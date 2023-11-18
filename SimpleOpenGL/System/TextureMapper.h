#ifndef __TEXTURE_TYPE_H__
#define __TEXTURE_TYPE_H__

#include "assimp//material.h"

#include <unordered_map>
#include <string>

namespace TextureMapper
{
	static std::vector<aiTextureType> aiTTypes = 
	{
		aiTextureType_DIFFUSE,
		aiTextureType_SPECULAR,
		aiTextureType_NORMALS,
		aiTextureType_METALNESS,
		aiTextureType_DIFFUSE_ROUGHNESS,
		aiTextureType_LIGHTMAP,
		aiTextureType_AMBIENT_OCCLUSION,
		aiTextureType_EMISSIVE
	};

	static std::unordered_map<aiTextureType, std::string> assimpTextureToString =
	{
		{aiTextureType_DIFFUSE, "texture_diffuse"},
		{aiTextureType_SPECULAR, "texture_specular"},
		{aiTextureType_NORMALS, "texture_normal"},
		{aiTextureType_METALNESS, "texture_metalness"},
		{aiTextureType_DIFFUSE_ROUGHNESS, "texture_roughness"},
		{aiTextureType_LIGHTMAP, "texture_ao"},
		{aiTextureType_AMBIENT_OCCLUSION, "texture_ao"},
		{aiTextureType_EMISSIVE, "texture_emissive"}
	};

	static std::string GetTextureString(aiTextureType aiTType)
	{
		if (TextureMapper::assimpTextureToString.find(aiTType) == TextureMapper::assimpTextureToString.end())
		{
			return "";
		}
		return TextureMapper::assimpTextureToString[aiTType];
	}
};


#endif
