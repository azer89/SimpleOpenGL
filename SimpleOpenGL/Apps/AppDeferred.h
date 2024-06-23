#ifndef APP_DEFERRED
#define APP_DEFERRED

#include "AppBase.h"
#include "Shader.h"
#include "Model.h"
#include "Light.h"
#include "Texture.h"

class AppDeferred final : AppBase
{
public:
	int MainLoop() override;

private:
	void InitScene();
	void InitLights(); 

	void UpdateLightPositions();

	void RenderScene(const Shader& shader);
	void RenderLights();

private:

	// Scene
	std::unique_ptr<Model> foxModel{};
	std::unique_ptr<Texture> grassTexture{};
	unsigned int planeVAO{};

	// Lights
	std::unique_ptr<Shader> lightSphereShader{};
	std::vector<Light> lights{};
	std::vector<float> lightAngles{};
	std::vector<float> lightRadii{};
};

#endif