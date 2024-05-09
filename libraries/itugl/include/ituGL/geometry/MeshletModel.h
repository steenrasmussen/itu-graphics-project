//
// Created by steen on 09-05-2024.
//

#ifndef ITU_GRAPHICS_PROGRAMMING_MESHLETMODEL_H
#define ITU_GRAPHICS_PROGRAMMING_MESHLETMODEL_H


#include <vector>
#include "meshoptimizer.h"
#include "glm/vec4.hpp"

constexpr size_t maxVertices = 64;
constexpr size_t maxTriangles = 124;
constexpr float coneWeight = 0.0f;

//constexpr unsigned int maxTaskShaderWorkgroup

enum BufferIndex {
    VERTICES = 0,
    MESHLET_VERTICES,
    MESHLET_TRIANGLES,
    MESHLETS
//    MVP
};
class MeshletModel {

public:
    MeshletModel();
    void Draw();

private:

    void generateMeshlets();
    void initializeBuffers();

    std::vector<glm::vec4> vertices;

    std::vector<meshopt_Meshlet> meshlets;
    std::vector<unsigned int> meshlet_vertices;
    std::vector<unsigned char> meshlet_triangles;

    unsigned int taskShaderCount{0};
};


#endif //ITU_GRAPHICS_PROGRAMMING_MESHLETMODEL_H
