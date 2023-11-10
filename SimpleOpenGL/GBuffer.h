#ifndef __G_BUFFER_H__
#define __G_BUFFER_H__

#include "Shader.h"
#include "Light.h"

#include <vector>

#include <memory>

class GBuffer
{
public:
	GBuffer();

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
	void Init();

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