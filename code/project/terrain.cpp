#include "terrain.hpp"
#include <common/shader.h>
#include <iostream>
#include <vector>

const float Heightmap::LOWEST_HEIGHT = 0.0f;
const float Heightmap::HIGHEST_HEIGHT = 20.0f;
const float Heightmap::HEIGHT_STEP = (Heightmap::HIGHEST_HEIGHT - Heightmap::LOWEST_HEIGHT) / 255.0f;

Heightmap::Heightmap()
{
	
	gli::storage heightmap_image = gli::load_dds((DIRECTORY_TEXTURES + FILE_HEIGHTMAP_TEXTURE).c_str());
	
	if (heightmap_image.dimensions(0).x != HEIGHTMAP_RESOLUTION_X || heightmap_image.dimensions(0).y != HEIGHTMAP_RESOLUTION_Y)
		throw std::runtime_error("Heightmap texture of invalid dimensions.");
	if (heightmap_image.format() != gli::format::FORMAT_A8_UNORM)
		throw std::runtime_error("Heightmap texture of invalid format.");
	
	// Calculate the height values.
	for (int y = 0; y < HEIGHTMAP_RESOLUTION_Y; ++y)
	{
		for (int x = 0; x < HEIGHTMAP_RESOLUTION_X; ++x)
		{
			int i = y * HEIGHTMAP_RESOLUTION_X + x;
			heights[i] = LOWEST_HEIGHT + HEIGHT_STEP * heightmap_image.data()[i];
		}
	}

	// Calculate the normals.
	// Algorithm source: http://www.flipcode.com/archives/Calculating_Vertex_Normals_for_Height_Maps.shtml
	for (int y = 0; y < HEIGHTMAP_RESOLUTION_Y; ++y)
	{
		for (int x = 0; x < HEIGHTMAP_RESOLUTION_X; ++x)
		{
			int i = y * HEIGHTMAP_RESOLUTION_X + x;

			float sx = GetHeight((x < HEIGHTMAP_RESOLUTION_X - 1 ? x + 1 : x), y) - GetHeight((x > 0 ? x - 1 : x), y);
			float sy = GetHeight(x, (y < HEIGHTMAP_RESOLUTION_Y - 1 ? y + 1 : y)) - GetHeight(x, (y > 0 ? y - 1 : y));

			if (x == 0 || x == HEIGHTMAP_RESOLUTION_X - 1)
				sx *= 2;
			if (y == 0 || y == HEIGHTMAP_RESOLUTION_Y - 1)
				sy *= 2;

			normals[i] = glm::normalize(glm::vec3(-sx, 2, sy));
		}
	}
}

float Heightmap::GetHeight(int x, int y) const
{
	//return 0.0f;
	return heights[y * HEIGHTMAP_RESOLUTION_X + x];
}

const glm::vec3& Heightmap::GetNormal(int x, int y) const
{
	return normals[y * HEIGHTMAP_RESOLUTION_X + x];
}



const float Terrain::TERRAIN_WIDTH = 128.0f;
const float Terrain::TERRAIN_HEIGHT = 128.0f;

Terrain::Terrain()
{
	// Generate the vertex data.
	float xstride = 1.0f / Heightmap::HEIGHTMAP_RESOLUTION_X;
	float ystride = 1.0f / Heightmap::HEIGHTMAP_RESOLUTION_Y;
	std::vector<glm::vec3> positions(VERTEX_COUNT);
	std::vector<glm::vec3> normals(VERTEX_COUNT);
	std::vector<glm::vec2> texcoords(VERTEX_COUNT);
	int count = 0;
	for (int quady = 0; quady < Heightmap::HEIGHTMAP_RESOLUTION_Y - 1; ++quady)
	{
		for (int quadx = 0; quadx < Heightmap::HEIGHTMAP_RESOLUTION_X - 1; ++quadx)
		{
			int x = quadx;
			int y = quady;
			float heights[] = { heightmap.GetHeight(x, y), heightmap.GetHeight(x + 1, y), heightmap.GetHeight(x + 1, y + 1), heightmap.GetHeight(x, y + 1) };
			//float heights[] = { 0.0f, 0.0f, 0.0f, 0.0f };

			positions[count] = glm::vec3(x * xstride * TERRAIN_WIDTH, heights[0], y * ystride * TERRAIN_HEIGHT);
			normals[count] = heightmap.GetNormal(x, y);
			texcoords[count] = glm::vec2(x * xstride, 1.0f - y * ystride);
			count++;

			y++;
			positions[count] = glm::vec3(x * xstride * TERRAIN_WIDTH, heights[3], y * ystride * TERRAIN_HEIGHT);
			normals[count] = heightmap.GetNormal(x, y);
			texcoords[count] = glm::vec2(x * xstride, 1.0f - y * ystride);
			count++;

			x++;
			positions[count] = glm::vec3(x * xstride * TERRAIN_WIDTH, heights[2], y * ystride * TERRAIN_HEIGHT);
			normals[count] = heightmap.GetNormal(x, y);
			texcoords[count] = glm::vec2(x * xstride, 1.0f - y * ystride);
			count++;

			y--;
			x--;
			positions[count] = glm::vec3(x * xstride * TERRAIN_WIDTH, heights[0], y * ystride * TERRAIN_HEIGHT);
			normals[count] = heightmap.GetNormal(x, y);
			texcoords[count] = glm::vec2(x * xstride, 1.0f - y * ystride);
			count++;

			x++;
			y++;
			positions[count] = glm::vec3(x * xstride * TERRAIN_WIDTH, heights[2], y * ystride * TERRAIN_HEIGHT);
			normals[count] = heightmap.GetNormal(x, y);
			texcoords[count] = glm::vec2(x * xstride, 1.0f - y * ystride);
			count++;

			y--;
			positions[count] = glm::vec3(x * xstride * TERRAIN_WIDTH, heights[1], y * ystride * TERRAIN_HEIGHT);
			normals[count] = heightmap.GetNormal(x, y);
			texcoords[count] = glm::vec2(x * xstride, 1.0f - y * ystride);
			count++;
		}
	}

	// Generate the buffers.
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	
	glGenBuffers(1, &position_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, position_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * VERTEX_COUNT, &positions[0], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glGenBuffers(1, &normal_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, normal_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * VERTEX_COUNT, &normals[0], GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glGenBuffers(1, &texcoord_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, texcoord_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * VERTEX_COUNT, &texcoords[0], GL_STATIC_DRAW);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	// Load the program.
	terrain_vs = CompileShaderFromFile((DIRECTORY_SHADERS + FILE_TERRAIN_VS).c_str(), GL_VERTEX_SHADER);
	terrain_fs = CompileShaderFromFile((DIRECTORY_SHADERS + FILE_TERRAIN_FS).c_str(), GL_FRAGMENT_SHADER);
	terrain_program = glCreateProgram();
	glAttachShader(terrain_program, terrain_vs);
	glAttachShader(terrain_program, terrain_fs);
	LinkProgram(terrain_program);

	// Setup the uniform buffer.
	glGenBuffers(1, &uniform_buffer);

	uniform_data.material_specular_color = glm::vec4(0.4f, 0.4f, 0.4f, 10.0f);

	glBindBufferBase(GL_UNIFORM_BUFFER, UNIFORM_BINDING_INSTANCE, uniform_buffer);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(UniformBufferPerInstance), &uniform_data, GL_STATIC_DRAW);
}

Terrain::~Terrain()
{

}

void Terrain::Render()
{
	glUseProgram(terrain_program);

	glBindBufferBase(GL_UNIFORM_BUFFER, UNIFORM_BINDING_INSTANCE, uniform_buffer);
	glBindVertexArray(vao);
	glDrawArrays(GL_TRIANGLES, 0, VERTEX_COUNT);
}