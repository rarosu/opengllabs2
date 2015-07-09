#include "particle.hpp"
#include <common/shader.h>
#include <glm/gtx/transform.hpp>
#include <iostream>

float RandomFloat()
{
	const int PRECISION = 10000;
	return static_cast<float>(rand() % PRECISION) / PRECISION;
}

ParticleEmitter::~ParticleEmitter()
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
	glDeleteBuffers(1, &uniform_buffer);
}

void ParticleEmitter::Render()
{
	glUseProgram(particle_program);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDepthMask(GL_FALSE);

	glActiveTexture(GL_TEXTURE0 + TEXTURE_UNIT_DIFFUSE);
	glBindSampler(TEXTURE_UNIT_DIFFUSE, sampler);
	glBindTexture(GL_TEXTURE_2D, texture);

	glBindBufferBase(GL_UNIFORM_BUFFER, UNIFORM_BINDING_INSTANCE, uniform_buffer);
	glBindVertexArray(vao);
	glDrawArrays(GL_POINTS, 0, particle_count);

	glDepthMask(GL_TRUE);
}

ParticleEmitter::ParticleEmitter(const glm::vec3& origin, int particle_count, const std::string& texture_file)
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
	, particle_count(particle_count)
{
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
	gli::storage texture_image = gli::load_dds((DIRECTORY_TEXTURES + texture_file).c_str());
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, texture_image.dimensions(0).x, texture_image.dimensions(0).y, 0, GL_BGRA, GL_UNSIGNED_BYTE, texture_image.data());

	// Setup the initial uniform buffer.
	uniform_data.model_matrix = glm::translate(origin);

	glGenBuffers(1, &uniform_buffer);
	glBindBufferBase(GL_UNIFORM_BUFFER, UNIFORM_BINDING_INSTANCE, uniform_buffer);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(UniformBufferPerInstance), &uniform_data, GL_STATIC_DRAW);
}

void ParticleEmitter::GenerateBuffers(glm::vec3* positions)
{
	// Setup the initial vertex buffer.
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &position_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, position_vbo);
	glBufferData(GL_ARRAY_BUFFER, particle_count * sizeof(glm::vec3), positions, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glEnableVertexAttribArray(0);
}

void ParticleEmitter::UpdateBuffers(glm::vec3* positions)
{
	// Update the buffer.
	glBindBuffer(GL_ARRAY_BUFFER, position_vbo);
	glBufferSubData(GL_ARRAY_BUFFER, 0, particle_count * sizeof(glm::vec3), positions);
}



ShaftEmitter::ShaftEmitter(const glm::vec3& origin)
	: ParticleEmitter(origin, PARTICLE_COUNT, FILE_PARTICLE_SHAFT_TEXTURE)
{
	// Set initial values for the particles.
	for (int i = 0; i < PARTICLE_COUNT; ++i)
	{
		particle_time_lived[i] = 0.0f;
		particle_time_death[i] = RandomFloat() * 2.0f;
	}

	// Create the new buffer.
	GenerateBuffers(particle_positions);
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
	UpdateBuffers(particle_positions);
}

void ShaftEmitter::GenerateNewParticle(int index)
{
	particle_positions[index] = glm::vec3(RandomFloat() * 2.0f - 1.0f, 0.0f, RandomFloat() * 2.0f - 1.0f);
	particle_velocities[index] = glm::vec3(0.0f, 0.0f, 0.0f);
	particle_accelerations[index] = glm::vec3(0.0f, 15.0f + RandomFloat() * 2.0f, 0.0f);
	particle_time_lived[index] = 0.0f;
	particle_time_death[index] = 1.0f + RandomFloat() * 0.5f;
}



SmokeEmitter::SmokeEmitter(const glm::vec3& origin)
	: ParticleEmitter(origin, PARTICLE_COUNT, FILE_PARTICLE_SMOKE_TEXTURE)
{
	// Set initial values for the particles.
	for (int i = 0; i < PARTICLE_COUNT; ++i)
	{
		particle_time_lived[i] = 0.0f;
		particle_time_death[i] = 5.0f * RandomFloat();
	}

	// Generate the buffer.
	GenerateBuffers(particle_positions);
}

void SmokeEmitter::Update(float dt)
{
	// Update the simulation.
	for (int i = 0; i < PARTICLE_COUNT; ++i)
	{
		particle_positions[i] += particle_directions[i] * dt;
		particle_time_lived[i] += dt;

		if (particle_time_lived[i] >= particle_time_death[i])
		{
			GenerateNewParticle(i);
		}
	}

	// Update the buffer.
	UpdateBuffers(particle_positions);
}

void SmokeEmitter::GenerateNewParticle(int index)
{
	particle_positions[index] = glm::vec3();
	particle_directions[index] = glm::normalize(glm::vec3(2.0f * RandomFloat() - 1.0f, 2.0f * RandomFloat() - 1.0f, 2.0f * RandomFloat() - 1.0f));
	particle_time_lived[index] = 0.0f;
	particle_time_death[index] = 5.0f + RandomFloat();
}


OrbitEmitter::OrbitEmitter(const glm::vec3& origin)
	: ParticleEmitter(origin, PARTICLE_COUNT, FILE_PARTICLE_ORBIT_TEXTURE)
{
	// Set initial values for the particles.
	for (int i = 0; i < PARTICLE_COUNT; ++i)
	{
		GenerateNewParticle(i);
	}

	// Generate the buffer.
	GenerateBuffers(particle_positions);
}

void OrbitEmitter::Update(float dt)
{
	for (int i = 0; i < PARTICLE_COUNT; ++i)
	{
		glm::vec3 e1 = glm::vec3(0.0f, -particle_orbit_planes[i].z, particle_orbit_planes[i].y);
		glm::vec3 e2 = glm::cross(particle_orbit_planes[i], e1);
		particle_angles[i] += 3.0f * dt;
		particle_positions[i] = particle_radius[i] * (e1 * std::cos(particle_angles[i]) + e2 * std::sin(particle_angles[i]));
	}

	// Update the buffer.
	UpdateBuffers(particle_positions);
}

void OrbitEmitter::GenerateNewParticle(int index)
{
	particle_orbit_planes[index] = glm::normalize(glm::vec3(2.0f * RandomFloat() - 1.0f, 2.0f * RandomFloat() - 1.0f, 2.0f * RandomFloat() - 1.0f));
	particle_radius[index] = 1.5f + 2.0f * RandomFloat();
	particle_angles[index] = 0.0f;
	particle_positions[index] = particle_radius[index] * glm::vec3(0.0f, -particle_orbit_planes[index].z, particle_orbit_planes[index].y);
}