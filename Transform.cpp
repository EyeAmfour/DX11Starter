#include "Transform.h"
// --------------------------------------------------------
// Updates the world and world inverse transpose matrices
// --------------------------------------------------------
void Transform::UpdateMatrices() {
    //Create matrices for translation, scale, and rotation
    DirectX::XMMATRIX translation = DirectX::XMMatrixTranslationFromVector(DirectX::XMLoadFloat3(&position));

    DirectX::XMMATRIX rotation = DirectX::XMMatrixRotationRollPitchYawFromVector(DirectX::XMLoadFloat3(&this->rotation));
        
    DirectX::XMMATRIX scale = DirectX::XMMatrixScalingFromVector(DirectX::XMLoadFloat3(&this->scale));

    //Apply the transformation matrices
    DirectX::XMMATRIX world = scale * rotation * translation;

    //Sore the final matrix and its inverse transpose as a 4x4 float
    DirectX::XMStoreFloat4x4(&this->world, world);
    DirectX::XMStoreFloat4x4(&this->worldInverseTranspose,
        DirectX::XMMatrixInverse(0, DirectX::XMMatrixTranspose(world)));
}

Transform::Transform() {
    position = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
    scale = DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f);
    rotation = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);

    right = DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f);
    up = DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f);
    forward = DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f);

    DirectX::XMStoreFloat4x4(&world, DirectX::XMMatrixIdentity());
    DirectX::XMStoreFloat4x4(&worldInverseTranspose, DirectX::XMMatrixIdentity());
}

Transform::~Transform() {
}

// --------------------------------------------------------
// Sets the position using the provided x, y, and z params
// --------------------------------------------------------
void Transform::SetPosition(float x, float y, float z) {
    position = DirectX::XMFLOAT3(x, y, z);
}

// --------------------------------------------------------
// Sets the position using the provided position
// --------------------------------------------------------
void Transform::SetPosition(DirectX::XMFLOAT3 position) {
    this->position = DirectX::XMFLOAT3(position);
}

// -----------------------------------------------------------------
// Sets the rotation using the provided pitch, yaw, and roll params
// -----------------------------------------------------------------
void Transform::SetRotation(float pitch, float yaw, float roll) {
    rotation = DirectX::XMFLOAT3(pitch, yaw, roll);
}

// --------------------------------------------------------
// Sets the rotation using the provided rotation
// --------------------------------------------------------
void Transform::SetRotation(DirectX::XMFLOAT3 rotation) {
    this->rotation = DirectX::XMFLOAT3(rotation);
}

// --------------------------------------------------------
// Sets the scale using the provided x, y, and z params
// --------------------------------------------------------
void Transform::SetScale(float x, float y, float z) {
    scale = DirectX::XMFLOAT3(x, y, z);
}

// --------------------------------------------------------
// Sets the scale using the provided scale
// --------------------------------------------------------
void Transform::SetScale(DirectX::XMFLOAT3 scale) {
    this->scale = DirectX::XMFLOAT3(scale);
}

// --------------------------------------------------------
// Gets the position
// --------------------------------------------------------
DirectX::XMFLOAT3 Transform::GetPosition() {
    return position;
}

// --------------------------------------------------------
// Gets the rotation
// --------------------------------------------------------
DirectX::XMFLOAT3 Transform::GetPitchYawRoll() {
    return rotation;
}

// --------------------------------------------------------
// Gets the scale
// --------------------------------------------------------
DirectX::XMFLOAT3 Transform::GetScale() {
    return scale;
}

// --------------------------------------------------------
// Updates and gets the world matrix
// --------------------------------------------------------
DirectX::XMFLOAT4X4 Transform::GetWorldMatrix() {
    UpdateMatrices();
    return world;
}

// --------------------------------------------------------
// Updates and gets the world inverse transpose matrix
// --------------------------------------------------------
DirectX::XMFLOAT4X4 Transform::GetWorldInverseTransposeMatrix() {
    UpdateMatrices();
    return worldInverseTranspose;
}

DirectX::XMFLOAT3 Transform::GetRight() {
    return right;
}

DirectX::XMFLOAT3 Transform::GetUp() {
    return up;
}

DirectX::XMFLOAT3 Transform::GetForward() {
    return forward;
}

// -------------------------------------------------------------
// Translates the position with the provided x, y, and z params
// -------------------------------------------------------------
void Transform::MoveAbsolute(float x, float y, float z) {
    MoveAbsolute(DirectX::XMFLOAT3(x, y, z));
}

// --------------------------------------------------------
// Translates the position with the provided offset
// --------------------------------------------------------
void Transform::MoveAbsolute(DirectX::XMFLOAT3 offset) {
    DirectX::XMStoreFloat3(
        &this->position,
        DirectX::XMVectorAdd(
            DirectX::XMLoadFloat3(&this->position),
            DirectX::XMLoadFloat3(&offset)
        )
    );
}

void Transform::MoveRelative(float x, float y, float z) {
    DirectX::XMFLOAT3 input(x, y, z);
    DirectX::XMVECTOR direction = DirectX::XMLoadFloat3(&input);
    DirectX::XMVECTOR quaternion = DirectX::XMQuaternionRotationRollPitchYaw(rotation.x, rotation.y, rotation.z);
    DirectX::XMVECTOR rotated = DirectX::XMVector3Rotate(direction, quaternion);

    DirectX::XMVECTOR pos = DirectX::XMLoadFloat3(&position);
    DirectX::XMStoreFloat3(&position, DirectX::XMVectorAdd(pos, rotated));
}

// --------------------------------------------------------
// Rotates with the provided pitch, yaw, and roll params
// --------------------------------------------------------
void Transform::Rotate(float pitch, float yaw, float roll) {
    Rotate(DirectX::XMFLOAT3(pitch, yaw, roll));
}

// --------------------------------------------------------
// Rotates with the provided rotation
// --------------------------------------------------------
void Transform::Rotate(DirectX::XMFLOAT3 rotation) {
    //Update the rotation
    DirectX::XMStoreFloat3(
        &this->rotation, 
        DirectX::XMVectorAdd(
            DirectX::XMLoadFloat3(&this->rotation), 
            DirectX::XMLoadFloat3(&rotation)
        )
    );

    //Update right, up, and forward relative to transform
    DirectX::XMVECTOR worldRight = DirectX::XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
    DirectX::XMVECTOR worldUp = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    DirectX::XMVECTOR worldForward = DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);

    DirectX::XMVECTOR quaternion = DirectX::XMQuaternionRotationRollPitchYaw(this->rotation.x, this->rotation.y, this->rotation.z);

    DirectX::XMStoreFloat3(&this->right, DirectX::XMVector3Rotate(worldRight, quaternion));
    DirectX::XMStoreFloat3(&this->up, DirectX::XMVector3Rotate(worldUp, quaternion));
    DirectX::XMStoreFloat3(&this->forward, DirectX::XMVector3Rotate(worldForward, quaternion));
}

// --------------------------------------------------------
// Scales with the provided x, y, and z params
// --------------------------------------------------------
void Transform::Scale(float x, float y, float z) {
    Scale(DirectX::XMFLOAT3(x, y, z));
}

// --------------------------------------------------------
// Scales with the provided scale
// --------------------------------------------------------
void Transform::Scale(DirectX::XMFLOAT3 scale) {
    DirectX::XMStoreFloat3(
        &this->scale,
        DirectX::XMVectorMultiply(
            DirectX::XMLoadFloat3(&this->scale),
            DirectX::XMLoadFloat3(&scale)
        )
    );
}
