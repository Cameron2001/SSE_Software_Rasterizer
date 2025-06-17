#include "Camera.h"

#include <algorithm>
#include <stdexcept>
#include <iostream>
#include <cassert>
#include <cmath>

#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"

Camera::Camera(const glm::vec3& position, const glm::vec3 up, float yaw, float pitch, float fov,
               float aspectRatio, float nearPlane, float farPlane)
	: mPosition(position), mUp(up), mWorldUp(0.0f, 1.0f, 0.0f), mYaw(yaw), mPitch(pitch), mFov(fov),
	  mAspectRatio(aspectRatio), mNearPlane(nearPlane), mFarPlane(farPlane)
{
	if (fov <= 0.0f || fov >= 180.0f)
	{
		throw std::invalid_argument("Field of view must be between 0 and 180 degrees");
	}
	if (aspectRatio <= 0.0f)
	{
		throw std::invalid_argument("Aspect ratio must be positive");
	}
	if (nearPlane <= 0.0f)
	{
		throw std::invalid_argument("Near plane must be positive");
	}
	if (farPlane <= nearPlane)
	{
		throw std::invalid_argument("Far plane must be greater than near plane");
	}

	updateProjectionMatrix();
	updateViewMatrix();
}

void Camera::updateViewMatrix()
{
	// clamp to avoid gimbal lock
	mPitch = std::clamp(mPitch, -89.0f, 89.0f);

	// calculate front vector from yaw and pitch
	glm::vec3 front;
	front.x = cos(glm::radians(mYaw)) * cos(glm::radians(mPitch));
	front.y = sin(glm::radians(mPitch));
	front.z = sin(glm::radians(mYaw)) * cos(glm::radians(mPitch));

	assert(glm::length(front) > 1e-6f && "Degenerate front vector in camera");

	mFront = glm::normalize(front);

	// also re-calculate the Right and Up vector
	mRight = glm::normalize(glm::cross(mFront, mWorldUp));
	mUp = glm::normalize(glm::cross(mRight, mFront));

	mViewMatrix = glm::lookAt(mPosition, mPosition + mFront, mUp);
	mViewProjectionMatrix = mProjectionMatrix * mViewMatrix;
}

void Camera::updateProjectionMatrix()
{
	mProjectionMatrix = glm::perspective(glm::radians(mFov), mAspectRatio, mNearPlane, mFarPlane);
	mViewProjectionMatrix = mProjectionMatrix * mViewMatrix;
}

void Camera::setPosition(const glm::vec3& position)
{
	if (!std::isfinite(position.x) || !std::isfinite(position.y) || !std::isfinite(position.z))
	{
		throw std::invalid_argument("Camera position contains NaN or infinite values");
	}

	mPosition = position;
	updateViewMatrix();
}

void Camera::setDirection(const float yaw, const float pitch)
{
	if (pitch < -90.0f || pitch > 90.0f)
	{
		std::cerr << "Warning: Pitch value " << pitch << " is outside normal range [-90, 90]\n";
	}

	mYaw = yaw;
	mPitch = pitch;
	updateViewMatrix();
}

void Camera::setFov(const float fov)
{
	if (fov <= 0.0f || fov >= 180.0f)
	{
		throw std::invalid_argument("Field of view must be between 0 and 180 degrees");
	}
	mFov = fov;
	updateProjectionMatrix();
}

void Camera::setProjectionParams(const float aspectRatio, const float nearPlane, const float farPlane)
{
	if (aspectRatio <= 0.0f)
	{
		throw std::invalid_argument("Aspect ratio must be positive");
	}
	if (nearPlane <= 0.0f)
	{
		throw std::invalid_argument("Near plane must be positive");
	}
	if (farPlane <= nearPlane)
	{
		throw std::invalid_argument("Far plane must be greater than near plane");
	}

	mAspectRatio = aspectRatio;
	mNearPlane = nearPlane;
	mFarPlane = farPlane;
	updateProjectionMatrix();
}
