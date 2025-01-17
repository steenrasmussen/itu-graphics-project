#version 450

out vec4 FragColor;

in PerVertexData
{
    vec3 WorldPosition;
    vec3 WorldNormal;
    vec3 Color;
} fragIn;

uniform float AmbientReflection;
uniform float DiffuseReflection;
uniform float SpecularReflection;
uniform float SpecularExponent;

uniform vec3 AmbientColor;
uniform vec3 LightColor;
uniform vec3 LightPosition;
uniform vec3 CameraPosition;

vec3 GetAmbientReflection(vec3 objectColor)
{
    return AmbientColor * AmbientReflection * objectColor;
}

vec3 GetDiffuseReflection(vec3 objectColor, vec3 lightVector, vec3 normalVector)
{
    return LightColor * DiffuseReflection * objectColor * max(dot(lightVector, normalVector), 0.0f);
}

vec3 GetSpecularReflection(vec3 lightVector, vec3 viewVector, vec3 normalVector)
{
    vec3 halfVector = normalize(lightVector + viewVector);
    return LightColor * SpecularReflection * pow(max(dot(halfVector, normalVector), 0.0f), SpecularExponent);
}

vec3 GetBlinnPhongReflection(vec3 objectColor, vec3 lightVector, vec3 viewVector, vec3 normalVector)
{
    return GetAmbientReflection(objectColor)
    + GetDiffuseReflection(objectColor, lightVector, normalVector)
    + GetSpecularReflection(lightVector, viewVector, normalVector);
}

void main() {
    vec4 objectColor = vec4(fragIn.Color, 1.0f);
    vec3 lightVector = normalize(LightPosition - fragIn.WorldPosition);
    vec3 viewVector = normalize(CameraPosition - fragIn.WorldPosition);
    vec3 normalVector = normalize(fragIn.WorldNormal);
    FragColor = vec4(GetBlinnPhongReflection(objectColor.rgb, lightVector, viewVector, normalVector), 1.0f);
}