#version 440

in vec3 vs_normal;
in vec2 vs_texcoord;

out vec4 out_color;

void main()
{
	out_color = vec4(1.0f, 1.0f, 0.0f, 1.0f);
}