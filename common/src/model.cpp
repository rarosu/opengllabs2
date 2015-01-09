#include "../include/common/model.h"
#include <fstream>
#include <string>
#include <sstream>

bool LoadOBJ(const char* filepath, std::vector<glm::vec3>& positions, std::vector<glm::vec3>& normals, std::vector<glm::vec2>& texcoords)
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

					positions.push_back(positionLUT[v]);
					texcoords.push_back(texcoordLUT[vt]);
					normals.push_back(normalLUT[vn]);
				}
				
			}
		}
	}
	else
	{
		result = false;
	}

	if (!result)
	{
		positions.clear();
		normals.clear();
		texcoords.clear();
	}

	return result;
}