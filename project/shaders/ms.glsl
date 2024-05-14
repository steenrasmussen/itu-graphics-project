#version 450

#extension GL_NV_mesh_shader : require

layout(local_size_x = 32) in;
layout(triangles, max_vertices = 64, max_primitives = 124) out;

taskNV in Task {
    uint meshletIndices[32];
} IN;

struct Vertex {
    vec4 position;
    vec4 normal;
};

layout(std430, binding = 0) buffer VertexBuffer {
    Vertex vertices[];
} vb;

layout(std430, binding = 1) buffer VertexIndexBuffer {
    uint vertices[];
} vi;

layout(std430, binding = 2) buffer TriangleIndexBuffer {
    uint indices[];
} ti;

struct Meshlet
{
    uint vertexOffset;
    uint triangleOffset;
    uint vertexCount;
    uint triangleCount;
};

layout (std430, binding = 3) buffer MeshletBuffer
{
    Meshlet meshlets[];
} mb;

uniform mat4 WorldMatrix;
uniform mat4 ViewProjMatrix;

out PerVertexData
{
    vec3 WorldPosition;
    vec3 WorldNormal;
    vec3 Color;
} v_out[];

#define MAX_COLORS 10
vec3 meshletcolors[MAX_COLORS] = {
    vec3(1,0,0),
    vec3(0,1,0),
    vec3(0,0,1),
    vec3(1,1,0),
    vec3(1,0,1),
    vec3(0,1,1),
    vec3(0.5,0,0),
    vec3(0,0.5,0),
    vec3(0,0.0,0.5),
    vec3(0.5,0.5,0.5)
};

void main() {

    uint groupId = gl_WorkGroupID.x;
    uint threadId = gl_LocalInvocationID.x;

    uint meshletIndex = IN.meshletIndices[groupId];
    Meshlet meshlet = mb.meshlets[meshletIndex];

    for (uint i = threadId; i < meshlet.vertexCount; i += 32) {
        uint position = vi.vertices[meshlet.vertexOffset + i];
        gl_MeshVerticesNV[i].gl_Position = ViewProjMatrix * WorldMatrix * vb.vertices[position].position;
        v_out[i].WorldPosition = (WorldMatrix * vb.vertices[position].position).xyz;
        v_out[i].WorldNormal = (WorldMatrix * vb.vertices[position].normal).xyz;
        v_out[i].Color = meshletcolors[meshletIndex % MAX_COLORS];
    }

    uint indexCount = (meshlet.triangleCount * 3 + 3) / 4;
    uint indexOffset = meshlet.triangleOffset / 4;
    for (uint i = threadId; i < indexCount; i += 32) {
        writePackedPrimitiveIndices4x8NV(4 * i, ti.indices[indexOffset + i]);
    }

    if (threadId == 0) {
        gl_PrimitiveCountNV = meshlet.triangleCount;
    }
}