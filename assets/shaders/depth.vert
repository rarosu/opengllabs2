#version 440

layout(location = 0) in vec3 in_positionM;

layout(binding = 2, std140) uniform PerInstance
{
	mat4 modelMatrix;
	mat4 normalMatrix;

	// The last component is the shininess of the material.
	vec4 materialSpecularColor;
};

layout(binding = 3, std140) uniform PerShadowcaster
{
	mat4 lightProjectionViewMatrix;
	mat4 lightBiasMatrix;
};

void main(void)
{
	gl_Position = lightProjectionViewMatrix * modelMatrix * vec4(in_positionM, 1.0f);
}