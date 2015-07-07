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

in vec3 vs_position_W;
in vec3 vs_normal_W;
in vec2 vs_texcoord;
in vec4 vs_position_L[SPOT_LIGHT_COUNT_MAX];

out vec4 out_color;

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

layout(binding = 0) uniform sampler2D sampler_diffuse;
layout(binding = 1) uniform sampler2DArray sampler_shadowmap;

void AddSpotLightContribution(SpotLight light, vec3 surface_color, vec3 surface_to_camera, inout vec3 diffuse, inout vec3 specular);

void main()
{
    vec3 ambient = vec3(0);
    vec3 diffuse = vec3(0);
    vec3 specular = vec3(0);
    vec3 surface_color = texture(sampler_diffuse, vs_texcoord).rgb;
    vec3 surface_to_camera = camera_position_W.xyz - vs_position_W;
    float surface_to_camera_distance = length(surface_to_camera);

	// Ambient lighting.
    ambient = ambient_light.intensity.rgb * surface_color;

	// Spot lights.
	for (int i = 0; i < spot_light_count; ++i)
	{
		//AddSpotLightContribution(spot_lights[i], surface_color, surface_to_camera, diffuse, specular);

		vec4 position_L = vs_position_L[i] / vs_position_L[i].w;
		vec3 shadowcoords = vec3(position_L.s, position_L.t, i);

		if (shadowcoords.x < 0.0f || shadowcoords.x > 1.0f || shadowcoords.y < 0.0f || shadowcoords.y > 1.0f || position_L.z < 0.0f || position_L.z > 1.0f)
		{
			
		}
		else
		{
			if (position_L.z - 0.001f < texture(sampler_shadowmap, shadowcoords).r)
			{
				// Do light calculations as we are in front of occluding geometry.
				AddSpotLightContribution(spot_lights[i], surface_color, surface_to_camera, diffuse, specular);
			}
		}

		/*
		if (shadowcoords.x >= 0.0f && shadowcoords.x <= 1.0f && shadowcoords.y >= 0.0f && shadowcoords.y <= 1.0f && position_L.z >= 0.0f && position_L.z <= 1.0f)
		{
			if (position_L.z - 0.001f < texture(sampler_shadowmap, shadowcoords).r)
			{
				AddSpotLightContribution(spot_lights[i], surface_color, surface_to_camera, diffuse, specular);
			}
		}
		*/
	}

	// Sum the lighting terms.
	out_color = vec4(ambient + diffuse + specular, 1.0f);
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