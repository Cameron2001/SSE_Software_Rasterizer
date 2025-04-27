#pragma once
#include "glm/vec3.hpp"
#include "glm/mat4x4.hpp"

class Camera
{
public:
	explicit Camera(const glm::vec3& position = glm::vec3(0.0f, 0.0f, 0.0f),
	                glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
	                float yaw = -90.0f,
	                float pitch = 0.0f,
	                float fov = 90.0f,
	                float aspectRatio = 16.0f / 9.0f,
	                float nearPlane = 0.1f,
	                float farPlane = 100.0f);

	void setPosition(const glm::vec3& position);
	void setDirection(float yaw, float pitch);
	void setFov(float fov);
	void setProjectionParams(float aspectRatio, float nearPlane, float farPlane);

	const glm::mat4& getViewMatrix() const { return mViewMatrix; }
	const glm::mat4& getProjectionMatrix() const { return mProjectionMatrix; }
	const glm::mat4& getViewProjectionMatrix() const { return mViewProjectionMatrix; }

	const glm::vec3& getPosition() const { return mPosition; }
	const glm::vec3& getFront() const { return mFront; }
	const glm::vec3& getUp() const { return mUp; }
	const glm::vec3& getRight() const { return mRight; }

	float getYaw() const { return mYaw; }
	float getPitch() const { return mPitch; }
	float getFov() const { return mFov; }

private:
	void updateViewMatrix();
	void updateProjectionMatrix();

	glm::vec3 mPosition;
	glm::vec3 mFront;
	glm::vec3 mUp;
	glm::vec3 mRight;
	glm::vec3 mWorldUp;

	// euler angles in degrees
	float mYaw;
	float mPitch;
	float mFov;

	float mAspectRatio;
	float mNearPlane;
	float mFarPlane;

	glm::mat4 mViewMatrix;
	glm::mat4 mProjectionMatrix;
	glm::mat4 mViewProjectionMatrix;
};
