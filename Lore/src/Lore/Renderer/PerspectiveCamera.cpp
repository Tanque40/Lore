#include "lrpch.h"

#include "Lore/Renderer/PerspectiveCamera.h"

namespace Lore {

	PerspectiveCamera::PerspectiveCamera() {
		m_CameraPosition = glm::vec3(0.0f, 0.0f, 3.0f);
		m_CameraDirection = glm::vec3(0.0f, 0.0f, 1.0f);
		m_CameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
		m_CameraRight = glm::vec3(1.0f, 0.0f, 0.0f);
	}

	void PerspectiveCamera::ProcessKeyboard(CameraMovement direction, float deltaTime) {
		float velocity = m_MovementSpeed * deltaTime;
		if (direction == CameraMovement::FORWARD)
			m_CameraPosition += m_CameraDirection * velocity;
		if (direction == CameraMovement::BACKWARD)
			m_CameraPosition -= m_CameraDirection * velocity;
		if (direction == CameraMovement::LEFT)
			m_CameraPosition -= m_CameraRight * velocity;
		if (direction == CameraMovement::RIGHT)
			m_CameraPosition += m_CameraRight * velocity;
	}

	void PerspectiveCamera::ProcessMouseMovement(float xOffset, float yOffset, bool constrainPitch) {
		xOffset *= m_MouseSensitivity;
		yOffset *= m_MouseSensitivity;

		m_Yaw += xOffset;
		m_Pitch += yOffset;

		// make sure that when pitch is out of bounds, screen doesn't get flipped
		if (constrainPitch) {
			if (m_Pitch > 89.0f)
				m_Pitch = 89.0f;
			if (m_Pitch < -89.0f)
				m_Pitch = -89.0f;
		}

		// update Front, Right and Up Vectors using the updated Euler angles
		UpdateCameraVectors();
	}

	void PerspectiveCamera::SetDirection(const glm::vec3& direction) {
		// UpdateCameraVectors() reconstruye m_CameraDirection a partir de m_Yaw/m_Pitch,
		// así que hay que derivar esos ángulos de la dirección pedida o quedaría pisada
		// por el valor anterior de yaw/pitch en la siguiente llamada.
		glm::vec3 d = glm::normalize(direction);
		m_Pitch = glm::degrees(asin(glm::clamp(d.y, -1.0f, 1.0f)));
		m_Yaw = glm::degrees(atan2(d.z, d.x));

		// Igual que en ProcessMouseMovement: evita el pitch a +/-90 exactos, donde
		// front y m_CameraUp quedan paralelos y cross() colapsa a un vector nulo.
		m_Pitch = glm::clamp(m_Pitch, -89.0f, 89.0f);

		UpdateCameraVectors();
	}

	void PerspectiveCamera::ProcessMouseScroll(float yOffset) {
		m_Fov -= (float)yOffset;
		if (m_Fov < 1.0f)
			m_Fov = 1.0f;
		if (m_Fov > 90.0f)
			m_Fov = 90.0f;
	}

	void PerspectiveCamera::UpdateCameraVectors() {
		glm::vec3 front;
		front.x = cos(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
		front.y = sin(glm::radians(m_Pitch));
		front.z = sin(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
		m_CameraDirection = glm::normalize(front);

		m_CameraRight = glm::normalize(glm::cross(m_CameraDirection, m_CameraUp));
		m_CameraUp = glm::normalize(glm::cross(m_CameraRight, m_CameraDirection));
	}

}
