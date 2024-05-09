#version 450

#extension GL_NV_mesh_shader : require
#extension GL_KHR_shader_subgroup_ballot: require

layout(local_size_x = 32) in;

struct Meshlet
{
    uint vertexOffset;
    uint triangleOffset;
    uint vertexCount;
    uint triangleCount;
};

taskNV out Task
{
    uint meshletIndices[32];
} OUT;

void main() {
    uint groupId = gl_WorkGroupID.x;
    uint threadId = gl_LocalInvocationID.x;

    uint meshletIndex = (groupId * 32) + threadId;
    OUT.meshletIndices[threadId] = meshletIndex;

    if (threadId == 0) {
        // TODO: We spawn to make workgroups for that last iteration
        gl_TaskCountNV = 32;
    }
}