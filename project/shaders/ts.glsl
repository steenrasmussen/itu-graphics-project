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

uniform vec3 CullingCameraPosition;
uniform mat4 WorldMatrix;

struct mesletBounds
{
    vec3 center;
    float radius;
    vec3 normal;
    float angle;
};

layout(std430, binding = 4) buffer MesletBoundsBuffer {
    mesletBounds bounds[];
} bb;

bool coneCull(vec3 center, vec3 cone_axis, float cone_cutoff, vec3 camera_position)
{
    return dot(normalize(center - camera_position), cone_axis) >= cone_cutoff;
}

void main() {
    uint groupId = gl_WorkGroupID.x;
    uint threadId = gl_LocalInvocationID.x;
    uint meshletIndex = (groupId * 32) + threadId;

    mesletBounds bounds = bb.bounds[meshletIndex];
    vec4 center = WorldMatrix * vec4(bounds.center, 1.0f);
    vec3 axis = mat3(WorldMatrix) * bounds.normal;
    float cutOff = bounds.angle;
    bool accept = !coneCull(center.xyz, axis, cutOff, CullingCameraPosition);

    uvec4 ballot = subgroupBallot(accept);
    uint index = subgroupBallotExclusiveBitCount(ballot);

    if (accept) {
        OUT.meshletIndices[index] = meshletIndex;
    }

    if (threadId == 0) {
        gl_TaskCountNV = subgroupBallotBitCount(ballot);
    }
}