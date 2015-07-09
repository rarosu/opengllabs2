#pragma once

#include <string>
#include <glm/glm.hpp>
#include <gli/gli.hpp>

struct AmbientLight
{
	glm::vec4 intensity;
};

struct DirectionalLight
{
	glm::vec4 direction_W;
	glm::vec4 intensity;
};

struct PointLight
{
	glm::vec4 position_W;
	glm::vec4 intensity;
	float cutoff;
	float padding[3];
};

struct SpotLight
{
	glm::vec4 position_W;
	glm::vec4 direction_W;
	glm::vec4 intensity;
	float cutoff;
	float angle;
	float padding[2];
};

const int POINT_LIGHT_COUNT = 2;
const int DIRECTIONAL_LIGHT_COUNT = 2;
const int SPOT_LIGHT_COUNT = 2;

struct UniformBufferConstant
{
	AmbientLight ambient_light;
	DirectionalLight directional_lights[DIRECTIONAL_LIGHT_COUNT];
	PointLight point_lights[POINT_LIGHT_COUNT];
	SpotLight spot_lights[SPOT_LIGHT_COUNT];
};

struct UniformBufferPerFrame
{
	glm::mat4 view_matrix;
	glm::mat4 projection_matrix;
	glm::vec4 camera_position_W;
};

struct UniformBufferPerInstance
{
	glm::mat4 model_matrix;
	glm::mat4 normal_matrix;
	glm::vec4 material_specular_color;
};

const std::string WINDOW_TITLE = "Project";
const std::string DIRECTORY_ASSETS_ROOT = "../../../assets/";
const std::string DIRECTORY_SHADERS = "../../../code/project/shaders/";
const std::string DIRECTORY_MODELS = DIRECTORY_ASSETS_ROOT + "models/";
const std::string DIRECTORY_TEXTURES = DIRECTORY_ASSETS_ROOT + "textures/";
const std::string FILE_PARTICLE_VS = "particle.vert";
const std::string FILE_PARTICLE_GS = "particle.geom";
const std::string FILE_PARTICLE_FS = "particle.frag";
const std::string FILE_PARTICLE_SHAFT_TEXTURE = "shaft.dds";
const std::string FILE_PARTICLE_SMOKE_TEXTURE = "smoke.dds";
const std::string FILE_HEIGHTMAP_TEXTURE = "heightmap.dds";
const std::string FILE_TERRAIN_VS = "terrain.vert";
const std::string FILE_TERRAIN_FS = "terrain.frag";
const std::string FILE_TERRAIN_TEXTURE_1 = "stk_generic_grassb.dds";
const std::string FILE_TERRAIN_TEXTURE_2 = "stktex_generic_earth_a.dds";
const std::string FILE_TERRAIN_TEXTURE_3 = "dirt_5.dds";
const std::string FILE_TERRAIN_MASK = "terrain_mask.dds";

const int UNIFORM_BINDING_CONSTANT = 0;
const int UNIFORM_BINDING_FRAME = 1;
const int UNIFORM_BINDING_INSTANCE = 2;
const int TEXTURE_UNIT_DIFFUSE = 0;
const int TEXTURE_UNIT_TERRAIN_MASK = 0;
const int TEXTURE_UNIT_TERRAIN_1 = 1;
const int TEXTURE_UNIT_TERRAIN_2 = 2;
const int TEXTURE_UNIT_TERRAIN_3 = 3;

const int VIEWPORT_WIDTH_INITIAL = 800;
const int VIEWPORT_HEIGHT_INITIAL = 600;
const float PERSPECTIVE_NEAR = 1.0f;
const float PERSPECTIVE_FAR = 100.0f;
const float PERSPECTIVE_FOV = glm::radians(75.0f);
const float CAMERA_SENSITIVITY = 0.005f;
const float CAMERA_MOVE_SPEED = 10.0f;
const float MODEL_ROTATION_SPEED = 1.0f;