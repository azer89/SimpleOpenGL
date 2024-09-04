#ifndef PIPELINE_BLOOM
#define PIPELINE_BLOOM

#include "Shader.h"

#include "glm/glm.hpp"

#include <memory>
#include <array>

class PipelineBloom
{
public:
	PipelineBloom(uint32_t blurIteration_);

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
	std::array<uint32_t, 2> colorBuffers_{};

	// Blur pass
	bool horizontal_{};
	std::array<uint32_t, 2> pingpongFBO_{};
	std::array<uint32_t, 2> pingpongColorbuffers_{};

	// Full screen quad
	uint32_t quadVAO_{};
	uint32_t quadVBO_{};
};

#endif