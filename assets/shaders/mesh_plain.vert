#version 440

layout(location = 0) in vec3 in_positionM;

layout(binding = 1, std140) uniform PerFrame
{
	mat4 viewMatrix;
	mat4 projectionMatrix;
	vec4 cameraPositionW;
};

layout(binding = 2, std140) uniform PerInstance
{
	mat4 modelMatrix;
	vec4 color;
};

void main()
{
	gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(in_positionM, 1.0f);
}