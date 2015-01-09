#pragma once

#include <vector>
#include <glm/glm.hpp>

bool LoadOBJ(const char* filepath, std::vector<glm::vec3>& positions, std::vector<glm::vec3>& normals, std::vector<glm::vec2>& texcoords);