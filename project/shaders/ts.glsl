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

uniform mat4 WorldMatrix;
uniform mat4 ViewProjMatrix;

uniform vec3 CullingCameraPosition;
uniform mat4 CullingViewProjMatrix;

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

vec4[6] extractFrustumPlanes(mat4 matrix) {
    mat4 mt = transpose(matrix);
    vec4 planes[6];
    planes[0] = mt[3] + mt[0]; // left
    planes[1] = mt[3] - mt[0]; // right
    planes[2] = mt[3] - mt[1]; // top
    planes[3] = mt[3] + mt[1]; // bottom
    planes[4] = mt[2];         // near
    planes[5] = mt[3] - mt[2]; // far

    for (int i = 0; i < 6; ++i) {
        planes[i] /= length(planes[i].xyz);
    }
    return planes;
}

bool sphereInFrustum(vec4 planes[6], vec3 center, float radius) {
    for(int i = 0; i < 6; ++i) {
        if (dot(center, planes[i].xyz) + planes[i].w < -radius) {
            return false;
        }
    }
    return true;
}

void main() {
    uint groupId = gl_WorkGroupID.x;
    uint threadId = gl_LocalInvocationID.x;
    uint meshletIndex = (groupId * 32) + threadId;

    mesletBounds bounds = bb.bounds[meshletIndex];
    vec4 center = WorldMatrix * vec4(bounds.center, 1.0f);
    float radius = bounds.radius;

    // Frustum culling
    vec4 planes[6] = extractFrustumPlanes(CullingViewProjMatrix);
    bool frustumVisible = sphereInFrustum(planes, center.xyz, radius);

    // Backface culling
    vec3 axis = mat3(WorldMatrix) * bounds.normal;
    float cutOff = bounds.angle;
    bool isFrontFacing = !coneCull(center.xyz, axis, cutOff, CullingCameraPosition);

    bool accept = frustumVisible && isFrontFacing;

    uvec4 ballot = subgroupBallot(accept);
    uint index = subgroupBallotExclusiveBitCount(ballot);

    if (accept) {
        OUT.meshletIndices[index] = meshletIndex;
    }

    if (threadId == 0) {
        gl_TaskCountNV = subgroupBallotBitCount(ballot);
    }
}