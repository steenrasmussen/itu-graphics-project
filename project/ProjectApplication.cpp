#include "ProjectApplication.h"

#include <imgui.h>

#include "ituGL/asset/ShaderLoader.h"
#include "ituGL/shader/Shader.h"
#include "ituGL/shader/ShaderProgram.h"

#include "glm/gtc/type_ptr.hpp"
#include "ituGL/asset/Texture2DLoader.h"
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/transform.hpp>
#include "glm/vec4.hpp"
#include "ituGL/shader/ShaderUniformCollection.h"
#include "ituGL/shader/Material.h"

static bool freeze_camera{false};

ProjectApplication::ProjectApplication()
        : Application(1024, 1024, "Project"), m_cameraPosition(0, 25, 25), m_cameraTranslationSpeed(20.0f),
          m_cameraRotationSpeed(0.5f), m_cameraEnabled(false), m_cameraEnablePressed(false),
          m_mousePosition(GetMainWindow().GetMousePosition(true)), m_ambientColor(0.0f), m_lightColor(0.0f),
          m_lightIntensity(0.0f), m_lightPosition(0.0f), m_specularExponent(100.0f) {
    m_isMeshShadersSupported = GLAD_GL_NV_mesh_shader != 0;
    if (!m_isMeshShadersSupported) {
        Terminate(-3, "This application requires a NVIDIA GPU with support for Mesh Shaders");
    }
}

void ProjectApplication::Initialize() {
    Application::Initialize();

    InitializeCamera();
    InitializeLights();
    InitializeMeshShaderPath();

    m_imGui.Initialize(GetMainWindow());

    m_gpuInfo.vendor = reinterpret_cast< char const * >(glGetString(GL_VENDOR));
    m_gpuInfo.renderer = reinterpret_cast< char const * >(glGetString(GL_RENDERER));
    m_gpuInfo.version = reinterpret_cast< char const * >(glGetString(GL_VERSION));

    if (m_isMeshShadersSupported) {
        // Max number of vertices that a mesh shader can output
        glGetIntegerv(GL_MAX_MESH_OUTPUT_VERTICES_NV, &meshShaderProperties.maxOutputVertices);
        // Max number of primitives that a mesh shader can output
        glGetIntegerv(GL_MAX_MESH_OUTPUT_PRIMITIVES_NV, &meshShaderProperties.maxOutputPrimitives);
        glGetIntegeri_v(GL_MAX_MESH_WORK_GROUP_SIZE_NV, 0, &meshShaderProperties.maxWorkGroupSize);
    }

    glEnable(GL_DEPTH_TEST);
}

void ProjectApplication::Update() {
    Application::Update();

    if (!freeze_camera) {
        m_debugCullingEnabled = false;
    } else if (!m_debugCullingEnabled) {
        m_cullingViewProjMatrix = m_camera.GetViewProjectionMatrix();
        m_cullingCameraPosition = m_cameraPosition;
        m_debugCullingEnabled = true;
    }

    // Update camera controller
    UpdateCamera();
}

void ProjectApplication::Render() {
    Application::Render();

    GetDevice().Clear(true, Color(0.0f, 0.0f, 0.0f, 1.0f), true, 1.0f);

    m_model.Draw();
    RenderGUI();

    GetMainWindow().SwapBuffers();
    GetDevice().PollEvents();
}

void ProjectApplication::Cleanup() {
    Application::Cleanup();

    m_imGui.Cleanup();
}

void ProjectApplication::RenderGUI() {
    m_imGui.BeginFrame();

    ImGui::Checkbox("Freeze camera", &freeze_camera);
    ImGui::Spacing();
    ImGui::Text("Total meshlets: %d", m_model.GetMeshletCount());
    ImGui::Text("Visible meshlets: %d", m_model.GetVisibleMeshletCount());
    ImGui::Spacing();

    if (ImGui::CollapsingHeader("GPU info")) {
        ImGui::Text("Vendor: %s", m_gpuInfo.vendor.c_str());
        ImGui::Text("Renderer: %s", m_gpuInfo.renderer.c_str());
        ImGui::Text("Version: %s", m_gpuInfo.version.c_str());
    }

    if (ImGui::CollapsingHeader("Lighting")) {
        // Add debug controls for light properties
        ImGui::ColorEdit3("Ambient color", &m_ambientColor[0]);
        ImGui::Separator();
        ImGui::DragFloat3("Light position", &m_lightPosition[0], 0.1f);
        ImGui::ColorEdit3("Light color", &m_lightColor[0]);
        ImGui::DragFloat("Light intensity", &m_lightIntensity, 0.05f, 0.0f, 100.0f);
        ImGui::Separator();
        ImGui::DragFloat("Specular exponent", &m_specularExponent, 1.0f, 0.0f, 1000.0f);
    }

    m_imGui.EndFrame();
}

void ProjectApplication::InitializeMeshShaderPath() {

    std::vector<const char *> taskShaderPaths;
    taskShaderPaths.push_back("shaders/ts.glsl");
    Shader taskShader = ShaderLoader(Shader::TaskShaderNv).Load(taskShaderPaths);

    std::vector<const char *> meshShaderPaths;
    meshShaderPaths.push_back("shaders/ms.glsl");
    Shader meshShader = ShaderLoader(Shader::MeshShaderNv).Load(meshShaderPaths);

    std::vector<const char *> fragmentShaderPaths;
    fragmentShaderPaths.push_back("shaders/fs.glsl");
    Shader fragmentShader = ShaderLoader(Shader::FragmentShader).Load(fragmentShaderPaths);

    shaderProgram = std::make_shared<ShaderProgram>();
    shaderProgram->BuildTaskMeshProgram(taskShader, meshShader, fragmentShader);

    ShaderUniformCollection::NameSet filteredUniforms;
    filteredUniforms.insert("WorldMatrix");
    filteredUniforms.insert("ViewProjMatrix");
    filteredUniforms.insert("AmbientColor");
    filteredUniforms.insert("LightColor");

    // Create reference material
    std::shared_ptr<Material> material = std::make_shared<Material>(shaderProgram, filteredUniforms);
    material->SetUniformValue("AmbientReflection", 1.0f);
    material->SetUniformValue("DiffuseReflection", 1.0f);
    material->SetUniformValue("SpecularReflection", 1.0f);
    material->SetUniformValue("SpecularExponent", 100.0f);

    ShaderProgram::Location worldMatrixLocation = shaderProgram->GetUniformLocation("WorldMatrix");
    ShaderProgram::Location viewProjMatrixLocation = shaderProgram->GetUniformLocation("ViewProjMatrix");
    ShaderProgram::Location cullingViewProjMatrixLocation = shaderProgram->GetUniformLocation("CullingViewProjMatrix");
    ShaderProgram::Location cullingCameraPositionLocation = shaderProgram->GetUniformLocation("CullingCameraPosition");
    ShaderProgram::Location ambientColorLocation = shaderProgram->GetUniformLocation("AmbientColor");
    ShaderProgram::Location lightColorLocation = shaderProgram->GetUniformLocation("LightColor");
    ShaderProgram::Location lightPositionLocation = shaderProgram->GetUniformLocation("LightPosition");
    ShaderProgram::Location cameraPositionLocation = shaderProgram->GetUniformLocation("CameraPosition");
    ShaderProgram::Location meshletCountLocation = shaderProgram->GetUniformLocation("MeshletCount");

    material->SetShaderSetupFunction([=](ShaderProgram &shaderProgram) {
        shaderProgram.SetUniform(worldMatrixLocation, glm::scale(glm::vec3(10.f)));
        shaderProgram.SetUniform(viewProjMatrixLocation, m_camera.GetViewProjectionMatrix());
        shaderProgram.SetUniform(meshletCountLocation, m_model.GetMeshletCount());

        if (m_debugCullingEnabled) {
            shaderProgram.SetUniform(cullingViewProjMatrixLocation, m_cullingViewProjMatrix);
            shaderProgram.SetUniform(cullingCameraPositionLocation, m_cullingCameraPosition);
        } else {
            shaderProgram.SetUniform(cullingViewProjMatrixLocation, m_camera.GetViewProjectionMatrix());
            shaderProgram.SetUniform(cullingCameraPositionLocation, m_cameraPosition);
        }

        // Set camera and light uniforms
        shaderProgram.SetUniform(ambientColorLocation, m_ambientColor);
        shaderProgram.SetUniform(lightColorLocation, m_lightColor * m_lightIntensity);
        shaderProgram.SetUniform(lightPositionLocation, m_lightPosition);
        shaderProgram.SetUniform(cameraPositionLocation, m_cameraPosition);
    });

    m_model.AddMaterial(material);
}

void ProjectApplication::UpdateCamera() {
    Window &window = GetMainWindow();

    // Update if camera is enabled (controlled by SPACE key)
    {
        bool enablePressed = window.IsKeyPressed(GLFW_KEY_SPACE);
        if (enablePressed && !m_cameraEnablePressed) {
            m_cameraEnabled = !m_cameraEnabled;

            window.SetMouseVisible(!m_cameraEnabled);
            m_mousePosition = window.GetMousePosition(true);
        }
        m_cameraEnablePressed = enablePressed;
    }

    if (!m_cameraEnabled)
        return;

    glm::mat4 viewTransposedMatrix = glm::transpose(m_camera.GetViewMatrix());
    glm::vec3 viewRight = viewTransposedMatrix[0];
    glm::vec3 viewForward = -viewTransposedMatrix[2];

    // Update camera translation
    {
        glm::vec2 inputTranslation(0.0f);

        if (window.IsKeyPressed(GLFW_KEY_A))
            inputTranslation.x = -1.0f;
        else if (window.IsKeyPressed(GLFW_KEY_D))
            inputTranslation.x = 1.0f;

        if (window.IsKeyPressed(GLFW_KEY_W))
            inputTranslation.y = 1.0f;
        else if (window.IsKeyPressed(GLFW_KEY_S))
            inputTranslation.y = -1.0f;

        inputTranslation *= m_cameraTranslationSpeed;
        inputTranslation *= GetDeltaTime();

        // Double speed if SHIFT is pressed
        if (window.IsKeyPressed(GLFW_KEY_LEFT_SHIFT))
            inputTranslation *= 2.0f;

        m_cameraPosition += inputTranslation.x * viewRight + inputTranslation.y * viewForward;
    }

//     Update camera rotation
    {
        glm::vec2 mousePosition = window.GetMousePosition(true);
        glm::vec2 deltaMousePosition = mousePosition - m_mousePosition;
        m_mousePosition = mousePosition;

        glm::vec3 inputRotation(-deltaMousePosition.x, deltaMousePosition.y, 0.0f);

        inputRotation *= m_cameraRotationSpeed;

        viewForward =
                glm::rotate(inputRotation.x, glm::vec3(0, 1, 0)) * glm::rotate(inputRotation.y, glm::vec3(viewRight)) *
                glm::vec4(viewForward, 0);
    }

    // Update view matrix
    m_camera.SetViewMatrix(m_cameraPosition, m_cameraPosition + viewForward);
}

void ProjectApplication::InitializeCamera() {
    // Set view matrix, from the camera position looking to the origin
    m_camera.SetViewMatrix(m_cameraPosition, glm::vec3(0.0f));

    // Set perspective matrix
    float aspectRatio = GetMainWindow().GetAspectRatio();
    m_camera.SetPerspectiveProjectionMatrix(1.0f, aspectRatio, 0.1f, 1000.0f);
}

void ProjectApplication::InitializeLights() {
    m_ambientColor = glm::vec3(0.25f);
    m_lightColor = glm::vec3(1.0f);
    m_lightIntensity = 0.7f;
    m_lightPosition = glm::vec3(-10.0f, 20.0f, 10.0f);
}
