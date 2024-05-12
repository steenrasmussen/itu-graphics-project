#ifndef PROJECTAPPLICATION_H
#define PROJECTAPPLICATION_H

#include <ituGL/application/Application.h>
#include <memory>

#include "ituGL/utils/DearImGui.h"
#include "ituGL/shader/ShaderProgram.h"
#include "ituGL/camera/Camera.h"
#include "ituGL/geometry/MeshletModel.h"

struct GpuInfo {
    std::string vendor;
    std::string renderer;
    std::string version;
};

struct MeshShaderPropterties {
    GLint maxOutputVertices;
    GLint maxOutputPrimitives;
    GLint maxWorkGroupSize;
};

class ProjectApplication : public Application {
public:
    ProjectApplication();

    ~ProjectApplication() override = default;

protected:
    void Initialize() override;

    void Update() override;

    void Render() override;

    void Cleanup() override;

private:
    void InitializeCamera();
    void InitializeLights();

    void UpdateCamera();

    void RenderGUI();

    void InitializeMeshShaderPath();

    // Mouse position for camera controller
    glm::vec2 m_mousePosition;

    DearImGui m_imGui;

    GpuInfo m_gpuInfo;
    bool m_isMeshShadersSupported{};
    MeshShaderPropterties meshShaderProperties = {};

    std::shared_ptr<ShaderProgram> shaderProgram;

    // Camera controller parameters
    Camera m_camera;
    glm::vec3 m_cameraPosition;
    float m_cameraTranslationSpeed;
    float m_cameraRotationSpeed;
    bool m_cameraEnabled;
    bool m_cameraEnablePressed;

    MeshletModel m_model;

    bool m_debugCullingEnabled{false};
    glm::vec3 m_cullingCameraPosition{0.0f};
    glm::mat4 m_cullingViewProjMatrix{1.0f};

    // Add light variables
    glm::vec3 m_ambientColor;
    glm::vec3 m_lightColor;
    float m_lightIntensity;
    glm::vec3 m_lightPosition;

    // Specular exponent debug
    float m_specularExponent;
};


#endif //PROJECTAPPLICATION_H
