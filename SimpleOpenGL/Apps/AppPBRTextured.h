#ifndef APP_PBR_TEXTURED
#define APP_PBR_TEXTURED

#include "AppBase.h"
#include "Model.h"
#include "Light.h"

#include <memory>

class AppPBRTextured final : AppBase
{
public:
	int MainLoop() override;

private:
	void InitScene();
	void RenderScene(const Shader& shader) const;

private:
	// Scene
	std::unique_ptr<Model> dragonModel{};

	// Lights
	std::unique_ptr<Shader> lightSphereShader{};
	std::vector<Light> lights{};
	std::vector<float> lightAngles{};
	std::vector<float> lightRadii{};
};

#endif