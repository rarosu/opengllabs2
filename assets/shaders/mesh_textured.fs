#version 440

#define POINT_LIGHT_COUNT 1

struct PointLight
{
    vec4 positionW;
    vec4 intensity;
    float cutoff;
};

in vec3 vs_positionW;
in vec3 vs_normalW;
in vec2 vs_texcoord;

out vec4 out_color;

layout(binding = 0, std140) uniform PerFrame
{
	mat4 viewMatrix;
	mat4 projectionMatrix;
    vec4 cameraPositionW;
};

layout(binding = 1, std140) uniform PerInstance
{
	mat4 modelMatrix;
	mat4 normalMatrix;

    // The last component is the shininess of the material.
    vec4 materialSpecularColor;
};

layout(binding = 2, std140) uniform Constant
{
	vec4 ambientLightIntensity;
    PointLight pointLights[POINT_LIGHT_COUNT];
};

layout(binding = 0) uniform sampler2D samplerDiffuse;

void main()
{
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    vec3 surfaceColor = texture(samplerDiffuse, vs_texcoord).rgb;
    vec3 surfaceToCamera = cameraPositionW.xyz - vs_positionW;
    float surfaceToCameraDistance = length(surfaceToCamera);
    
    // Ambient lighting
    ambient = ambientLightIntensity.rgb * surfaceColor;

    // Point lights
    for (int i = 0; i < POINT_LIGHT_COUNT; ++i)
    {
        vec3 surfaceToLight = pointLights[i].positionW.xyz - vs_positionW;
        float surfaceToLightDistance = length(surfaceToLight);
        float attenuation = (pointLights[i].cutoff - surfaceToLightDistance) / pointLights[i].cutoff;

        // Diffuse lighting
        float diffuseCoefficient = max(0.0f, dot(vs_normalW, surfaceToLight)) / surfaceToLightDistance;
        diffuse += attenuation * diffuseCoefficient * surfaceColor * pointLights[i].intensity.rgb;

        // Specular lighting
        vec3 halfway = normalize(surfaceToLight + surfaceToCamera);
        float specularCoefficient = pow(max(0.0f, dot(vs_normalW, halfway)), materialSpecularColor.a);
        specular += attenuation * specularCoefficient * materialSpecularColor.rgb * pointLights[i].intensity.rgb;
    }

    out_color = vec4(ambient + diffuse + specular, 1.0f);
}