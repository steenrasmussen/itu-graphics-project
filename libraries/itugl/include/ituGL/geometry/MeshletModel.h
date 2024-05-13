#ifndef ITU_GRAPHICS_PROGRAMMING_MESHLETMODEL_H
#define ITU_GRAPHICS_PROGRAMMING_MESHLETMODEL_H


#include <vector>
#include "meshoptimizer.h"
#include "glm/vec4.hpp"
#include "glm/vec3.hpp"
#include "ituGL/shader/Material.h"

constexpr size_t maxVertices = 64;
constexpr size_t maxTriangles = 124;
constexpr float coneWeight = .2f;

struct bounds_struct
{
    glm::vec3 center;
    float radius;
    glm::vec3 normal;
    float angle;
};

enum BufferIndex {
    VERTICES = 0,
    MESHLET_VERTICES,
    MESHLET_TRIANGLES,
    MESHLETS,
    MESHLET_BOUNDS
};

class MeshletModel {

public:
    MeshletModel();
    void Draw();
    void AddMaterial(const std::shared_ptr<Material>& material);
    [[nodiscard]] unsigned int GetMeshletCount() const;

private:

    void generateMeshlets();
    void initializeBuffers();

    std::vector<glm::vec4> vertices;

    std::vector<meshopt_Meshlet> meshlets;
    std::vector<unsigned int> meshlet_vertices;
    std::vector<unsigned char> meshlet_triangles;

    unsigned int taskShaderCount{0};
    std::vector<bounds_struct> meshletBounds;
    std::vector<std::shared_ptr<Material>> m_materials;
};


#endif //ITU_GRAPHICS_PROGRAMMING_MESHLETMODEL_H
