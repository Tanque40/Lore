#pragma once

#include <glm/glm.hpp>

class PerspectiveCamera {
private:
	glm::vec3 m_CameraPosition;
	glm::vec3 m_CameraDirection;
	glm::vec3 m_CameraUp;
	glm::vec3 m_CameraRight;
	float m_Fov = 45.f;

public:
	PerspectiveCamera();

	inline void SetPosition(const glm::vec3& position) {
		m_CameraPosition = position;
		RecalculateCameraVectors();
	}
	inline void SetDirection(const glm::vec3& direction) {
		m_CameraDirection = direction;
		RecalculateCameraVectors();
	}
	inline void SetUp(const glm::vec3& up) {
		m_CameraUp = up;
		RecalculateCameraVectors();
	}
	inline void SetRight(const glm::vec3& right) {
		m_CameraRight = right;
		RecalculateCameraVectors();
	}
	inline void SetFov(float fov) { m_Fov = fov; }

	inline const glm::vec3& GetPosition() const { return m_CameraPosition; }
	inline const glm::vec3& GetDirection() const { return m_CameraDirection; }
	inline const glm::vec3& GetUp() const { return m_CameraUp; }
	inline const glm::vec3& GetRight() const { return m_CameraRight; }
	inline float GetFov() const { return m_Fov; }

	void RecalculateCameraVectors() {
		m_CameraRight = glm::normalize(glm::cross(m_CameraDirection, m_CameraUp));
		m_CameraUp = glm::cross(m_CameraRight, m_CameraDirection);
	}

};
