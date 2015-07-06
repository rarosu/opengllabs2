#pragma once

#define GLM_FORCE_RADIANS

#include <glm/glm.hpp>

struct Frustum
{
	float near_z;
	float far_z;
	float fovY;
	float width;
	float height;

	Frustum();
	Frustum(float near_z, float far_z, float fovY, float width, float height);
	glm::mat4 GetPerspectiveProjection() const;
};

class Camera
{
public:
	Camera();
	
	void SetProjection(const glm::mat4& projection);
	void SetPosition(const glm::vec3& position);
	void SetFacing(const glm::vec3& facing);
	void LookAt(const glm::vec3& target);

	const glm::vec3& GetPosition() const;
	const glm::vec3& GetFacing() const;
	glm::vec3 GetRight() const;
	const glm::mat4& GetView() const;
	const glm::mat4& GetProjection() const;
	const glm::mat4& GetProjectionView() const;

	void RecalculateMatrices();
private:
	glm::vec3 position;
	glm::vec3 facing;
	glm::mat4 view;
	glm::mat4 projection;
	glm::mat4 projectionView;
};