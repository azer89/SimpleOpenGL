#ifndef APP_DEFERRED_SAAO
#define APP_DEFERRED_SAAO

#include "AppBase.h"
#include "Model.h"
#include "Texture.h"
#include "Light.h"

#include <memory>

class AppDeferredSSAO final : AppBase
{
public:
	int MainLoop() override;

private:
	void InitScene();
	void UpdateLightPositions();
	void InitLights();
	void RenderLights() const;
	void RenderScene(const Shader& shader) const;
	

private:
	// Scene
	std::unique_ptr<Model> sponzaModel{};

	// Lights
	std::unique_ptr<Shader> lightSphereShader{};
	std::vector<Light> lights{};
	std::vector<float> lightAngles{};
	std::vector<float> lightRadii{};
};

#endif