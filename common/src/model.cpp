#include "../include/common/model.h"
#include <fstream>
#include <string>
#include <sstream>

bool LoadOBJ(const char* filepath, OBJ& model)
{
	bool result = true;
	std::ifstream file(filepath);

	std::vector<glm::vec3> positionLUT;
	std::vector<glm::vec3> normalLUT;
	std::vector<glm::vec2> texcoordLUT;
	if (file.is_open())
	{
		while (!file.eof())
		{
			std::string line;
			std::stringstream ss;

			std::getline(file, line, '\n');
			ss.str(line);

			std::string identifier;
			ss >> identifier;

			if (identifier == "v")
			{
				glm::vec3 position;

				ss >> position.x;
				ss >> position.y;
				ss >> position.z;

				if (ss.fail() || ss.bad())
				{
					result = false;
					break;
				}
				
				positionLUT.push_back(position);
			}
			else if (identifier == "vn")
			{
				glm::vec3 normal;

				ss >> normal.x;
				ss >> normal.y;
				ss >> normal.z;

				if (ss.fail() || ss.bad())
				{
					result = false;
					break;
				}

				normalLUT.push_back(normal);
			}
			else if (identifier == "vt")
			{
				glm::vec2 texcoord;

				ss >> texcoord.s;
				ss >> texcoord.t;

				if (ss.fail() || ss.bad())
				{
					result = false;
					break;
				}

				texcoordLUT.push_back(texcoord);
			}
			else if (identifier == "f")
			{
				for (int i = 0; i < 3; ++i)
				{
					unsigned int v;
					unsigned int vt;
					unsigned int vn;

					ss >> v;
					ss.ignore();
					ss >> vt;
					ss.ignore();
					ss >> vn;

					if (ss.fail() || ss.bad())
					{
						result = false;
						break;
					}

					v = v - 1;
					vt = vt - 1;
					vn = vn - 1;

					model.positions.push_back(positionLUT[v]);
					model.texcoords.push_back(texcoordLUT[vt]);
					model.normals.push_back(normalLUT[vn]);
				}
			}
			else if (identifier == "mtllib")
			{
				ss >> model.mtllib;
			}
		}
	}
	else
	{
		result = false;
	}

	if (!result)
	{
		model.positions.clear();
		model.normals.clear();
		model.texcoords.clear();
	}

	return result;
}

bool LoadMTL(const char* filepath, MTL& material)
{
	bool result = true;
	std::ifstream file(filepath);

	if (file.is_open())
	{
		while (!file.eof())
		{
			std::string line;
			std::stringstream ss;

			std::getline(file, line, '\n');
			ss.str(line);

			std::string identifier;
			ss >> identifier;

			if (identifier == "Ka")
			{
				ss >> material.Ka.r;
				ss >> material.Ka.g;
				ss >> material.Ka.b;

				if (ss.fail() || ss.bad())
				{
					result = false;
					break;
				}
			}
			else if (identifier == "Kd")
			{
				ss >> material.Kd.r;
				ss >> material.Kd.g;
				ss >> material.Kd.b;

				if (ss.fail() || ss.bad())
				{
					result = false;
					break;
				}
			}
			else if (identifier == "Ks")
			{
				ss >> material.Ks.r;
				ss >> material.Ks.g;
				ss >> material.Ks.b;

				if (ss.fail() || ss.bad())
				{
					result = false;
					break;
				}
			}
			else if (identifier == "Ns")
			{
				ss >> material.Ns;

				if (ss.fail() || ss.bad())
				{
					result = false;
					break;
				}
			}
			else if (identifier == "map_Ka")
			{
				ss >> material.map_Ka;
			}
			else if (identifier == "map_Kd")
			{
				ss >> material.map_Kd;
			}
			else if (identifier == "map_Ks")
			{
				ss >> material.map_Ks;
			}
		}
	}
	else
	{
		result = false;
	}

	return result;
}