#pragma once

#include <glm/glm.hpp>

struct Sphere
{
	glm::vec3 center;
	float radius;

	Sphere();
	Sphere(const glm::vec3& center, float radius);
};

struct OBB
{
	glm::vec3 center;
	glm::vec3 side_unit_vectors[3];
	float side_half_lengths[3];

	OBB();
	OBB(const glm::vec3& center, const glm::vec3& length_vector, const glm::vec3& height_vector, float width);
};

struct Triangle
{
	glm::vec3 vertices[3];

	Triangle();
	Triangle(const glm::vec3& vertex_1, const glm::vec3& vertex_2, const glm::vec3& vertex_3);
};

struct Ray
{
	glm::vec3 origin;
	glm::vec3 direction;

	Ray();
	Ray(const glm::vec3& origin, const glm::vec3& direction);

	struct Intersection
	{
		bool intersected;
		float t;
		glm::vec3 normal;

		Intersection();
		Intersection(bool intersected, float t, const glm::vec3& normal);
	};

	Intersection intersect(const Sphere& sphere) const;
	Intersection intersect(const OBB& obb) const;
	Intersection intersect(const Triangle& triangle) const;
};