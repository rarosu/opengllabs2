#pragma once

#include <string>
#include <glm/glm.hpp>
#define NOMINMAX
#include <GL/gl3w.h>
#include "constants.hpp"

/*
	Generate a random number between 0.0f and 1.0f.
*/
float RandomFloat();

/*
	Particle base class.
*/
class ParticleEmitter
{
public:
	virtual ~ParticleEmitter();
	virtual void Update(float dt) = 0;
	void Render();
protected:
	ParticleEmitter(const glm::vec3& origin, int particle_count, const std::string& texture_file);

	void GenerateBuffers(glm::vec3* positions);
	void UpdateBuffers(glm::vec3* positions);
private:
	glm::vec3 origin;
	UniformBufferPerInstance uniform_data;
	GLuint position_vbo;
	GLuint vao;
	GLuint particle_vs;
	GLuint particle_gs;
	GLuint particle_fs;
	GLuint particle_program;
	GLuint texture;
	GLuint sampler;
	GLuint uniform_buffer;
	GLuint particle_count;
};

/*
	Emits particles in a stream upwards.
*/
class ShaftEmitter : public ParticleEmitter
{
public:
	ShaftEmitter(const glm::vec3& origin);

	void Update(float dt);
private:
	static const int PARTICLE_COUNT = 128;

	glm::vec3 particle_positions[PARTICLE_COUNT];
	glm::vec3 particle_velocities[PARTICLE_COUNT];
	glm::vec3 particle_accelerations[PARTICLE_COUNT];
	float particle_time_lived[PARTICLE_COUNT];
	float particle_time_death[PARTICLE_COUNT];
	
	void GenerateNewParticle(int index);
};

/*
	Emits a slowly expanding particle cloud.
*/
class SmokeEmitter : public ParticleEmitter
{
public:
	SmokeEmitter(const glm::vec3& origin);

	void Update(float dt);
private:
	static const int PARTICLE_COUNT = 128;

	glm::vec3 particle_positions[PARTICLE_COUNT];
	glm::vec3 particle_directions[PARTICLE_COUNT];
	float particle_time_lived[PARTICLE_COUNT];
	float particle_time_death[PARTICLE_COUNT];

	void GenerateNewParticle(int index);
};

/*
	Emits particles orbiting a point.
*/
class OrbitEmitter : public ParticleEmitter
{
public:
	OrbitEmitter(const glm::vec3& origin);

	void Update(float dt);
private:
	static const int PARTICLE_COUNT = 64;

	glm::vec3 particle_positions[PARTICLE_COUNT];
	glm::vec3 particle_orbit_planes[PARTICLE_COUNT];
	float particle_radius[PARTICLE_COUNT];
	float particle_angles[PARTICLE_COUNT];

	void GenerateNewParticle(int index);
};