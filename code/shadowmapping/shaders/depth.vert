#version 440

layout(location = 0) in vec3 in_position_M;

layout(binding = 2, std140) uniform PerInstance
{
	mat4 model_matrix;
	mat4 light_projection_view_matrix;	// Using the normal_matrix location.
    vec4 material_specular_color;
};

void main(void)
{
	gl_Position = light_projection_view_matrix * model_matrix * vec4(in_position_M, 1.0f);
}