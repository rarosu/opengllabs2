#pragma once

#define NOMINMAX

#include "constants.hpp"
#include <GL/gl3w.h>

class Heightmap
{
public:
	static const int HEIGHTMAP_RESOLUTION_X = 128;
	static const int HEIGHTMAP_RESOLUTION_Y = 128;
	static const float LOWEST_HEIGHT;
	static const float HIGHEST_HEIGHT;
	static const float HEIGHT_STEP;
	
	Heightmap();

	float GetHeight(int x, int y) const;
	const glm::vec3& GetNormal(int x, int y) const;
private:
	float heights[HEIGHTMAP_RESOLUTION_X * HEIGHTMAP_RESOLUTION_Y];
	glm::vec3 normals[HEIGHTMAP_RESOLUTION_X * HEIGHTMAP_RESOLUTION_Y];
};

class Terrain
{
public:
	Terrain();
	~Terrain();

	void Render();
private:
	static const float TERRAIN_WIDTH;
	static const float TERRAIN_HEIGHT;
	static const int VERTEX_COUNT = (Heightmap::HEIGHTMAP_RESOLUTION_Y - 1) * (Heightmap::HEIGHTMAP_RESOLUTION_Y - 1) * 6;

	Heightmap heightmap;
	UniformBufferPerInstance uniform_data;
	GLuint textures[3];
	GLuint mask_texture;
	GLuint sampler;
	GLuint position_vbo;
	GLuint normal_vbo;
	GLuint texcoord_vbo;
	GLuint vao;
	GLuint terrain_vs;
	GLuint terrain_fs;
	GLuint terrain_program;
	GLuint uniform_buffer;
};