#version 440

in vec2 gs_texcoord;

out vec4 out_color;

layout(binding = 0) uniform sampler2D sampler_diffuse;

void main()
{
	out_color = texture(sampler_diffuse, gs_texcoord);
	//out_color = vec4(texture(sampler_diffuse, gs_texcoord).rgb, 1.0f);
	//out_color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
}