#pragma once

#include <string>
#include <glm/glm.hpp>
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
	virtual void Update(float dt) = 0;
	virtual void Render() = 0;
private:
	
};

/*
	Emits particles in a stream upwards.
*/
class ShaftEmitter : public ParticleEmitter
{
public:
	ShaftEmitter(const glm::vec3& origin);
	~ShaftEmitter();

	void Update(float dt);
	void Render();
private:
	static const int PARTICLE_COUNT = 128;

	glm::vec3 origin;
	glm::vec3 particle_positions[PARTICLE_COUNT];
	glm::vec3 particle_velocities[PARTICLE_COUNT];
	glm::vec3 particle_accelerations[PARTICLE_COUNT];
	float particle_time_lived[PARTICLE_COUNT];
	float particle_time_death[PARTICLE_COUNT];

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
	
	
	void GenerateNewParticle(int index);
};

/*
	Emits a slowly expanding particle cloud.
*/
class SmokeEmitter : public ParticleEmitter
{

};

/*
	Emits particles orbiting a point.
*/
class OrbitEmitter : public ParticleEmitter
{

};