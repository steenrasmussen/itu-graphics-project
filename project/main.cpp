#include <iostream>
#include "ProjectApplication.h"
#include "glm/vec3.hpp"
#include "glm/vec4.hpp"
#include "glm/mat4x4.hpp"

using namespace glm;

int main() {

//    mat4 vp = mat4(
//        1.83048761f, 0.0f, 0.0f, 0.0f,
//        0.0f, 1.29435027f, -0.70724821f, -0.707106829f,
//        0.0f, -1.29435027f, -0.70724821f, -0.707106829f,
//        0.0f, 0.0f, 28.0899086f, 28.2842731f
//    );
//
//    mat4 viewSpace = mat4(
//            0.99999994f, 0.0f, -0.0f, 0.0f,
//            -0.0f, 0.707106769f, 0.707106829f, 0.0f,
//            0.0f, -0.707106769f, 0.707106829f, 0.0f,
//            -0.0f, -0.0f, -28.2842731f, 1.0f
//    );

//    mat4 cullingVP = mat4(
//            -1.83048761f, 0.0f, 0.0f, 0.0f,
//            0.0f, 1.29435027f, -0.70724821f, -0.707106829f,
//            0.0f, 1.29435027f, 0.70724821f, 0.707106829f,
//            0.0f, 0.0f, 28.0899086f, 28.2842731f
//    );

//    mat4 cullingViewspace = mat4(
//            -0.99999994f, 0.0f, -0.0f, 0.0f,
//            0.0f, 0.707106769f, 0.707106829f, 0.0f,
//            0.0f, 0.707106769f, -0.707106829f, 0.0f,
//            -0.0f, -0.0f, -28.2842731f, 1.0f
//    );
//
//    vec3 cameraPosition = vec3(0.0f, 20.0f, 20.0f);
//    vec3 coneApex = viewSpace * vec4(0.087621279f, 0.0206827428f, 0.0450193286f, 1.0f);
//    vec3 coneAxis = viewSpace * vec4(-0.670318782f, -0.742067396f, 0.00292623346f, 1.0f);
//
//    vec3 cullingCameraPosition = vec3(0.0f, 20.0f, -20.0f);
//    vec3 cullingConeApex = cullingViewspace * vec4(0.087621279f, 0.0206827428f, 0.0450193286f, 1.0f);
//    vec3 cullingConeAxis = cullingViewspace * vec4(-0.670318782f, -0.742067396f, 0.00292623346f, 1.0f);
//
//    float coneCutoff = 0.979893445f;
//
//    bool keep = dot(normalize(coneApex - cameraPosition), coneAxis) >= coneCutoff;
//    std::cout << "Keep: " << keep;
//    keep = dot(normalize(cullingConeApex - cullingCameraPosition), cullingConeAxis) >= coneCutoff;
//    std::cout << "\nKeep: " << keep;

    ProjectApplication projectApplication;
    return projectApplication.Run();

//    return 0;
}
