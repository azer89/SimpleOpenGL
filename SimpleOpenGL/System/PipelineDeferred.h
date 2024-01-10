#ifndef PIPELINE_DEFERRED
#define PIPELINE_DEFERRED

#include "Shader.h"
#include "Light.h"

#include <vector>
#include <memory>

class PipelineDeferred
{
public:
	PipelineDeferred(
		const char* gBufferVertexShader,
		const char* gBufferFragmentShader,
		const char* lightingVertexShader,
		const char* lightingFragmentShader
	);

	// Call these in order
	void StartGeometryPass(const glm::mat4& projection, const glm::mat4& view);
	void EndGeometryPass();
	void StartLightingPass(const std::vector<Light>& lights, const glm::vec3& cameraPosition);
	void Blit();

	Shader* GetGeometryShader()
	{
		if (gBufferShader == nullptr)
		{
			return nullptr;
		}
		return gBufferShader.get();
	}

private:
	void Init(
		const char* gBufferVertexShader,
		const char* gBufferFragmentShader,
		const char* lightingVertexShader,
		const char* lightingFragmentShader
	);

	std::unique_ptr<Shader> gBufferShader;
	std::unique_ptr<Shader> lightingShader;

	unsigned int gBuffer;
	unsigned int gPosition;
	unsigned int gNormal;
	unsigned int gAlbedoSpec;

	unsigned int rboDepth;
	unsigned int quadVAO;
};

#endif
