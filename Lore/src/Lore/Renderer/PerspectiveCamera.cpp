#include "lrpch.h"

#include "PerspectiveCamera.h"

PerspectiveCamera::PerspectiveCamera() {
	m_CameraPosition = glm::vec3(0.0f, 0.0f, 3.0f);
	m_CameraDirection = glm::vec3(0.0f, 0.0f, 1.0f);
	m_CameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
	m_CameraRight = glm::vec3(1.0f, 0.0f, 0.0f);
}
