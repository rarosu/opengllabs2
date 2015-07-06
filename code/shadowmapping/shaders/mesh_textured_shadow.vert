#version 440

#define SPOT_LIGHT_COUNT_MAX 5

struct AmbientLight
{
	vec4 intensity;
};

struct SpotLight
{
    mat4 light_projection_view_matrix;
	vec4 position_W;
	vec4 direction_W;
	vec4 intensity;
	float cutoff;
	float angle;
};

layout(location = 0) in vec3 in_position_M;
layout(location = 1) in vec3 in_normal_M;
layout(location = 2) in vec2 in_texcoord;

out vec3 vs_position_W;
out vec3 vs_normal_W;
out vec2 vs_texcoord;
out vec4 vs_position_L[SPOT_LIGHT_COUNT_MAX];

layout(binding = 0, std140) uniform Constant
{
	AmbientLight ambient_light;
	SpotLight spot_lights[SPOT_LIGHT_COUNT_MAX];
	mat4 bias_matrix;
	int spot_light_count;
};

layout(binding = 1, std140) uniform PerFrame
{
	mat4 view_matrix;
	mat4 projection_matrix;
    vec4 camera_position_W;
};

layout(binding = 2, std140) uniform PerInstance
{
	mat4 model_matrix;
	mat4 normal_matrix;
    vec4 material_specular_color;
};

void main()
{
	gl_Position = projection_matrix * view_matrix * model_matrix * vec4(in_position_M, 1.0f);
	vs_position_W = (model_matrix * vec4(in_position_M, 1.0f)).xyz;
	vs_normal_W = normalize(mat3(normal_matrix) * in_normal_M);
	vs_texcoord = in_texcoord;

	for (int i = 0; i < spot_light_count; ++i)
	{
		vs_position_L[i] = bias_matrix * spot_lights[i].light_projection_view_matrix * model_matrix * vec4(in_position_M, 1.0f);
	}
}