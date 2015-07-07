#version 440

#define SPOT_LIGHT_COUNT_MAX 5
#define EPSILON 0.00001

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
layout(binding = 1) uniform sampler2DArrayShadow sampler_shadowmap;

void AddSpotLightContribution(int light_index, vec3 surface_color, vec3 surface_to_camera, inout vec3 diffuse, inout vec3 specular);

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
		AddSpotLightContribution(i, surface_color, surface_to_camera, diffuse, specular);
	}

	// Sum the lighting terms.
	out_color = vec4(ambient + diffuse + specular, 1.0f);
}

float CalculateShadowFactor(int light_index)
{
	vec3 position_L = vs_position_L[light_index].xyz / vs_position_L[light_index].w;
	vec4 shadowcoords = vec4(position_L.s, position_L.t, light_index, position_L.z + EPSILON);

	return texture(sampler_shadowmap, shadowcoords).r;
}

void AddSpotLightContribution(int light_index, vec3 surface_color, vec3 surface_to_camera, inout vec3 diffuse, inout vec3 specular)
{
	SpotLight light = spot_lights[light_index];
	vec3 surface_to_light = light.position_W.xyz - vs_position_W;
	float surface_to_light_distance = length(surface_to_light);
	float angle = acos(dot(-surface_to_light, light.direction_W.xyz) / surface_to_light_distance);
		
	if (angle <= light.angle)
	{
		float attenuation = max(0.0f, light.cutoff - surface_to_light_distance) / light.cutoff;
		float shadow_factor = CalculateShadowFactor(light_index);
			
		// Diffuse lighting
		float diffuse_coefficient = max(0.0f, dot(vs_normal_W, surface_to_light)) / surface_to_light_distance;
		diffuse += attenuation * shadow_factor * diffuse_coefficient * surface_color * light.intensity.rgb;
		
		// Specular lighting
		if (diffuse_coefficient > 0.0f)
		{
			vec3 halfway = normalize(surface_to_camera + surface_to_light);
			float specular_coefficient = pow(max(0.0f, dot(vs_normal_W, halfway)), material_specular_color.a);
			specular += attenuation * shadow_factor * specular_coefficient * material_specular_color.rgb * light.intensity.rgb;
		}
	}
}