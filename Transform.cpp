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
    position = DirectX::XMFLOAT3(
        position.x + offset.x,
        position.y + offset.y,
        position.z + offset.z
    );
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
    this->rotation = DirectX::XMFLOAT3(
        this->rotation.x + rotation.x,
        this->rotation.y + rotation.y,
        this->rotation.z + rotation.z
    );
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
    this->scale = DirectX::XMFLOAT3(
        this->scale.x * scale.x,
        this->scale.y * scale.y,
        this->scale.z * scale.z
    );
}
