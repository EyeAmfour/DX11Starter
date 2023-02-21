#include "Camera.h"
#include "Input.h"

Camera::Camera(
	float x, float y, float z, 
	float moveSpeed, 
	float rotationSpeed, 
	float fieldOfView, 
	float aspectRatio) : 
	moveSpeed(moveSpeed),
	rotationSpeed(rotationSpeed),
	fieldOfView(fieldOfView) {

	transform = std::make_shared<Transform>();

	transform->SetPosition(x, y, z);
	UpdateViewMatrix();
	UpdateProjectionMatrix(aspectRatio);
}

// --------------------------------------------------------
// Get the View Matrix
// --------------------------------------------------------
DirectX::XMFLOAT4X4 Camera::GetView() {
	return viewMatrix;
}

// --------------------------------------------------------
// Get the Projection Matrix
// --------------------------------------------------------
DirectX::XMFLOAT4X4 Camera::GetProjection() {
	return projectionMatrix;
}

// --------------------------------------------------------
// Get the Transform smart pointer
// --------------------------------------------------------
std::shared_ptr<Transform> Camera::GetTransform() {
	return transform;
}

// --------------------------------------------------------
// Get the move speed
// --------------------------------------------------------
float Camera::GetMoveSpeed() {
	return moveSpeed;
}

// --------------------------------------------------------
// Get the rotation speed
// --------------------------------------------------------
float Camera::GetRotationSpeed() {
	return rotationSpeed;
}

// --------------------------------------------------------
// Get the field of view
// --------------------------------------------------------
float Camera::GetFieldOfView() {
	return fieldOfView;
}

// --------------------------------------------------------
// Update the camera
// --------------------------------------------------------
void Camera::Update(float dt) {
	//Get keyboard and mouse input
	Input& input = Input::GetInstance();

	float finalSpeed = moveSpeed * dt;

	//Move forward
	if (input.KeyDown('W')) {
		transform->MoveRelative(0.0f, 0.0f, finalSpeed);
	}

	//Move backwards
	if (input.KeyDown('S')) {
		transform->MoveRelative(0.0f, 0.0f, -finalSpeed);
	}

	//Move left
	if (input.KeyDown('A')) {
		transform->MoveRelative(-finalSpeed, 0.0f, 0.0f);
	}

	//Move right
	if (input.KeyDown('D')) {
		transform->MoveRelative(finalSpeed, 0.0f, 0.0f);
	}

	//Move up
	if (input.KeyDown(VK_SPACE)) {
		transform->MoveRelative(0.0f, finalSpeed, 0.0f);
	}

	//Move down
	if (input.KeyDown(VK_SHIFT)) {
		transform->MoveRelative(0.0f, -finalSpeed, 0.0f);
	}

	//Check for mouse movement when dragging
	if (input.MouseLeftDown()) {
		float xDiff = input.GetMouseXDelta() * rotationSpeed;
		float yDiff = input.GetMouseYDelta() * rotationSpeed;
		transform->Rotate(yDiff, xDiff, 0.0f);
	}

	//Update the view matrix
	UpdateViewMatrix();

}

// --------------------------------------------------------
// Updates the View Matrix
// --------------------------------------------------------
void Camera::UpdateViewMatrix() {
	//Grab the position from the transform
	DirectX::XMFLOAT3 pos = transform->GetPosition();
	DirectX::XMFLOAT3 forward = transform->GetForward();

	DirectX::XMVECTOR posVector = DirectX::XMLoadFloat3(&pos);
	DirectX::XMVECTOR forwardVector = DirectX::XMLoadFloat3(&forward);

	DirectX::XMMATRIX view = DirectX::XMMatrixLookToLH(
		posVector, //Where you are
		forwardVector, //Which way you are looking
		DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)); //Which way is up

	DirectX::XMStoreFloat4x4(&viewMatrix, view);
}

// --------------------------------------------------------
// Updates the Projection Matrix
// --------------------------------------------------------
void Camera::UpdateProjectionMatrix(float aspectRatio) {
	DirectX::XMMATRIX proj = DirectX::XMMatrixPerspectiveFovLH(
		fieldOfView,
		aspectRatio,
		0.01f, //near plane
		1000.0f); //far plane

	DirectX::XMStoreFloat4x4(&projectionMatrix, proj);

}

// --------------------------------------------------------
// Sets the move speed
// --------------------------------------------------------
void Camera::SetMoveSpeed(float moveSpeed) {
	this->moveSpeed = moveSpeed;
}

// --------------------------------------------------------
// Sets the rotation speed
// --------------------------------------------------------
void Camera::SetRotationSpeed(float rotationSpeed) {
	this->rotationSpeed = rotationSpeed;
}
