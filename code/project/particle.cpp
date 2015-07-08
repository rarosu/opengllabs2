#include "particle.hpp"
#include <common/shader.h>
#include <gli/gli.hpp>
#include <glm/gtx/transform.hpp>
#include <iostream>

float RandomFloat()
{
	const int PRECISION = 10000;
	return static_cast<float>(rand() % PRECISION) / PRECISION;
}

ShaftEmitter::ShaftEmitter(const glm::vec3& origin)
	: origin(origin)
	, position_vbo(0)
	, vao(0)
	, particle_vs(0)
	, particle_gs(0)
	, particle_fs(0)
	, particle_program(0)
	, texture(0)
	, sampler(0)
	, uniform_buffer(0)
{
	// Set initial values for the particles.
	for (int i = 0; i < PARTICLE_COUNT; ++i)
	{
		//GenerateNewParticle(i);
		particle_time_lived[i] = 0.0f;
		particle_time_death[i] = RandomFloat() * 2.0f;
	}

	// Setup the initial vertex buffer.
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	
	glGenBuffers(1, &position_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, position_vbo);
	glBufferData(GL_ARRAY_BUFFER, PARTICLE_COUNT * sizeof(glm::vec3), particle_positions, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glEnableVertexAttribArray(0);

	// Load the program.
	particle_vs = CompileShaderFromFile((DIRECTORY_SHADERS + FILE_PARTICLE_VS).c_str(), GL_VERTEX_SHADER);
	particle_gs = CompileShaderFromFile((DIRECTORY_SHADERS + FILE_PARTICLE_GS).c_str(), GL_GEOMETRY_SHADER);
	particle_fs = CompileShaderFromFile((DIRECTORY_SHADERS + FILE_PARTICLE_FS).c_str(), GL_FRAGMENT_SHADER);
	particle_program = glCreateProgram();
	glAttachShader(particle_program, particle_vs);
	glAttachShader(particle_program, particle_gs);
	glAttachShader(particle_program, particle_fs);
	LinkProgram(particle_program);

	// Generate a texture sampler.
	glGenSamplers(1, &sampler);
	glSamplerParameteri(sampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glSamplerParameteri(sampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glSamplerParameteri(sampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// Load the particle texture.
	gli::storage texture_image = gli::load_dds((DIRECTORY_TEXTURES + FILE_PARTICLE_SHAFT_TEXTURE).c_str());
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, texture_image.dimensions(0).x, texture_image.dimensions(0).y, 0, GL_BGRA, GL_UNSIGNED_BYTE, texture_image.data());

	// Setup the initial uniform buffer.
	uniform_data.model_matrix = glm::translate(origin);
	
	glGenBuffers(1, &uniform_buffer);
	glBindBufferBase(GL_UNIFORM_BUFFER, UNIFORM_BINDING_INSTANCE, uniform_buffer);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(UniformBufferPerInstance), &uniform_data, GL_STATIC_DRAW);
}

ShaftEmitter::~ShaftEmitter()
{
	glDetachShader(particle_program, particle_vs);
	glDetachShader(particle_program, particle_gs);
	glDetachShader(particle_program, particle_fs);
	glDeleteProgram(particle_program);
	glDeleteShader(particle_vs);
	glDeleteShader(particle_gs);
	glDeleteShader(particle_fs);
	glDeleteBuffers(1, &position_vbo);
	glDeleteVertexArrays(1, &vao);
	glDeleteTextures(1, &texture);
	glDeleteSamplers(1, &sampler);
}

void ShaftEmitter::Update(float dt)
{
	// Update the simulation.
	for (int i = 0; i < PARTICLE_COUNT; ++i)
	{
		particle_velocities[i] += particle_accelerations[i] * dt;
		particle_positions[i] += particle_velocities[i] * dt;
		particle_time_lived[i] += dt;

		if (particle_time_lived[i] >= particle_time_death[i])
		{
			GenerateNewParticle(i);
		}
	}

	// Update the buffer.
	glBindBuffer(GL_ARRAY_BUFFER, position_vbo);
	glBufferSubData(GL_ARRAY_BUFFER, 0, PARTICLE_COUNT * sizeof(glm::vec3), particle_positions);
}

void ShaftEmitter::Render()
{
	glUseProgram(particle_program);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDepthMask(GL_FALSE);
	//glBlendFunc(GL_ONE, GL_ONE);


	glActiveTexture(GL_TEXTURE0 + TEXTURE_UNIT_DIFFUSE);
	glBindSampler(TEXTURE_UNIT_DIFFUSE, sampler);
	glBindTexture(GL_TEXTURE_2D, texture);

	glBindBufferBase(GL_UNIFORM_BUFFER, UNIFORM_BINDING_INSTANCE, uniform_buffer);
	glBindVertexArray(vao);
	glDrawArrays(GL_POINTS, 0, PARTICLE_COUNT);

	glDepthMask(GL_TRUE);
}

void ShaftEmitter::GenerateNewParticle(int index)
{
	particle_positions[index] = glm::vec3(RandomFloat() * 2.0f - 1.0f, 0.0f, RandomFloat() * 2.0f - 1.0f);
	particle_velocities[index] = glm::vec3(0.0f, 0.0f, 0.0f);
	particle_accelerations[index] = glm::vec3(0.0f, 15.0f + RandomFloat() * 2.0f, 0.0f);
	particle_time_lived[index] = 0.0f;
	particle_time_death[index] = 1.0f + RandomFloat() * 0.5f;
}