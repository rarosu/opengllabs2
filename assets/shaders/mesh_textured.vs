#version 440

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec2 in_texcoord;

out vec3 vs_normal;
out vec2 vs_texcoord;

layout(std140) uniform PerFrame
{
	mat4 viewMatrix;
	mat4 projectionMatrix;
};

layout(std140) uniform PerInstance
{
	mat4 modelMatrix;
	mat4 normalMatrix;
};

void main()
{
	gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(in_position, 1.0f);
	vs_normal = normalize(viewMatrix * normalMatrix * vec4(in_normal, 0.0f)).xyz;
	vs_texcoord = in_texcoord;
}