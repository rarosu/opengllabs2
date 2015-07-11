/*
	Course: DV1222 HT2011
	Assignment: Ray-Casting/Tracing
	Aimed grade: G
	Author: Lars Woxberg
	Year: 2015

	This lab demonstrates a simple non-interactive raytracer. The following collision checks have been implemented:
	- Ray vs Sphere
	- Ray vs Box
	- Ray vs Triangle
*/

#pragma once

#include <GL/gl3w.h>
#include <glm/glm.hpp>
#include <SDL2/SDL.h>
#include <common/shader.h>
#include <common/camera.h>
#include <string>
#include "geometry.hpp"

const std::string WINDOW_TITLE = "Raytracing";
const std::string DIRECTORY_SHADERS = "../../../code/raytracing/shaders/";
const std::string FILE_OVERLAY_VS = "overlay.vert";
const std::string FILE_OVERLAY_FS = "overlay.frag";
const int VIEWPORT_WIDTH_INITIAL = 800;
const int VIEWPORT_HEIGHT_INITIAL = 600;
const float PERSPECTIVE_NEAR = 1.0f;
const float PERSPECTIVE_FAR = 100.0f;
const float PERSPECTIVE_FOV = glm::radians(75.0f);
const int TEXTURE_UNIT_DIFFUSE = 0;
const int SPHERE_COUNT = 2;
const int BOX_COUNT = 2;
const int TRIANGLE_COUNT = 2;

template <typename T>
struct Entity
{
	T geometry;
	glm::vec3 color;
};

struct PointLight
{
	glm::vec3 position;
	glm::vec3 color;
	float cutoff;
};

class Raytracing
{
public:
	Raytracing();
	~Raytracing();
private:
	SDL_Window* window;
	SDL_GLContext glcontext;
	Frustum camera_frustum;
	Camera camera;
	GLuint sampler;
	GLuint overlay_texture;
	GLuint overlay_position_vbo;
	GLuint overlay_texcoord_vbo;
	GLuint overlay_vao;
	GLuint overlay_vs;
	GLuint overlay_fs;
	GLuint overlay_program;
	unsigned int viewport_width;
	unsigned int viewport_height;
	bool running;
	Entity<Sphere> spheres[SPHERE_COUNT];
	Entity<OBB> boxes[BOX_COUNT];
	Entity<Triangle> triangles[TRIANGLE_COUNT];
	PointLight light;

	void SetupContext();
	void SetupResources();
	void Run();
	void HandleEvents();
	void RenderScene();
	void RaytraceTexture();
	Ray GetRayFromScreenCoordinates(int screen_x, int screen_y) const;
	glm::vec3 IntersectRayVsScene(const Ray& ray) const;
};