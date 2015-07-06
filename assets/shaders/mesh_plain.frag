#version 440

out vec4 out_color;

layout(binding = 2, std140) uniform PerInstance
{
	mat4 modelMatrix;
	vec4 color;
};

void main()
{
    out_color = color;
}