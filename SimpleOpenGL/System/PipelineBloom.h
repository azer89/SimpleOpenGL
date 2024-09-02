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
		const glm::vec3& lightPosition) const;
	
	// 2
	void EndFirstPass() const;
	
	// 3
	void StartBlurPass();
	
	// 4
	void RenderComposite() const;

	[[nodiscard]] Shader* GetMainShader() const { return mainShader_.get(); }

private:
	void InitQuad();
	void RenderQuad() const;

private:
	std::unique_ptr<Shader> mainShader_{};
	std::unique_ptr<Shader> shaderBlur_{};
	std::unique_ptr<Shader> shaderFinal_{};

	uint32_t blurIteration_{};

	// First pass
	uint32_t hdrFBO_{};
	uint32_t colorBuffers_[2]{};

	// Blur pass
	bool horizontal_{};
	uint32_t pingpongFBO_[2]{};
	uint32_t pingpongColorbuffers_[2]{};

	// Full screen quad
	uint32_t quadVAO_{};
	uint32_t quadVBO_{};
};

#endif