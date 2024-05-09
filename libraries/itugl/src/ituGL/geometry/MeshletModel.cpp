//
// Created by steen on 09-05-2024.
//

#include "ituGL/geometry/MeshletModel.h"
#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "assimp/mesh.h"
#include "glm/vec4.hpp"
#include "assimp/scene.h"
#include "glad/glad.h"

MeshletModel::MeshletModel() {
    generateMeshlets();
    initializeBuffers();
    taskShaderCount = ceil(meshlets.size() / static_cast<float>(32));
}

void MeshletModel::Draw() {
    glDrawMeshTasksNV(0, taskShaderCount);
}

void MeshletModel::generateMeshlets() {
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile("models/mill/Mill.obj", aiProcess_CalcTangentSpace | aiProcess_GenNormals | aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_SortByPType);

    aiMesh *mesh = scene->mMeshes[2];

    std::vector<unsigned int> indices;
    for (unsigned int faceIndex = 0; faceIndex < mesh->mNumFaces; faceIndex++) {
        aiFace face = mesh->mFaces[faceIndex];
        for (unsigned int i = 0; i < face.mNumIndices; i++)
            indices.push_back(face.mIndices[i]);
    }

    for (unsigned int vertexNo = 0; vertexNo < mesh->mNumVertices; vertexNo++) {
        glm::vec4 vector;
        vector.x = mesh->mVertices[vertexNo].x;
        vector.y = mesh->mVertices[vertexNo].y;
        vector.z = mesh->mVertices[vertexNo].z;
        vector.w = 1.0f;
        vertices.push_back(vector);
    }

    const size_t maxMeshlets = meshopt_buildMeshletsBound(indices.size(), maxVertices, maxTriangles);

    meshlets.resize(maxMeshlets);
    meshlet_vertices.resize(maxMeshlets * maxVertices);
    meshlet_triangles.resize(maxMeshlets * maxTriangles * 3);

    size_t meshletCount = meshopt_buildMeshlets(meshlets.data(), meshlet_vertices.data(), meshlet_triangles.data(), indices.data(),
                                                 indices.size(), &vertices[0].x, vertices.size(), sizeof(glm::vec4), maxVertices, maxTriangles, coneWeight);

    const meshopt_Meshlet& last = meshlets[meshletCount - 1];
    meshlet_vertices.resize(last.vertex_offset + last.vertex_count);
    meshlet_triangles.resize(last.triangle_offset + ((last.triangle_count * 3 + 3) & ~3));
    meshlets.resize(meshletCount);

    // Something seems off with the bounds or ?
    std::vector<meshopt_Bounds> bounds;
    for (const meshopt_Meshlet & m : meshlets) {
        meshopt_optimizeMeshlet(&meshlet_vertices[m.vertex_offset], &meshlet_triangles[m.triangle_offset], m.triangle_count, m.vertex_count);

        meshopt_Bounds meshletBounds = meshopt_computeMeshletBounds(&meshlet_vertices[m.vertex_offset], &meshlet_triangles[m.triangle_offset],
                                                                    m.triangle_count, &vertices[0].x, vertices.size(), sizeof(glm::vec4));
        bounds.push_back(meshletBounds);
    }
}

void MeshletModel::initializeBuffers() {
    GLuint buffer[4];
    glGenBuffers(4, buffer);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffer[BufferIndex::VERTICES]);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(glm::vec4) * vertices.size(), &vertices[0],  GL_STATIC_READ);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, BufferIndex::VERTICES, buffer[BufferIndex::VERTICES]);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffer[BufferIndex::MESHLET_VERTICES]);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(unsigned int) * meshlet_vertices.size(), &meshlet_vertices[0],  GL_STATIC_READ);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, BufferIndex::MESHLET_VERTICES, buffer[BufferIndex::MESHLET_VERTICES]);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffer[BufferIndex::MESHLET_TRIANGLES]);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(unsigned char) * meshlet_triangles.size(), &meshlet_triangles[0],  GL_STATIC_READ);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, BufferIndex::MESHLET_TRIANGLES, buffer[BufferIndex::MESHLET_TRIANGLES]);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffer[BufferIndex::MESHLETS]);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(meshopt_Meshlet) * meshlets.size(), &meshlets[0],  GL_STATIC_READ);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, BufferIndex::MESHLETS, buffer[BufferIndex::MESHLETS]);
}
