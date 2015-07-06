#version 440

layout(location = 0) in vec3 in_positionM;
layout(location = 1) in vec3 in_normalM;
layout(location = 2) in vec2 in_texcoord;

out vec3 vs_positionW;
out vec3 vs_normalW;
out vec2 vs_texcoord;

layout(binding = 1, std140) uniform PerFrame
{
	mat4 viewMatrix;
	mat4 projectionMatrix;
	vec4 cameraPositionW;
};

layout(binding = 2, std140) uniform PerInstance
{
	mat4 modelMatrix;
	mat4 normalMatrix;

	// The last component is the shininess of the material.
	vec4 materialSpecularColor;
};

void main()
{
	gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(in_positionM, 1.0f);
	vs_positionW = (modelMatrix * vec4(in_positionM, 1.0f)).xyz;
	vs_normalW = normalize(mat3(normalMatrix) * in_normalM);
	vs_texcoord = in_texcoord;
}