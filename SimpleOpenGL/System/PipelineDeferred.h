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

	// Get G-Buffer shader
	[[nodiscard]] Shader* GetGBufferShader() const
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

	std::unique_ptr<Shader> gBufferShader{};
	std::unique_ptr<Shader> lightingShader{};

	unsigned int gBuffer = 0;
	unsigned int gPosition = 0;
	unsigned int gNormal = 0;
	unsigned int gAlbedoSpec = 0;

	unsigned int rboDepth = 0;
	unsigned int quadVAO = 0;
};

#endif
