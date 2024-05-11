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
struct Matricies {
    glm::mat4 ViewProjectionMatrix;
    glm::mat4 ModelMatrix;
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

    void UpdateCamera();

    void RenderGUI();

    void InitializeMeshShaderPath();

    // Mouse position for camera controller
    glm::vec2 m_mousePosition;

    DearImGui m_imGui;

    GpuInfo m_gpuInfo;
    bool m_isMeshShadersSupported{};
    MeshShaderPropterties meshShaderProperties = {};

    std::shared_ptr<ShaderProgram> shaderProgramPtr;

    // Camera controller parameters
    Camera m_camera;
    glm::vec3 m_cameraPosition;
    float m_cameraTranslationSpeed;
    float m_cameraRotationSpeed;
    bool m_cameraEnabled;
    bool m_cameraEnablePressed;

    MeshletModel m_model;
    ShaderProgram::Location m_worldMatrixLocation{-1};
    ShaderProgram::Location m_viewProjMatrixLocation{-1};

    ShaderProgram::Location m_cullingCameraPositionLocation{-1};

    Camera m_cullingCamera;
    glm::vec3 m_cullingCameraPosition;
};


#endif //PROJECTAPPLICATION_H
