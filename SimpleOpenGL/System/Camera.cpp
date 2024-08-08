#include "Camera.h"
#include "AppSettings.h"

// Set GLM_FORCE_DEPTH_ZERO_TO_ONE in the preprocessor
#include "glm/gtc/matrix_transform.hpp"

#include <iostream>

Camera::Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch) : 
	Position(position),
	Front(glm::vec3(0.0f, 0.0f, -1.0f)),
	Up(glm::vec3(0.f, 1.f, 0.f)),
	Right(glm::vec3(1.f, 0.f, 0.f)),
	WorldUp(up),
	Yaw(yaw),
	Pitch(pitch),
	MovementSpeed(CAMERA_SPEED),
	MouseSensitivity(CAMERA_SENSITIVITY),
	Zoom(CAMERA_ZOOM),
	projectionMatrix(glm::mat4(1.f)),
	viewMatrix(glm::mat4(1.f))
{
	UpdateInternal();
}

Camera::Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch) : 
	Position(glm::vec3(posX, posY, posZ)),
	Front(glm::vec3(0.0f, 0.0f, -1.0f)), 
	Up(glm::vec3(0.f, 1.f, 0.f)),
	Right(glm::vec3(1.f, 0.f, 0.f)),
	WorldUp(glm::vec3(upX, upY, upZ)),
	Yaw(yaw),
	Pitch(pitch),
	MovementSpeed(CAMERA_SPEED), 
	MouseSensitivity(CAMERA_SENSITIVITY),
	Zoom(CAMERA_ZOOM),
	projectionMatrix(glm::mat4(1.f)),
	viewMatrix(glm::mat4(1.f))
{
	UpdateInternal();
}

glm::mat4 Camera::GetProjectionMatrix() const
{
	return projectionMatrix;
}

glm::mat4 Camera::GetViewMatrix() const
{
	return viewMatrix;
}

void Camera::ProcessKeyboard(CameraMovement direction, float deltaTime)
{
	const float velocity = MovementSpeed * deltaTime;
	if (direction == CameraForward)
	{
		Position += Front * velocity;
	}
	else if (direction == CameraBackward)
	{
		Position -= Front * velocity;
	}
	else if (direction == CameraLeft)
	{
		Position -= Right * velocity;
	}
	else if (direction == CameraRight)
	{
		Position += Right * velocity;
	}

	UpdateInternal();
}

// Processes input received from a mouse input system. Expects the offset value in both the x and y direction.
void Camera::ProcessMouseMovement(float xOffset, float yOffset, bool constrainPitch)
{
	xOffset *= MouseSensitivity;
	yOffset *= MouseSensitivity;

	Yaw += xOffset;
	Pitch += yOffset;

	// Make sure that when pitch is out of bounds, screen doesn't get flipped
	if (constrainPitch)
	{
		if (Pitch > 89.0f)
		{
			Pitch = 89.0f;
		}
		else if (Pitch < -89.0f)
		{
			Pitch = -89.0f;
		}
	}

	// Update vectors and matrices
	UpdateInternal();
}

// Processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
void Camera::ProcessMouseScroll(float yOffset)
{
	Zoom -= yOffset;
	
	if (Zoom < 1.0f)
	{
		Zoom = 1.0f;
	}
	else if (Zoom > 45.0f)
	{
		Zoom = 45.0f;
	}

	UpdateInternal();
}

// Update vectors and matrices
void Camera::UpdateInternal()
{
	// Recalculate front vector
	Front = glm::normalize(
		glm::vec3
		(
			cos(glm::radians(Yaw)) * cos(glm::radians(Pitch)), // x
			sin(glm::radians(Pitch)), // y
			sin(glm::radians(Yaw)) * cos(glm::radians(Pitch)) // z
		)
	);
	
	// Recalculate right and up vector
	Right = glm::normalize(glm::cross(Front, WorldUp));  // Normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
	Up = glm::normalize(glm::cross(Right, Front));

	// Matrices
	projectionMatrix = glm::perspective(glm::radians(Zoom), 
		static_cast<float>(AppSettings::ScreenWidth) / static_cast<float>(AppSettings::ScreenHeight),
		NEAR_PLANE,
		FAR_PLANE);
	viewMatrix = glm::lookAt(Position, Position + Front, Up);
}