#version 440

#define POINT_LIGHT_COUNT 1

in vec3 vs_positionW;
in vec3 vs_normalW;
in vec2 vs_texcoord;

out vec4 out_color;

layout(binding = 0, std140) uniform PerFrame
{
	mat4 viewMatrix;
	mat4 projectionMatrix;
};

layout(binding = 2, std140) uniform Constant
{
	vec4 ambientLightIntensity;
    vec4 pointLightPositionW[POINT_LIGHT_COUNT];
	vec4 pointLightIntensity[POINT_LIGHT_COUNT];
};

layout(binding = 0) uniform sampler2D samplerDiffuse;

void main()
{
    vec4 lightIntensity = ambientLightIntensity;
    for (int i = 0; i < POINT_LIGHT_COUNT; ++i)
    {
        vec3 L = normalize(pointLightPositionW[i].xyz - vs_positionW);
        float incidence = clamp(dot(vs_normalW, L), 0.0f, 1.0f);

        lightIntensity += pointLightIntensity[i] * incidence;
    }

    out_color = texture(samplerDiffuse, vs_texcoord) * lightIntensity;
}