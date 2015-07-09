#version 440

in vec2 gs_texcoord;

out vec4 out_color;

layout(binding = 0) uniform sampler2D sampler_diffuse;

void main()
{
	out_color = texture(sampler_diffuse, gs_texcoord);
}