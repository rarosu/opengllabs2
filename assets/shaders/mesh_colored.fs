#version 440

#define POINT_LIGHT_COUNT 1

in vec3 vs_positionV;
in vec3 vs_normalV;
in vec2 vs_texcoord;

out vec4 out_color;

layout(binding = 0, std140) uniform PerFrame
{
	mat4 viewMatrix;
	mat4 projectionMatrix;

	vec4 pointLightPositionV[POINT_LIGHT_COUNT];
};

layout(binding = 2, std140) uniform Constant
{
	vec4 ambientLightIntensity;
	vec4 pointLightIntensity[POINT_LIGHT_COUNT];
};

void main()
{
    //vec4 color = vec4(1.0f, 1.0f, 0.0f, 1.0f);
    vec4 color = ambientLightIntensity;

    for (int i = 0; i < POINT_LIGHT_COUNT; ++i)
    {
        vec3 L = normalize(pointLightPositionV[i].xyz - vs_positionV);
        float incidence = clamp(dot(vs_normalV, L), 0.0f, 1.0f);

        color += pointLightIntensity[i] * incidence;
    }
    

    out_color = color;
}