#ifndef CAMERA
#define CAMERA

#include "glm/glm.hpp"

enum CameraMovement
{
	CameraForward,
	CameraBackward,
	CameraLeft,
	CameraRight,
};

// Default camera values
constexpr float CAMERA_YAW = -90.0f;
constexpr float CAMERA_PITCH = 0.0f;
constexpr float CAMERA_SPEED = 2.5f;
constexpr float CAMERA_SENSITIVITY = 0.1f;
constexpr float CAMERA_ZOOM = 45.0f;
constexpr float NEAR_PLANE = 0.1f;
constexpr float FAR_PLANE = 100.0f;

class Camera
{
public:
	// Attributes
	glm::vec3 Position{};
	glm::vec3 Front{};
	glm::vec3 Up{};
	glm::vec3 Right{};
	glm::vec3 WorldUp{};

	// Euler Angles
	float Yaw{};
	float Pitch{};

	// Options
	float MovementSpeed{};
	float MouseSensitivity{};
	float Zoom{};

private:
	glm::mat4 projectionMatrix{};
	glm::mat4 viewMatrix{};

public:
	Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), 
		glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), 
		float yaw = CAMERA_YAW,
		float pitch = CAMERA_PITCH);

	Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch);

	[[nodiscard]] glm::mat4 GetProjectionMatrix() const;
	[[nodiscard]] glm::mat4 GetViewMatrix() const;

	void ProcessKeyboard(CameraMovement direction, float deltaTime);
	void ProcessMouseMovement(float xOffset, float yOffset, bool constrainPitch = true);
	void ProcessMouseScroll(float yOffset);
	
private:
	void UpdateInternal();
};

#endif
