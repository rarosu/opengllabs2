#version 440

layout(points) in;
layout(triangle_strip) out;
layout(max_vertices = 4) out;

out vec2 gs_texcoord;

layout(binding = 1, std140) uniform PerFrame
{
	mat4 view_matrix;
	mat4 projection_matrix;
    vec4 camera_position_W;
};

const float PARTICLE_HALFSIZE = 0.25f;

void main(void) 
{
	vec4 corner_V;

	corner_V = gl_in[0].gl_Position + PARTICLE_HALFSIZE * vec4(+1.0f, +1.0f, 0.0f, 0.0f);
	gl_Position = projection_matrix * corner_V;
	gs_texcoord = vec2(0.0f, 1.0f);
	EmitVertex();

	corner_V = gl_in[0].gl_Position + PARTICLE_HALFSIZE * vec4(-1.0f, +1.0f, 0.0f, 0.0f);
	gl_Position = projection_matrix * corner_V;
	gs_texcoord = vec2(1.0f, 1.0f);
	EmitVertex();

	corner_V = gl_in[0].gl_Position + PARTICLE_HALFSIZE * vec4(+1.0f, -1.0f, 0.0f, 0.0f);
	gl_Position = projection_matrix * corner_V;
	gs_texcoord = vec2(0.0f, 0.0f);
	EmitVertex();

	corner_V = gl_in[0].gl_Position + PARTICLE_HALFSIZE * vec4(-1.0f, -1.0f, 0.0f, 0.0f);
	gl_Position = projection_matrix * corner_V;
	gs_texcoord = vec2(1.0f, 0.0f);
	EmitVertex();

	EndPrimitive();
}