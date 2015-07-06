/*
	Course: DV1222 HT2011
	Assignment: Transformation and lighting
	Aimed grade: G
	Author: Lars Woxberg
	Year: 2015

	This lab demonstrates a rotating, textured cube lit by a point light.

	Camera controls:
		Move: W, A, S, D.
		Pan: Hold left mouse button and drag.
*/

#pragma once

#define GLM_FORCE_RADIANS

#include <GL/gl3w.h>
#include <glm/glm.hpp>
#include <common/model.h>
#include <common/shader.h>
#include <common/camera.h>
#include <SDL2/SDL.h>
#include <string>

const std::string DIRECTORY_ASSETS_ROOT = "../../../assets/";
const std::string DIRECTORY_SHADERS = "../../../code/lighting/shaders/";
const std::string DIRECTORY_MODELS = DIRECTORY_ASSETS_ROOT + "models/";
const std::string DIRECTORY_TEXTURES = DIRECTORY_ASSETS_ROOT + "textures/";
const std::string FILE_CUBE_MODEL = "crate.obj";
const std::string FILE_MESH_VS = "mesh_textured.vert";
const std::string FILE_MESH_FS = "mesh_textured.frag";
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

struct PointLight
{
	glm::vec4 position_W;
	glm::vec4 intensity;
	float cutoff;
	float padding[3];
};

struct UniformBufferConstant
{
	AmbientLight ambient_light;
	PointLight point_light;
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

class Lighting
{
public:
	Lighting();
	~Lighting();
private:
	SDL_Window* window;
	SDL_GLContext glcontext;
	Frustum camera_frustum;
	Camera camera;
	InputState input_state_current;
	InputState input_state_previous;
	UniformBufferConstant uniform_data_constant;
	UniformBufferPerFrame uniform_data_frame;
	UniformBufferPerInstance uniform_data_cube;
	float cube_angle;
	GLuint cube_vertex_count;
	GLuint cube_vbo_positions;
	GLuint cube_vbo_normals;
	GLuint cube_vbo_texcoords;
	GLuint cube_vao;
	GLuint cube_texture;
	GLuint mesh_vs;
	GLuint mesh_fs;
	GLuint mesh_program;
	GLuint uniform_buffer_constant;
	GLuint uniform_buffer_frame;
	GLuint uniform_buffer_cube;
	GLuint diffuse_sampler;
	unsigned int viewport_width;
	unsigned int viewport_height;
	bool running;

	void SetupContext();
	void SetupResources();
	void Run();
	void HandleEvents();
	void UpdateCamera(float dt);
	void UpdateScene(float dt);
	void RenderScene();
};