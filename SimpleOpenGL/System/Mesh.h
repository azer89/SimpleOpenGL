#ifndef MESH
#define MESH

#include <vector>
#include <unordered_map>

#include "glm/glm.hpp"

#include "Shader.h"
#include "Texture.h"
#include "TextureMapper.h"

#define MAX_BONE_INFLUENCE 4

struct Vertex
{
	glm::vec3 Position; // Position
	glm::vec3 Normal; // Normal
	glm::vec2 TexCoords; // TexCoords
	glm::vec3 Tangent; // Tangent
	glm::vec3 Bitangent; // Bitangent
	int m_BoneIDs[MAX_BONE_INFLUENCE]; // Bone indexes which will influence this vertex
	float m_Weights[MAX_BONE_INFLUENCE]; // Weights from each bone
};

class Mesh
{
public:
	Mesh() = default;
	
	// Constructors
	Mesh(
		std::vector<Vertex>&& vertices, 
		std::vector<unsigned int>&& indices, 
		std::unordered_map<TextureType, Texture>&& textures);

	// Cleanup resource
	void Delete() const;

	void AddTextureIfEmpty(TextureType tType, const std::string& filePath);

	// Render the mesh
	void Draw(const Shader& shader, bool skipTexture);

public:
	// Mesh Data
	std::vector<Vertex>	vertices{};
	std::vector<unsigned int> indices{};
	std::unordered_map<TextureType, Texture> textureMap{};
	unsigned int VAO{};


private:
	// Initializes all the buffer objects/arrays
	void SetupMesh();

private:
	// Render data 
	unsigned int VBO{};
	unsigned int EBO{};
};

#endif
