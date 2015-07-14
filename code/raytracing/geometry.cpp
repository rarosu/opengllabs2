#include "geometry.hpp"
#include <algorithm>
#include <limits>

Sphere::Sphere()
	: center(0.0f)
	, radius(1.0f)
{

}

Sphere::Sphere(const glm::vec3& center, float radius)
	: center(center)
	, radius(radius)
{

}

OBB::OBB()
{
	center = glm::vec3(0.0f);
	side_unit_vectors[0] = glm::vec3(1.0f, 0.0f, 0.0f);
	side_unit_vectors[1] = glm::vec3(0.0f, 1.0f, 0.0f);
	side_unit_vectors[2] = glm::vec3(0.0f, 0.0f, 1.0f);
	side_half_lengths[0] = 0.5f;
	side_half_lengths[1] = 0.5f;
	side_half_lengths[2] = 0.5f;
}

OBB::OBB(const glm::vec3& center, const glm::vec3& length_vector, const glm::vec3& height_vector, float width)
	: center(center)
{
	side_half_lengths[0] = glm::length(length_vector);
	side_half_lengths[1] = glm::length(height_vector);
	side_half_lengths[2] = width;
	
	side_unit_vectors[0] = length_vector / side_half_lengths[0];
	side_unit_vectors[1] = height_vector / side_half_lengths[1];
	side_unit_vectors[2] = glm::cross(height_vector, length_vector);
	
	side_half_lengths[0] *= 0.5f;
	side_half_lengths[1] *= 0.5f;
	side_half_lengths[2] *= 0.5f;
}

Triangle::Triangle()
{
	vertices[0] = glm::vec3(0.0f, 0.0f, 0.0f);
	vertices[1] = glm::vec3(1.0f, 0.0f, 0.0f);
	vertices[2] = glm::vec3(0.0f, 1.0f, 0.0f);
}

Triangle::Triangle(const glm::vec3& vertex_1, const glm::vec3& vertex_2, const glm::vec3& vertex_3)
{
	vertices[0] = vertex_1;
	vertices[1] = vertex_2;
	vertices[2] = vertex_3;
}

Ray::Ray()
	: origin(0.0f)
	, direction(0.0f, 0.0f, 1.0f)
{

}

Ray::Ray(const glm::vec3& origin, const glm::vec3& direction)
	: origin(origin)
	, direction(direction)
{

}

Ray::Intersection::Intersection()
	: intersected(false)
	, t(0.0f)
{

}

Ray::Intersection::Intersection(bool intersected, float t, const glm::vec3& normal)
	: intersected(intersected)
	, t(t)
	, normal(normal)
{

}

Ray::Intersection Ray::intersect(const Sphere& sphere) const
{
	glm::vec3 displacement = sphere.center - origin;
	float radiusSquared = sphere.radius * sphere.radius;
	float dot = glm::dot(displacement, direction);
	float distanceSquared = glm::dot(displacement, displacement);

	// Bail out if the sphere is behind the origin AND the origin is not inside of the sphere.
	if (dot < 0 && distanceSquared < radiusSquared)
		return Intersection();

	// Calculate the distance from the center of the sphere to the closest point on the ray.
	// Bail out if this distance is larger than the radius of the sphere.
	float shortestDistanceSquared = distanceSquared - dot * dot;
	if (shortestDistanceSquared > radiusSquared)
		return Intersection();

	// We have intersection, calculate the intersection point.
	float t = 0.0f;
	float q = std::sqrt(radiusSquared - shortestDistanceSquared);
	if (distanceSquared > radiusSquared)
		t = dot - q;
	else
		t = dot + q;

	return Intersection(true, t, glm::normalize(origin + direction * t - sphere.center));
}

Ray::Intersection Ray::intersect(const OBB& obb) const
{
	// Using the Slabs Method to determine intersection point.
	float tmin = std::numeric_limits<float>::min();	// The minimum value
	float tmax = std::numeric_limits<float>::max();	// The maximum value
	glm::vec3 normal;

	glm::vec3 displacement = obb.center - origin;
	for (int i = 0; i < 3; ++i) 
	{
		float e = glm::dot(obb.side_unit_vectors[i], displacement);
		float f = glm::dot(obb.side_unit_vectors[i], direction);

		if (std::abs(f) > std::numeric_limits<float>::epsilon()) 
		{
			float t[2];
			t[0] = (e + obb.side_half_lengths[i]) / f;
			t[1] = (e - obb.side_half_lengths[i]) / f;
			if (t[0] > t[1]) std::swap(t[0], t[1]);
			if (t[0] > tmin)
			{
				normal = glm::dot(direction, obb.side_unit_vectors[i]) < 0 ? obb.side_unit_vectors[i] : -obb.side_unit_vectors[i];
				tmin = t[0];
			}
			if (t[1] < tmax)
			{
				tmax = t[1];
			}

			if (tmin > tmax) return Intersection();
			if (tmax < 0) return Intersection();
		}
		else if ((-e - obb.side_half_lengths[i] > 0) || (-e + obb.side_half_lengths[i] < 0)) 
		{
			return Intersection();
		}
	}

	if (tmin > 0)
		return Intersection(true, tmin, normal);
	else
		return Intersection(true, tmax, -normal);
}

Ray::Intersection Ray::intersect(const Triangle& triangle) const
{
	// Solve the system:
	// (-d, p1 - p0, p2 - p0) (t, u, v) = o - p0
	// where d is the direction of the ray, o is the origin and p0 through p2 are
	// the triangle vertices.

	glm::vec3 e1 = triangle.vertices[1] - triangle.vertices[0];
	glm::vec3 e2 = triangle.vertices[2] - triangle.vertices[0];
	glm::vec3 q = glm::cross(direction, e2);
	float a = glm::dot(e1, q);

	// See if the system is invertible
	if (std::abs(a) < std::numeric_limits<float>::epsilon())
		return Intersection();

	float f = 1.0f / a;

	glm::vec3 s = origin - triangle.vertices[0];
	float u = f * glm::dot(s, q);

	// Make sure the barycentric coordinates are valid
	if (u < 0.0f)
		return Intersection();

	glm::vec3 r = glm::cross(s, e1);
	float v = f * glm::dot(direction, r);

	// Make sure the barycentric coordinates are valid
	if (v < 0.0f || u + v > 1.0f)
		return Intersection();

	// Return the intersection.
	float t = f * glm::dot(e2, r);
	glm::vec3 normal = glm::normalize(glm::cross(e1, e2));
	return Intersection(true, t, glm::dot(direction, normal) < 0.0f ? normal : -normal);
}
