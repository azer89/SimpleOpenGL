#ifndef MODEL
#define MODEL

#include "Mesh.h"
#include "Shader.h"

#include <string>
#include <vector>
#include <unordered_map>

#include "assimp/Importer.hpp"
#include "assimp/scene.h"

class Model
{
private:
	// Model data 
	std::unordered_map<std::string, Texture> textureMap{}; // key is the filename
	std::vector<Mesh> meshes{};
	std::string directory{};

public:
	// Constructor, expects a filepath to a 3D model.
	Model(const std::string& path);

	// Destructor
	~Model();

	// Draws the model, and thus all its meshes
	void Draw(const Shader& shader, bool skipTexture = false);

	void AddTextureIfEmpty(TextureType tType, const std::string& filePath);

private:
	// Loads a model with supported ASSIMP extensions from file and stores the resulting meshes in the meshes vector.
	void LoadModel(std::string const& path);

	// Processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats this process on its children nodes (if any).
	void ProcessNode(const aiNode* node, const aiScene* scene, const glm::mat4& parentTransform);

	Mesh ProcessMesh(const aiMesh* mesh, const aiScene* scene, const glm::mat4& transform);
};

#endif

