
#include "AppIrradianceCubemap.h"

#include "AppSettings.h"
#include "Shader.h"
#include "Texture.h"

int AppIrradianceCubemap::MainLoop()
{
    glEnable(GL_DEPTH_TEST);

    // Init
    glDepthFunc(GL_LEQUAL); // Set depth function to less than AND equal for skybox depth trick.
    Shader pbrShader("IrradianceCubemap//pbr.vertex", "IrradianceCubemap//pbr.fragment");
    Shader equirectangularToCubemapShader("IrradianceCubemap//cubemap.vertex", "IrradianceCubemap//equirectangular_to_cubemap.fragment");
    Shader irradianceShader("IrradianceCubemap//cubemap.vertex", "IrradianceCubemap//irradiance_convolution.fragment");
    Shader backgroundShader("IrradianceCubemap//background.vertex", "IrradianceCubemap//background.fragment");

    pbrShader.Use();
    pbrShader.SetInt("irradianceMap", 0);
    pbrShader.SetVec3("albedo", 0.5f, 0.0f, 0.0f);
    pbrShader.SetFloat("ao", 1.0f);

    backgroundShader.Use();
    backgroundShader.SetInt("environmentMap", 0);

    // PBR setup framebuffer
    unsigned int captureFBO;
    unsigned int captureRBO;
    glGenFramebuffers(1, &captureFBO);
    glGenRenderbuffers(1, &captureRBO);

    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);

    // PBR load the HDR environment map
    Texture hdrTexture;
    hdrTexture.CreateFromHDRFile(AppSettings::TextureFolder + "hdr//newport_loft.hdr");
    
    while (!GLFWWindowShouldClose())
    {
        ProcessLoop(
            glm::vec4(0.2f, 0.3f, 0.3f, 1.0f),
            GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT
        );

        // Code

        SwapBuffers();
    }

    Terminate();

    return 0;
}
