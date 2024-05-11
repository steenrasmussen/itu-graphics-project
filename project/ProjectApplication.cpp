#include "ProjectApplication.h"

#include <imgui.h>

#include "ituGL/asset/ShaderLoader.h"
#include "ituGL/shader/Shader.h"
#include "ituGL/shader/ShaderProgram.h"

#include "glm/gtc/type_ptr.hpp"
#include "ituGL/asset/Texture2DLoader.h"
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/transform.hpp>

static bool freeze_camera{false};

ProjectApplication::ProjectApplication()
        : Application(1024, 1024, "Project"), m_cameraPosition(0, 20, 20), m_cullingCameraPosition(0, 20, -20), m_cameraTranslationSpeed(20.0f),
          m_cameraRotationSpeed(0.5f), m_cameraEnabled(false), m_cameraEnablePressed(false),
          m_mousePosition(GetMainWindow().GetMousePosition(true)) {
}

void ProjectApplication::Initialize() {
    Application::Initialize();

    InitializeCamera();

    m_imGui.Initialize(GetMainWindow());

    m_gpuInfo.vendor = reinterpret_cast< char const * >(glGetString(GL_VENDOR));
    m_gpuInfo.renderer = reinterpret_cast< char const * >(glGetString(GL_RENDERER));
    m_gpuInfo.version = reinterpret_cast< char const * >(glGetString(GL_VERSION));

    m_isMeshShadersSupported = GLAD_GL_NV_mesh_shader != 0;

    if (m_isMeshShadersSupported) {
        // Max number of vertices that a mesh shader can output
        glGetIntegerv(GL_MAX_MESH_OUTPUT_VERTICES_NV, &meshShaderProperties.maxOutputVertices);
        // Max number of primitives that a mesh shader can output
        glGetIntegerv(GL_MAX_MESH_OUTPUT_PRIMITIVES_NV, &meshShaderProperties.maxOutputPrimitives);
        glGetIntegeri_v(GL_MAX_MESH_WORK_GROUP_SIZE_NV, 0, &meshShaderProperties.maxWorkGroupSize);
    }

    InitializeMeshShaderPath();

    glEnable(GL_DEPTH_TEST);

    // Debugging
//    glEnable(GL_CULL_FACE);
//    glCullFace(GL_BACK);
//    glFrontFace(GL_CCW);
}

void ProjectApplication::Update() {
    Application::Update();

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

    ImGui::Text("Vendor: %s", m_gpuInfo.vendor.c_str());
    ImGui::Text("Renderer: %s", m_gpuInfo.renderer.c_str());
    ImGui::Text("Version: %s", m_gpuInfo.version.c_str());

    ImGui::Text("Mesh shaders supported: %s", m_isMeshShadersSupported ? "Yes" : "No");

    if (m_isMeshShadersSupported) {
        ImGui::Text("Max number of output vertices: %d", meshShaderProperties.maxOutputVertices);
        ImGui::Text("Max number of output primitives: %d", meshShaderProperties.maxOutputPrimitives);
        ImGui::Text("Max Work Group size: %d", meshShaderProperties.maxWorkGroupSize);\
        ImGui::Checkbox("Freeze camera", &freeze_camera);
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

    shaderProgramPtr = std::make_shared<ShaderProgram>();
    shaderProgramPtr->BuildTaskMeshProgram(taskShader, meshShader, fragmentShader);

    m_model = MeshletModel();

    shaderProgramPtr->Use();

    m_worldMatrixLocation = shaderProgramPtr->GetUniformLocation("WorldMatrix");
    m_viewProjMatrixLocation = shaderProgramPtr->GetUniformLocation("ViewProjMatrix");
    m_cullingCameraPositionLocation = shaderProgramPtr->GetUniformLocation("CullingCameraPosition");

    shaderProgramPtr->SetUniform(m_worldMatrixLocation, glm::scale(glm::vec3(10.f)));
    shaderProgramPtr->SetUniform(m_viewProjMatrixLocation, m_camera.GetViewProjectionMatrix());
    shaderProgramPtr->SetUniform(m_cullingCameraPositionLocation, m_cameraPosition);
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
        shaderProgramPtr->SetUniform(m_cullingCameraPositionLocation, m_cameraPosition);
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
    shaderProgramPtr->SetUniform(m_viewProjMatrixLocation, m_camera.GetViewProjectionMatrix());
}

void ProjectApplication::InitializeCamera() {
    // Set view matrix, from the camera position looking to the origin
    m_camera.SetViewMatrix(m_cameraPosition, glm::vec3(0.0f));

    // Set perspective matrix
    float aspectRatio = GetMainWindow().GetAspectRatio();
    m_camera.SetPerspectiveProjectionMatrix(1.0f, aspectRatio, 0.1f, 1000.0f);

    m_cullingCamera.SetViewMatrix(m_cullingCameraPosition, glm::vec3(0.0f));
    m_cullingCamera.SetPerspectiveProjectionMatrix(1.0f, aspectRatio, 0.1f, 1000.0f);
}