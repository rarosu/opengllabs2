/*
	Course: DV1222 HT2011
	Assignment: Gruppuppgift
	Aimed grade: 5
	Author: Lars Woxberg
	Year: 2015

	This project demonstrates a heightmap textured using a blendmap technique. It also includes three different types
	of particle systems and a free-moving camera.

	Camera controls:
		Move: W, A, S, D
		Pan: Hold left mouse button and drag.
	FPS Camera controls:
		Move: W, A, S, D.
		Pan: Move the mouse cursor
	Toggle camera mode: C
	Quit: Alt-F4 or Escape.
*/

#pragma once

#define GLM_FORCE_RADIANS

#include <common/model.h>
#include <common/shader.h>
#include <common/camera.h>
#include <SDL2/SDL.h>
#include <GL/gl3w.h>
#include <glm/glm.hpp>
#include <memory>
#include "constants.hpp"
#include "particle.hpp"
#include "terrain.hpp"

struct InputState
{
	bool keys[SDL_NUM_SCANCODES];
	bool mouse_left_down;
	bool mouse_right_down;
	int mouse_x;
	int mouse_y;

	InputState();
};

class Project
{
public:
	Project();
	~Project();
private:
	SDL_Window* window;
	SDL_GLContext glcontext;
	Frustum camera_frustum;
	Camera camera;
	InputState input_state_current;
	InputState input_state_previous;
	unsigned int viewport_width;
	unsigned int viewport_height;
	bool running;
	bool fps_camera;

	UniformBufferConstant uniform_data_constant;
	UniformBufferPerFrame uniform_data_frame;
	GLuint uniform_buffer_constant;
	GLuint uniform_buffer_frame;
	std::unique_ptr<ParticleEmitter> emitters[3];
	std::unique_ptr<Terrain> terrain;

	void SetupContext();
	void SetupResources();
	void Run();
	void HandleEvents();
	void UpdateCamera(float dt);
	void UpdateCameraFPS(float dt);
	void UpdateScene(float dt);
	void RenderScene();
};