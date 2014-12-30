#include "../include/common/camera.h"
#include <glm/gtx/transform.hpp>

Camera::Camera()
	: position(0, 0, 0)
	, facing(0, 0, -1)
{

}

void Camera::SetProjection(const glm::mat4& projection)
{
	this->projection = projection;
}

void Camera::SetPosition(const glm::vec3& position)
{
	this->position = position;
}

void Camera::SetFacing(const glm::vec3& facing)
{
	this->facing = glm::normalize(facing);
}

void Camera::LookAt(const glm::vec3& target)
{
	facing = glm::normalize(target - position);
}


const glm::vec3& Camera::GetPosition() const
{
	return position;
}

const glm::vec3& Camera::GetFacing() const
{
	return facing;
}

const glm::mat4& Camera::GetView() const
{
	return view;
}

const glm::mat4& Camera::GetProjection() const
{
	return projection;
}

const glm::mat4& Camera::GetProjectionView() const
{
	return projectionView;
}


void Camera::RecalculateMatrices()
{
	view = glm::lookAt(position, position + facing, glm::vec3(0, 1, 0));

	projectionView = projection * view;
}

glm::mat4 Camera::GetPerspectiveProjection(float near, float far, float fovY, float width, float height)
{
	float aspect = width / height;
	float fovScale = 1.0f / std::tan(fovY * 0.5f);
	return glm::mat4(1.0f / (fovScale * aspect), 0.0f, 0.0f, 0.0f,
					 0.0f, 1.0f / fovScale, 0.0f, 0.0f,
					 0.0f, 0.0f, (near + far) / (near - far), -1.0f,
					 0.0f, 0.0f, 2 * near * far / (near - far), 0.0f);
}