#ifndef __APP_DEFERRED_H__
#define __APP_DEFERRED_H__

#include "AppBase.h"
#include "Shader.h"
#include "Model.h"
#include "Light.h"
#include "Texture.h"

class AppDeferred : AppBase
{
public:
	int MainLoop() override;

private:
	void InitScene();
	void InitLights(); 

	void RenderScene(const Shader& shader);
	void RenderLights();

private:

	// Scene
	std::unique_ptr<Model> foxModel;
	std::unique_ptr<Texture> grassTexture;
	unsigned int planeVAO = 0;

	// Lights
	std::unique_ptr<Shader> lightSphereShader;
	std::vector<Light> lights;
};

#endif