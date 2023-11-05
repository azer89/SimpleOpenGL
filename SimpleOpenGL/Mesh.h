#ifndef __MESH_H__
#define __MESH_H__

#include <glm/glm.hpp>
#include <vector>

#include "Shader.h"
#include "Texture.h"

#define MAX_BONE_INFLUENCE 4

struct Vertex
{
	// Position
	glm::vec3 Position;
	// Normal
	glm::vec3 Normal;
	// TexCoords
	glm::vec2 TexCoords;
	// Tangent
	glm::vec3 Tangent;
	// Bitangent
	glm::vec3 Bitangent;
	// Bone indexes which will influence this vertex
	int m_BoneIDs[MAX_BONE_INFLUENCE];
	// Weights from each bone
	float m_Weights[MAX_BONE_INFLUENCE];
};

class Mesh
{
public:
	// Mesh Data
	std::vector<Vertex>	vertices;
	std::vector<unsigned int> indices;
	std::vector<Texture> textures;
	unsigned int VAO;

	// Constructor
	Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures);

	// Render the mesh
	void Draw(Shader& shader);

private:
	// Render data 
	unsigned int VBO;
	unsigned int EBO;

	// Initializes all the buffer objects/arrays
	void SetupMesh();
};

#endif
