#version 440

layout(location = 0) in vec3 in_position_M;

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
	gl_Position = view_matrix * model_matrix * vec4(in_position_M, 1.0f);
}