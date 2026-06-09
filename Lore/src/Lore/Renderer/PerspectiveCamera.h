#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Lore {

	enum class CameraMovement {
		FORWARD,
		BACKWARD,
		LEFT,
		RIGHT
	};

	class PerspectiveCamera {
	private:
		float m_Yaw = 0.0f; // Mirando hacia el eje Z negativo
		float m_Pitch = 0.0f; // Sin inclinación vertical
		float m_MouseSensitivity = 0.15f;
		float m_Fov = 45.f;
		glm::vec3 m_CameraPosition;
		glm::vec3 m_CameraDirection;
		glm::vec3 m_CameraUp;
		glm::vec3 m_CameraRight;
		float m_MovementSpeed = 5.0f;

	public:
		PerspectiveCamera();

		void ProcessKeyboard(CameraMovement direction, float deltaTime);
		void ProcessMouseMovement(float xOffset, float yOffset, bool constrainPitch = true);
		void ProcessMouseScroll(float yOffset);

		inline void SetPosition(const glm::vec3& position) { m_CameraPosition = position; UpdateCameraVectors(); }
		inline void SetDirection(const glm::vec3& direction) { m_CameraDirection = direction; UpdateCameraVectors(); }

		glm::vec3 GetPosition() const { return m_CameraPosition; }
		glm::vec3 GetDirection() const { return m_CameraDirection; }
		glm::vec3 GetUp() const { return m_CameraUp; }
		glm::vec3 GetRight() const { return m_CameraRight; }
		float GetFov() const { return m_Fov; }

		float* GetPositionPtr() { return glm::value_ptr(m_CameraPosition); }
		float* GetMovemetSpeedPtr() { return &m_MovementSpeed; }
		float* GetFovPtr() { return &m_Fov; }

	private:
		void UpdateCameraVectors();
	};

}
