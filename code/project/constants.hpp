#pragma once

#include <string>
#include <glm/glm.hpp>

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

const int UNIFORM_BINDING_CONSTANT = 0;
const int UNIFORM_BINDING_FRAME = 1;
const int UNIFORM_BINDING_INSTANCE = 2;
const int TEXTURE_UNIT_DIFFUSE = 0;

const int VIEWPORT_WIDTH_INITIAL = 800;
const int VIEWPORT_HEIGHT_INITIAL = 600;
const float PERSPECTIVE_NEAR = 1.0f;
const float PERSPECTIVE_FAR = 100.0f;
const float PERSPECTIVE_FOV = glm::radians(75.0f);
const float CAMERA_SENSITIVITY = 0.005f;
const float CAMERA_MOVE_SPEED = 10.0f;
const float MODEL_ROTATION_SPEED = 1.0f;