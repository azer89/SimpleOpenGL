#ifndef __MODEL_H__
#define __MODEL_H__

#include "Mesh.h"
#include "Shader.h"

#include <string>
#include <vector>

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

class Model
{
public:
	// Model data 
	std::vector<Texture> textures_loaded; // Stores all the textures loaded so far, optimization to make sure textures aren't loaded more than once.
	std::vector<Mesh>  meshes;
	std::string directory;
	bool gammaCorrection;

	// Constructor, expects a filepath to a 3D model.
	Model(const std::string& path, bool gamma = false);

	// Draws the model, and thus all its meshes
	void Draw(const Shader& shader);

private:
	// Loads a model with supported ASSIMP extensions from file and stores the resulting meshes in the meshes vector.
	void LoadModel(std::string const& path);

	// Processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats this process on its children nodes (if any).
	void ProcessNode(aiNode* node, const aiScene* scene);

	Mesh ProcessMesh(aiMesh* mesh, const aiScene* scene);

	// Checks all material textures of a given type and loads the textures if they're not loaded yet.
	// The required info is returned as a Texture struct.
	std::vector<Texture> LoadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName);
};

#endif

