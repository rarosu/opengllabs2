/*
	Course: DV1416 VT2012
	Assignment: Shadow map
	Aimed grade: 5
	Author: Lars Woxberg
	Year: 2015

	This lab demonstrates the shadow mapping technique: a rotating cube casts shadows on a plane beneath it.

	Camera controls:
		Move: W, A, S, D.
		Pan: Hold left mouse button and drag.
	Change shadow map resolution: R
	Change number of spot lights: Keys 1 - 5.
*/

#pragma once

#define GLM_FORCE_RADIANS

#include <common/model.h>
#include <common/shader.h>
#include <common/camera.h>
#include <SDL2/SDL.h>
#include <GL/gl3w.h>
#include <glm/glm.hpp>
#include <string>

const std::string DIRECTORY_ASSETS_ROOT = "../../../assets/";
const std::string DIRECTORY_SHADERS = "../../../code/shadowmapping/shaders/";
const std::string DIRECTORY_MODELS = DIRECTORY_ASSETS_ROOT + "models/";
const std::string DIRECTORY_TEXTURES = DIRECTORY_ASSETS_ROOT + "textures/";
const std::string FILE_CUBE_MODEL = "crate.obj";
const std::string FILE_PLANE_MODEL = "plane.obj";
const std::string FILE_MESH_VS = "mesh_textured_shadow.vert";
const std::string FILE_MESH_FS = "mesh_textured_shadow.frag";
const std::string FILE_DEPTH_VS = "depth.vert";
const std::string FILE_DEPTH_FS = "depth.frag";

const int UNIFORM_BINDING_CONSTANT = 0;
const int UNIFORM_BINDING_FRAME = 1;
const int UNIFORM_BINDING_INSTANCE = 2;
const int TEXTURE_UNIT_DIFFUSE = 0;
const int TEXTURE_UNIT_SHADOWMAP = 1;

const float SPOT_LIGHT_ANGLE = glm::radians(22.5f);
const int SPOT_LIGHT_COUNT_MAX = 5;
const int SPOT_LIGHT_COUNT_DEFAULT = 3;
const int SHADOWMAP_WIDTHS[] = { 256, 512, 1024, 2048 };
const int SHADOWMAP_HEIGHTS[] = { 256, 512, 1024, 2048 };
const int SHADOWMAP_RESOLUTION_COUNT = sizeof(SHADOWMAP_WIDTHS) / sizeof(int);
const int SHADOWMAP_RESOLUTION_DEFAULT = 2;
const float SHADOWMAP_NEAR = 0.5f;
const float SHADOWMAP_FAR = 100.0f;

const int VIEWPORT_WIDTH_INITIAL = 800;
const int VIEWPORT_HEIGHT_INITIAL = 600;
const float PERSPECTIVE_NEAR = 1.0f;
const float PERSPECTIVE_FAR = 100.0f;
const float PERSPECTIVE_FOV = glm::radians(75.0f);
const float CAMERA_SENSITIVITY = 0.005f;
const float CAMERA_MOVE_SPEED = 0.1f;
const float CUBE_ROTATION_SPEED = 1.0f;

struct InputState
{
	bool keys[SDL_NUM_SCANCODES];
	bool mouse_left_down;
	bool mouse_right_down;
	int mouse_x;
	int mouse_y;

	InputState();
};

struct AmbientLight
{
	glm::vec4 intensity;
};

struct SpotLight
{
	glm::mat4 light_projection_view_matrix;
	glm::vec4 position_W;
	glm::vec4 direction_W;
	glm::vec4 intensity;
	float cutoff;
	float angle;
	float padding[2];
};

struct UniformBufferConstant
{
	AmbientLight ambient_light;
	SpotLight spot_lights[SPOT_LIGHT_COUNT_MAX];
	glm::mat4 bias_matrix;
	int spot_light_count;
	float padding[3];
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

struct Entity
{
	UniformBufferPerInstance uniform_data;
	GLuint vertex_count;
	GLuint vbo_positions;
	GLuint vbo_normals;
	GLuint vbo_texcoords;
	GLuint vao;
	GLuint texture;
	GLuint uniform_buffer;

	Entity();
};

class Shadowmapping
{
public:
	Shadowmapping();
	~Shadowmapping();
private:
	SDL_Window* window;
	SDL_GLContext glcontext;
	Frustum camera_frustum;
	Camera camera;
	InputState input_state_current;
	InputState input_state_previous;
	UniformBufferConstant uniform_data_constant;
	UniformBufferPerFrame uniform_data_frame;
	float cube_angle;
	Entity cube;
	Entity plane;
	GLuint mesh_vs;
	GLuint mesh_fs;
	GLuint mesh_program;
	GLuint uniform_buffer_constant;
	GLuint uniform_buffer_frame;
	GLuint diffuse_sampler;
	GLuint depth_vs;
	GLuint depth_fs;
	GLuint depth_program;
	GLuint shadowmap_texture_array;
	GLuint shadowmap_sampler;
	GLuint shadowmap_fbo;
	unsigned int shadowmap_width;
	unsigned int shadowmap_height;
	unsigned int viewport_width;
	unsigned int viewport_height;
	int shadowmap_resolution_index;
	bool running;

	void SetupContext();
	void SetupResources();
	void LoadModel(const char* filepath, Entity& entity);
	void Run();
	void HandleEvents();
	void UpdateCamera(float dt);
	void UpdateScene(float dt);
	void RenderScene();
	void RenderDepth();
	void UpdateShadowmapResources(int resolution_index, int spot_light_count);
};