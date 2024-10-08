#ifndef APP_SAAO
#define APP_SAAO

#include "AppBase.h"
#include "Model.h"
#include "Texture.h"
#include "Light.h"

#include <memory>

class AppSSAO final : AppBase
{
public:
	int MainLoop() override;

private:
	void InitScene();
	void RenderScene(const Shader& shader) const;

	void InitLights();
	void RenderLights();

private:
	// Scene
	std::unique_ptr<Model> sponzaModel{};
	std::unique_ptr<Model> renderModel{};

	// Lights
	std::unique_ptr<Shader> lightSphereShader{};
	std::vector<Light> lights{};
	std::vector<float> lightAngles{};
	std::vector<float> lightRadii{};
};

#endif