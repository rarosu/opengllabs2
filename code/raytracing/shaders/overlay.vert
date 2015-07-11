#version 440

layout(location = 0) in vec2 in_position_C;
layout(location = 1) in vec2 in_texcoord;

out vec2 vs_texcoord;

void main()
{
	gl_Position = vec4(in_position_C, 0.0f, 1.0f);
	vs_texcoord = in_texcoord;
}