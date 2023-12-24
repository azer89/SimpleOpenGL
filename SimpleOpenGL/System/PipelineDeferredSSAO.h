#ifndef PIPELINE_DEFERRED_SSAO
#define PIPELINE_DEFERRED_SSAO

#include <vector>
#include <memory>
#include "glm/glm.hpp"

#include "Shader.h"
#include "Light.h"

class PipelineDeferredSSAO
{
public:
	PipelineDeferredSSAO(
		const char* geomVertexShader,
		const char* geomFragmentShader,
		const char* lightVertexShader,
		const char* lightFragmentShader,
		const char* ssaoVertexShader,
		const char* ssaoFragmentShader,
		const char* blurVertexShader,
		const char* blurFragmentShader,
		int kernelSize,
		int noiseSize
	);

	void StartGeometryPass(const glm::mat4& projection, const glm::mat4& view);
	void EndGeometryPass();
	void StartSSAOPass
	(
		const glm::mat4& projection,
		int kernelSize,
		float radius,
		float bias
	);
	void StartBlurPass();
	void StartLightingPass(const std::vector<Light>& lights, 
		const glm::mat4& cameraview,
		const glm::vec3& cameraPosition);
	void Blit();

	Shader* GetGeometryShader()
	{
		if (shaderGeometry == nullptr)
		{
			return nullptr;
		}
		return shaderGeometry.get();
	}

private:
	void Init(
		const char* geomVertexShader,
		const char* geomFragmentShader,
		const char* lightVertexShader,
		const char* lightFragmentShader,
		const char* ssaoVertexShader,
		const char* ssaoFragmentShader,
		const char* blurVertexShader,
		const char* blurFragmentShader,
		int kernelSize,
		int noiseSize
	);

	void RenderQuad();

	std::vector<glm::vec3> ssaoKernel;

	std::unique_ptr<Shader> shaderGeometry;
	std::unique_ptr<Shader> shaderLighting;
	std::unique_ptr<Shader> shaderSSAO;
	std::unique_ptr<Shader> shaderBlur;

	unsigned int quadVAO;

	unsigned int gBufferFBO;
	unsigned int gPositionTexture;
	unsigned int gNormalTexture;
	unsigned int gAlbedoTexture;
	unsigned int depthRBO;

	unsigned int ssaoFBO;
	unsigned int ssaoColorTexture;
	unsigned int ssaoBlurTexture;
	unsigned int ssaoBlurFBO;

	unsigned int noiseTexture;
};

#endif