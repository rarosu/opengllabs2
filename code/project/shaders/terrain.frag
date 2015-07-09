#version 440

#define POINT_LIGHT_COUNT 2
#define DIRECTIONAL_LIGHT_COUNT 2
#define SPOT_LIGHT_COUNT 2

struct AmbientLight
{
	vec4 intensity;
};

struct DirectionalLight
{
	vec4 direction_W;
	vec4 intensity;
};

struct PointLight
{
	vec4 position_W;
	vec4 intensity;
	float cutoff;
};

struct SpotLight
{
	vec4 position_W;
	vec4 direction_W;
	vec4 intensity;
	float cutoff;
	float angle;
};

in vec3 vs_position_W;
in vec3 vs_normal_W;
in vec2 vs_texcoord;

out vec4 out_color;

layout(binding = 0, std140) uniform Constant
{
	AmbientLight ambient_light;
	DirectionalLight directional_lights[DIRECTIONAL_LIGHT_COUNT];
    PointLight point_lights[POINT_LIGHT_COUNT];
	SpotLight spot_lights[SPOT_LIGHT_COUNT];
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

layout(binding = 0) uniform sampler2D sampler_mask;
layout(binding = 1) uniform sampler2D sampler_terrain_1;
layout(binding = 2) uniform sampler2D sampler_terrain_2;
layout(binding = 3) uniform sampler2D sampler_terrain_3;

void AddDirectionalLightContribution(DirectionalLight light, vec3 surface_color, vec3 surface_to_camera, inout vec3 diffuse, inout vec3 specular);
void AddPointLightContribution(PointLight light, vec3 surface_color, vec3 surface_to_camera, inout vec3 diffuse, inout vec3 specular);
void AddSpotLightContribution(SpotLight light, vec3 surface_color, vec3 surface_to_camera, inout vec3 diffuse, inout vec3 specular);

void main()
{
    vec3 ambient = vec3(0);
    vec3 diffuse = vec3(0);
    vec3 specular = vec3(0);
	//vec3 surface_color = vec3(1.0f, 1.0f, 1.0f);
	//vec3 surface_color = texture(sampler_terrain_1, vs_texcoord).rgb;
	vec3 mask_value = texture(sampler_mask, vs_texcoord).rgb;
	vec3 surface_color = texture(sampler_terrain_1, vs_texcoord).rgb * mask_value.r +
						 texture(sampler_terrain_2, vs_texcoord).rgb * mask_value.g +
						 texture(sampler_terrain_3, vs_texcoord).rgb * mask_value.b;
    vec3 surface_to_camera = camera_position_W.xyz - vs_position_W;
    float surface_to_camera_distance = length(surface_to_camera);

	// Ambient lighting.
    ambient = ambient_light.intensity.rgb * surface_color;

	// Directional lights.
	for (int i = 0; i < DIRECTIONAL_LIGHT_COUNT; ++i)
	{
		AddDirectionalLightContribution(directional_lights[i], surface_color, surface_to_camera, diffuse, specular);
	}

	// Point lights.
	for (int i = 0; i < POINT_LIGHT_COUNT; ++i)
	{
		AddPointLightContribution(point_lights[i], surface_color, surface_to_camera, diffuse, specular);
	}

	// Spot lights.
	for (int i = 0; i < SPOT_LIGHT_COUNT; ++i)
    {
        AddSpotLightContribution(spot_lights[i], surface_color, surface_to_camera, diffuse, specular);
    }

	// Sum the lighting terms.
	out_color = vec4(ambient + diffuse + specular, 1.0f);
}

void AddDirectionalLightContribution(DirectionalLight light, vec3 surface_color, vec3 surface_to_camera, inout vec3 diffuse, inout vec3 specular)
{
	// Diffuse lighting
	float diffuse_coefficient = max(0.0f, dot(vs_normal_W, -light.direction_W.xyz));
    diffuse += diffuse_coefficient * surface_color * light.intensity.rgb;

	// Specular lighting
	if (diffuse_coefficient > 0.0f)
	{
		vec3 halfway = normalize(surface_to_camera - light.direction_W.xyz);
		float specular_coefficient = pow(max(0.0f, dot(vs_normal_W, halfway)), material_specular_color.a);
		specular += specular_coefficient * material_specular_color.rgb * light.intensity.rgb;
	}
}

void AddPointLightContribution(PointLight light, vec3 surface_color, vec3 surface_to_camera, inout vec3 diffuse, inout vec3 specular)
{
	vec3 surface_to_light = light.position_W.xyz - vs_position_W;
    float surface_to_light_distance = length(surface_to_light);
    float attenuation = max(0.0f, light.cutoff - surface_to_light_distance) / light.cutoff;

    // Diffuse lighting
    float diffuse_coefficient = max(0.0f, dot(vs_normal_W, surface_to_light)) / surface_to_light_distance;
    diffuse += attenuation * diffuse_coefficient * surface_color * light.intensity.rgb;

    // Specular lighting
	if (diffuse_coefficient > 0.0f)
	{
		vec3 halfway = normalize(surface_to_camera + surface_to_light);
		float specular_coefficient = pow(max(0.0f, dot(vs_normal_W, halfway)), material_specular_color.a);
		specular += attenuation * specular_coefficient * material_specular_color.rgb * light.intensity.rgb;
	}
}

void AddSpotLightContribution(SpotLight light, vec3 surface_color, vec3 surface_to_camera, inout vec3 diffuse, inout vec3 specular)
{
	vec3 surface_to_light = light.position_W.xyz - vs_position_W;
	float surface_to_light_distance = length(surface_to_light);
	float angle = acos(dot(-surface_to_light, light.direction_W.xyz) / surface_to_light_distance);
		
	if (angle <= light.angle)
	{
		float attenuation = max(0.0f, light.cutoff - surface_to_light_distance) / light.cutoff;
			
		// Diffuse lighting
		float diffuse_coefficient = max(0.0f, dot(vs_normal_W, surface_to_light)) / surface_to_light_distance;
		diffuse += attenuation * diffuse_coefficient * surface_color * light.intensity.rgb;
		
		// Specular lighting
		if (diffuse_coefficient > 0.0f)
		{
			vec3 halfway = normalize(surface_to_camera + surface_to_light);
			float specular_coefficient = pow(max(0.0f, dot(vs_normal_W, halfway)), material_specular_color.a);
			specular += attenuation * specular_coefficient * material_specular_color.rgb * light.intensity.rgb;
		}
	}
}