#pragma once

#include <vector>
#include <glm/glm.hpp>

struct OBJ
{
	std::vector<glm::vec3> positions;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec2> texcoords;
	std::string mtllib;
};

struct MTL
{
	glm::vec3 Ka;
	glm::vec3 Kd;
	glm::vec3 Ks;
	float Ns;
	std::string map_Ka;
	std::string map_Kd;
	std::string map_Ks;
};

bool LoadOBJ(const char* filepath, OBJ& model);
bool LoadMTL(const char* filepath, MTL& material);
