#include "lrpch.h"

#include "PerspectiveCamera.h"

PerspectiveCamera::PerspectiveCamera() {
	m_CameraPosition = glm::vec3(0.0f, 0.0f, 3.0f);
	glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
	m_CameraDirection = glm::normalize(m_CameraPosition - cameraTarget);
	glm::vec3 up = { 0.0f, 1.0f, 0.0f };
	m_CameraRight = glm::normalize(glm::cross(up, m_CameraDirection));
	m_CameraUp = glm::cross(m_CameraDirection, m_CameraRight);
}
