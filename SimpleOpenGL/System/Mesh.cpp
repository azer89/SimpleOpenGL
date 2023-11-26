#include "Mesh.h"
#include "TextureMapper.h"

#include "glad/glad.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <iostream>

// Constructor
Mesh::Mesh(std::vector<Vertex>&& _vertices, 
	std::vector<unsigned int>&& _indices, 
	std::unordered_map<TextureType, Texture>&& _textures) :
	vertices(std::move(_vertices)),
	indices(std::move(_indices)),
	textureMap(std::move(_textures))
{
	SetupMesh();
}

// Constructor
Mesh::Mesh(const std::vector<Vertex>& _vertices, 
	const std::vector<unsigned int>& _indices, 
	const std::unordered_map<TextureType, Texture>& _textures) :
	vertices(_vertices),
	indices(_indices),
	textureMap(_textures)
{
	SetupMesh();
}

void Mesh::Delete()
{
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
}

// Render the mesh
void Mesh::Draw(const Shader& shader, bool skipTexture)
{
	if (!skipTexture)
	{
		// Currently only supports one texture per type
		for (unsigned int i = 0; i < TextureMapper::NUM_TEXTURE_TYPE; ++i) // Iterate over TextureType elements
		{
			glActiveTexture(GL_TEXTURE0 + i);
			TextureType tType = static_cast<TextureType>(i + 1); // Casting
			if (textureMap.find(tType) == textureMap.end())
			{
				glBindTexture(GL_TEXTURE_2D, 0); // Flush
				continue;
			}

			Texture& texture = textureMap[tType];
			std::string name = TextureMapper::GetTextureString(tType) + "1";

			glUniform1i(glGetUniformLocation(shader.ID, name.c_str()), i);
			glBindTexture(GL_TEXTURE_2D, texture.GetID());
		}
	}

	// Draw mesh
	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(indices.size()), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	// Set back to defaults once configured.
	glActiveTexture(GL_TEXTURE0);
}

void Mesh::SetupMesh()
{
	// Create buffers/arrays
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);
	// Load data into vertex buffers
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	// A great thing about structs is that their memory layout is sequential for all its items.
	// The effect is that we can simply pass a pointer to the struct and it translates perfectly to a glm::vec3/2 array which
	// again translates to 3/2 floats which translates to a byte array.
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

	// Set the vertex attribute pointers
	// Vertex Positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
	// Vertex normals
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
	// Vertex texture coords
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
	// Vertex tangent
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Tangent));
	// Vertex bitangent
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Bitangent));
	// IDs
	glEnableVertexAttribArray(5);
	glVertexAttribIPointer(5, 4, GL_INT, sizeof(Vertex), (void*)offsetof(Vertex, m_BoneIDs));

	// Weights
	glEnableVertexAttribArray(6);
	glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, m_Weights));
	glBindVertexArray(0);
}