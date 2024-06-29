#ifndef PIPELINE_BLOOM
#define PIPELINE_BLOOM

#include "Shader.h"

#include "glm/glm.hpp"

#include <memory>

class PipelineBloom
{
public:
	PipelineBloom(unsigned blurIteration_);

	// 1
	void StartFirstPass(
		const glm::mat4& projection, 
		const glm::mat4& view,
		const glm::vec3& cameraPosition,
		const glm::vec3& lightPosition);
	
	// 2
	void EndFirstPass();
	
	// 3
	void StartBlurPass();
	
	// 4
	void RenderComposite();

	[[nodiscard]] Shader* GetMainShader() const { return mainShader.get(); }

private:
	void InitQuad();
	void RenderQuad();

private:
	std::unique_ptr<Shader> mainShader{};
	std::unique_ptr<Shader> shaderBlur{};
	std::unique_ptr<Shader> shaderFinal{};

	unsigned blurIteration{};

	// First pass
	unsigned int hdrFBO{};
	unsigned int colorBuffers[2]{};

	// Blur pass
	bool horizontal{};
	unsigned int pingpongFBO[2]{};
	unsigned int pingpongColorbuffers[2]{};

	// Full screen quad
	unsigned int quadVAO{};
	unsigned int quadVBO{};
};

#endif