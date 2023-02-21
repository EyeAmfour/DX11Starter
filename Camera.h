#pragma once

#include <DirectXMath.h>
#include <memory>
#include "Transform.h"

class Camera {
private:
	DirectX::XMFLOAT4X4 viewMatrix;
	DirectX::XMFLOAT4X4 projectionMatrix;

	std::shared_ptr<Transform> transform;

	float moveSpeed;
	float rotationSpeed;
	float fieldOfView;

public:
	Camera(
		float x, float y, float z,
		float moveSpeed,
		float rotationSpeed,
		float fieldOfView,
		float aspectRatio);

	DirectX::XMFLOAT4X4 GetView();
	DirectX::XMFLOAT4X4 GetProjection();
	std::shared_ptr<Transform> GetTransform();
	float GetMoveSpeed();
	float GetRotationSpeed();
	float GetFieldOfView();

	void Update(float dt);
	void UpdateViewMatrix();
	void UpdateProjectionMatrix(float aspectRatio);

	void SetMoveSpeed(float moveSpeed);
	void SetRotationSpeed(float rotationSpeed);
};