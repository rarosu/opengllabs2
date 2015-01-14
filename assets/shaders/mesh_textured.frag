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
in vec4 vs_positionL;

out vec4 out_color;

layout(binding = 0, std140) uniform Constant
{
	AmbientLight ambientLight;
	DirectionalLight directionalLights[DIRECTIONAL_LIGHT_COUNT];
    PointLight pointLights[POINT_LIGHT_COUNT];
	SpotLight spotLights[SPOT_LIGHT_COUNT];
};

layout(binding = 1, std140) uniform PerFrame
{
	mat4 viewMatrix;
	mat4 projectionMatrix;
    vec4 cameraPositionW;
};

layout(binding = 2, std140) uniform PerInstance
{
	mat4 modelMatrix;
	mat4 normalMatrix;

    // The last component is the shininess of the material.
    vec4 materialSpecularColor;
};

layout(binding = 0) uniform sampler2D samplerDiffuse;
layout(binding = 1) uniform sampler2D samplerShadowmap;

void AddDirectionalLightContribution(DirectionalLight light, vec3 surfaceColor, vec3 surfaceToCamera, inout vec3 diffuse, inout vec3 specular);
void AddPointLightContribution(PointLight light, vec3 surfaceColor, vec3 surfaceToCamera, inout vec3 diffuse, inout vec3 specular);
void AddSpotLightContribution(SpotLight light, vec3 surfaceColor, vec3 surfaceToCamera, inout vec3 diffuse, inout vec3 specular);

void main()
{
    vec3 ambient = vec3(0);
    vec3 diffuse = vec3(0);
    vec3 specular = vec3(0);
    vec3 surfaceColor = texture(samplerDiffuse, vs_texcoord).rgb;
    vec3 surfaceToCamera = cameraPositionW.xyz - vs_positionW;
    float surfaceToCameraDistance = length(surfaceToCamera);
    
    // Ambient lighting
    ambient = ambientLight.intensity.rgb * surfaceColor;

	// Directional lights
	for (int i = 0; i < DIRECTIONAL_LIGHT_COUNT; ++i)
	{
		AddDirectionalLightContribution(directionalLights[i], surfaceColor, surfaceToCamera, diffuse, specular);
	}

    // Point lights
    for (int i = 0; i < POINT_LIGHT_COUNT; ++i)
    {
        AddPointLightContribution(pointLights[i], surfaceColor, surfaceToCamera, diffuse, specular);
    }

	// Spot lights (index 0 is shadowmapped)
	vec2 shadowmapTexcoords = vec2(vs_positionL.s, vs_positionL.t);
	if (shadowmapTexcoords.s < 0.0f || shadowmapTexcoords.s > 1.0f || shadowmapTexcoords.t < 0.0f || shadowmapTexcoords.t > 1.0f)
	{
		// Do light calculations as we are outside the shadowmap.
		AddSpotLightContribution(spotLights[0], surfaceColor, surfaceToCamera, diffuse, specular);
	}
	else
	{
		if (vs_positionL.z - 0.001f < texture(samplerShadowmap, shadowmapTexcoords).r)
		{
			// Do light calculations as we are in front of occluding geometry.
			AddSpotLightContribution(spotLights[0], surfaceColor, surfaceToCamera, diffuse, specular);
		}
	}

	for (int i = 1; i < SPOT_LIGHT_COUNT; ++i)
    {
        AddSpotLightContribution(spotLights[i], surfaceColor, surfaceToCamera, diffuse, specular);
    }

    out_color = vec4(ambient + diffuse + specular, 1.0f);
	//out_color = texture(samplerDiffuse, vs_texcoord);
	//out_color = vec4(vec3(vs_positionL.z), 1.0f);
	//out_color = vec4(vec3(texture(samplerShadowmap, shadowmapTexcoords).r), 1.0f);
}




void AddDirectionalLightContribution(DirectionalLight light, vec3 surfaceColor, vec3 surfaceToCamera, inout vec3 diffuse, inout vec3 specular)
{
	// Diffuse lighting
	float diffuseCoefficient = max(0.0f, dot(vs_normalW, -light.directionW.xyz));
    diffuse += diffuseCoefficient * surfaceColor * light.intensity.rgb;

	// Specular lighting
	if (diffuseCoefficient > 0.0f)
	{
		vec3 halfway = normalize(surfaceToCamera - light.directionW.xyz);
		float specularCoefficient = pow(max(0.0f, dot(vs_normalW, halfway)), materialSpecularColor.a);
		specular += specularCoefficient * materialSpecularColor.rgb * light.intensity.rgb;
	}
}

void AddPointLightContribution(PointLight light, vec3 surfaceColor, vec3 surfaceToCamera, inout vec3 diffuse, inout vec3 specular)
{
	vec3 surfaceToLight = light.positionW.xyz - vs_positionW;
    float surfaceToLightDistance = length(surfaceToLight);
    float attenuation = max(0.0f, light.cutoff - surfaceToLightDistance) / light.cutoff;

    // Diffuse lighting
    float diffuseCoefficient = max(0.0f, dot(vs_normalW, surfaceToLight)) / surfaceToLightDistance;
    diffuse += attenuation * diffuseCoefficient * surfaceColor * light.intensity.rgb;

    // Specular lighting
	if (diffuseCoefficient > 0.0f)
	{
		vec3 halfway = normalize(surfaceToCamera + surfaceToLight);
		float specularCoefficient = pow(max(0.0f, dot(vs_normalW, halfway)), materialSpecularColor.a);
		specular += attenuation * specularCoefficient * materialSpecularColor.rgb * light.intensity.rgb;
	}
}

void AddSpotLightContribution(SpotLight light, vec3 surfaceColor, vec3 surfaceToCamera, inout vec3 diffuse, inout vec3 specular)
{
	vec3 surfaceToLight = light.positionW.xyz - vs_positionW;
	float surfaceToLightDistance = length(surfaceToLight);
	float angle = acos(dot(-surfaceToLight, light.directionW.xyz) / surfaceToLightDistance);
		
	if (angle <= light.angle)
	{
		float attenuation = max(0.0f, light.cutoff - surfaceToLightDistance) / light.cutoff;
			
		// Diffuse lighting
		float diffuseCoefficient = max(0.0f, dot(vs_normalW, surfaceToLight)) / surfaceToLightDistance;
		diffuse += attenuation * diffuseCoefficient * surfaceColor * light.intensity.rgb;
		
		// Specular lighting
		if (diffuseCoefficient > 0.0f)
		{
			vec3 halfway = normalize(surfaceToCamera + surfaceToLight);
			float specularCoefficient = pow(max(0.0f, dot(vs_normalW, halfway)), materialSpecularColor.a);
			specular += attenuation * specularCoefficient * materialSpecularColor.rgb * light.intensity.rgb;
		}
	}
}