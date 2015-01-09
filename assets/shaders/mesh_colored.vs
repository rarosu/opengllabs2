#version 440

#define POINT_LIGHT_COUNT 1

layout(location = 0) in vec3 in_positionM;
layout(location = 1) in vec3 in_normalM;
layout(location = 2) in vec2 in_texcoord;

out vec3 vs_positionV;
out vec3 vs_normalV;
out vec2 vs_texcoord;

layout(binding = 0, std140) uniform PerFrame
{
	mat4 viewMatrix;
	mat4 projectionMatrix;

	vec4 pointLightPositionV[POINT_LIGHT_COUNT];
};

layout(binding = 1, std140) uniform PerInstance
{
	mat4 modelMatrix;
	mat3 normalMatrix;
};

layout(binding = 2, std140) uniform Constant
{
	vec4 ambientLightIntensity;
	vec4 pointLightIntensity[POINT_LIGHT_COUNT];
};

void main()
{
	gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(in_positionM, 1.0f);
	vs_positionV = (viewMatrix * modelMatrix * vec4(in_positionM, 1.0f)).xyz;
	vs_normalV = normalize(viewMatrix * vec4(normalMatrix * in_normalM, 0.0f)).xyz;
	vs_texcoord = in_texcoord;
}