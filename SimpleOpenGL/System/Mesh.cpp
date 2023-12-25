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

void Mesh::AddTextureIfEmpty(TextureType tType, const std::string& filePath)
{
	if (textureMap.contains(tType)) // C++20 Feature
	{
		return;
	}
	Texture texture(tType, filePath);
	texture.CreateFromImageFile(filePath);
	textureMap[tType] = texture;
}

// Render the mesh
void Mesh::Draw(const Shader& shader, bool skipTexture)
{
	if (!skipTexture)
	{
		// Currently only supports one texture per type
		for (unsigned int i = 0; i < TextureMapper::NUM_TEXTURE_TYPE; ++i) // Iterate over TextureType elements
		{
			TextureType tType = static_cast<TextureType>(i + 1); // Casting

			if (!textureMap.contains(tType))
			{
				continue;
			}

			Texture& texture = textureMap[tType];
			std::string name = TextureMapper::GetTextureString(tType) + "1";

			glUniform1i(glGetUniformLocation(shader.ID, name.c_str()), i);
			glBindTextureUnit(i, texture.GetID()); // DSA
		}
	}

	// Draw mesh
	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(indices.size()), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

void Mesh::SetupMesh()
{
	// DSA
	glCreateBuffers(1, &VBO);
	glNamedBufferStorage(VBO, sizeof(Vertex) * vertices.size(), &vertices[0], GL_DYNAMIC_STORAGE_BIT);
	
	glCreateBuffers(1, &EBO);
	glNamedBufferStorage(EBO, sizeof(unsigned int) * indices.size(),  &indices[0], GL_DYNAMIC_STORAGE_BIT);

	glCreateVertexArrays(1, &VAO);
	glVertexArrayVertexBuffer(VAO, 0, VBO, 0, sizeof(Vertex));
	glVertexArrayElementBuffer(VAO, EBO);
	
	glEnableVertexArrayAttrib(VAO, 0);
	glEnableVertexArrayAttrib(VAO, 1);
	glEnableVertexArrayAttrib(VAO, 2);
	glEnableVertexArrayAttrib(VAO, 3);
	glEnableVertexArrayAttrib(VAO, 4);
	glEnableVertexArrayAttrib(VAO, 5);
	glEnableVertexArrayAttrib(VAO, 6);

	glVertexArrayAttribFormat(VAO, 0, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, Position));
	glVertexArrayAttribFormat(VAO, 1, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, Normal));
	glVertexArrayAttribFormat(VAO, 2, 2, GL_FLOAT, GL_FALSE, offsetof(Vertex, TexCoords));
	glVertexArrayAttribFormat(VAO, 3, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, Tangent));
	glVertexArrayAttribFormat(VAO, 4, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, Bitangent));
	glVertexArrayAttribIFormat(VAO, 5, 4, GL_INT, offsetof(Vertex, m_BoneIDs));
	glVertexArrayAttribFormat(VAO, 6, 4, GL_FLOAT, GL_FALSE, offsetof(Vertex, m_Weights));

	glVertexArrayAttribBinding(VAO, 0, 0);
	glVertexArrayAttribBinding(VAO, 1, 0);
	glVertexArrayAttribBinding(VAO, 2, 0);
	glVertexArrayAttribBinding(VAO, 3, 0);
	glVertexArrayAttribBinding(VAO, 4, 0);
	glVertexArrayAttribBinding(VAO, 5, 0);
	glVertexArrayAttribBinding(VAO, 6, 0);

#if _DEBUG
	std::cout << "Mesh vertex count " << vertices.size() << '\n';
	for (auto& it : textureMap)
	{
		std::cout << "Texture " << static_cast<int>(it.first) << ", " << it.second.GetName() << '\n';
	}
	std::cout << '\n';
#endif
}