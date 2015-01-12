#version 440

#define DIRECTIONAL_LIGHT_COUNT 1
#define POINT_LIGHT_COUNT 1
#define SPOT_LIGHT_COUNT 1

struct AmbientLight
{
	vec4 intensity;
};

struct DirectionalLight
{
	vec4 directionW;
	vec4 intensity;
};

struct PointLight
{
    vec4 positionW;
    vec4 intensity;
    float cutoff;
};

struct SpotLight
{
	vec4 positionW;
	vec4 directionW;
	vec4 intensity;
	float cutoff;
	float angle;
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
	AmbientLight ambientLight;
	DirectionalLight directionalLights[DIRECTIONAL_LIGHT_COUNT];
    PointLight pointLights[POINT_LIGHT_COUNT];
	SpotLight spotLights[SPOT_LIGHT_COUNT];
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
    ambient = ambientLight.intensity.rgb * surfaceColor;

	// Directional lights
	for (int i = 0; i < DIRECTIONAL_LIGHT_COUNT; ++i)
	{
		// Diffuse lighting
		float diffuseCoefficient = max(0.0f, dot(vs_normalW, -directionalLights[i].directionW.xyz));
        diffuse += diffuseCoefficient * surfaceColor * directionalLights[i].intensity.rgb;

		// Specular lighting
		vec3 halfway = normalize(surfaceToCamera - directionalLights[i].directionW.xyz);
        float specularCoefficient = pow(max(0.0f, dot(vs_normalW, halfway)), materialSpecularColor.a);
        specular += specularCoefficient * materialSpecularColor.rgb * directionalLights[i].intensity.rgb;
	}

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
        vec3 halfway = normalize(surfaceToCamera + surfaceToLight);
        float specularCoefficient = pow(max(0.0f, dot(vs_normalW, halfway)), materialSpecularColor.a);
        specular += attenuation * specularCoefficient * materialSpecularColor.rgb * pointLights[i].intensity.rgb;
    }

	// Spot lights
	for (int i = 0; i < SPOT_LIGHT_COUNT; ++i)
    {
        vec3 surfaceToLight = spotLights[i].positionW.xyz - vs_positionW;
		float surfaceToLightDistance = length(surfaceToLight);
		float angle = acos(dot(-surfaceToLight, spotLights[i].directionW.xyz) / surfaceToLightDistance);
		
		if (angle <= spotLights[i].angle)
		{
			float attenuation = (spotLights[i].cutoff - surfaceToLightDistance) / spotLights[i].cutoff;
			
			// Diffuse lighting
			float diffuseCoefficient = max(0.0f, dot(vs_normalW, surfaceToLight)) / surfaceToLightDistance;
			diffuse += attenuation * diffuseCoefficient * surfaceColor * spotLights[i].intensity.rgb;
		
			// Specular lighting
			vec3 halfway = normalize(surfaceToCamera + surfaceToLight);
			float specularCoefficient = pow(max(0.0f, dot(vs_normalW, halfway)), materialSpecularColor.a);
			specular += attenuation * specularCoefficient * materialSpecularColor.rgb * spotLights[i].intensity.rgb;
		}
    }

    out_color = vec4(ambient + diffuse + specular, 1.0f);
}