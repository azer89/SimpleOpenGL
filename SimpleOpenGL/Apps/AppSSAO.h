#ifndef __APP_SAAO_H_
#define __APP_SAAO_H_

#include "AppBase.h"
#include "Model.h"
#include "Texture.h"

#include <memory>

class AppSSAO : AppBase
{
public:
	int MainLoop() override;

private:
	void InitScene();

	void RenderScene(const Shader& shader);
	void RenderQuad();

private:
	// Scene
	std::unique_ptr<Model> foxModel;
	std::unique_ptr<Texture> grassTexture;
	unsigned int planeVAO = 0;
};

#endif