#include "Camera.h"

#include <algorithm>

#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"

Camera::Camera(const glm::vec3& position, const glm::vec3 up, float yaw, float pitch, float fov,
               float aspectRatio, float nearPlane, float farPlane)
	: mPosition(position), mUp(up), mWorldUp(0.0f, 1.0f, 0.0f), mYaw(yaw), mPitch(pitch), mFov(fov),
	  mAspectRatio(aspectRatio), mNearPlane(nearPlane), mFarPlane(farPlane)
{
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
	mFront = glm::normalize(front);

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
	mPosition = position;
	updateViewMatrix();
}

void Camera::setDirection(const float yaw, const float pitch)
{
	mYaw = yaw;
	mPitch = pitch;
	updateViewMatrix();
}

void Camera::setFov(const float fov)
{
	mFov = fov;
	updateProjectionMatrix();
}

void Camera::setProjectionParams(const float aspectRatio, const float nearPlane, const float farPlane)
{
	mAspectRatio = aspectRatio;
	mNearPlane = nearPlane;
	mFarPlane = farPlane;
	updateProjectionMatrix();
}
