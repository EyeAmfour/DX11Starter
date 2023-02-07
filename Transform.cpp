#include "Transform.h"

void Transform::UpdateMatrices() {
    //Create matrices for translation, scale, and rotation
    DirectX::XMMATRIX translation = DirectX::XMMatrixTranslation(
        this->position.x,
        this->position.y,
        this->position.z);

    DirectX::XMMATRIX scale = DirectX::XMMatrixScaling(
        this->scale.x,
        this->scale.y,
        this->scale.z);

    DirectX::XMMATRIX rotation = DirectX::XMMatrixRotationRollPitchYaw(
        this->rotation.x,
        this->rotation.y,
        this->rotation.z);

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

void Transform::SetPosition(float x, float y, float z) {
    position = DirectX::XMFLOAT3(x, y, z);
}

void Transform::SetPosition(DirectX::XMFLOAT3 position) {
    this->position = DirectX::XMFLOAT3(position);
}

void Transform::SetRotation(float pitch, float yaw, float roll) {
    rotation = DirectX::XMFLOAT3(pitch, yaw, roll);
}

void Transform::SetRotation(DirectX::XMFLOAT3 rotation) {
    this->rotation = DirectX::XMFLOAT3(rotation);
}

void Transform::SetScale(float x, float y, float z) {
    scale = DirectX::XMFLOAT3(x, y, z);
}

void Transform::SetScale(DirectX::XMFLOAT3 scale) {
    this->scale = DirectX::XMFLOAT3(scale);
}

DirectX::XMFLOAT3 Transform::GetPosition() {
    return position;
}

DirectX::XMFLOAT3 Transform::GetPitchYawRoll() {
    return rotation;
}

DirectX::XMFLOAT3 Transform::GetScale() {
    return scale;
}

DirectX::XMFLOAT4X4 Transform::GetWorldMatrix() {
    UpdateMatrices();
    return world;
}

DirectX::XMFLOAT4X4 Transform::GetWorldInverseTransposeMatrix() {
    UpdateMatrices();
    return worldInverseTranspose;
}

void Transform::MoveAbsolute(float x, float y, float z) {
    MoveAbsolute(DirectX::XMFLOAT3(x, y, z));
}

void Transform::MoveAbsolute(DirectX::XMFLOAT3 offset) {
    position = DirectX::XMFLOAT3(
        position.x + offset.x,
        position.y + offset.y,
        position.z + offset.z
    );
}

void Transform::Rotate(float pitch, float yaw, float roll) {
    Rotate(DirectX::XMFLOAT3(pitch, yaw, roll));
}

void Transform::Rotate(DirectX::XMFLOAT3 rotation) {
    this->rotation = DirectX::XMFLOAT3(
        this->rotation.x + rotation.x,
        this->rotation.y + rotation.y,
        this->rotation.z + rotation.z
    );
}

void Transform::Scale(float x, float y, float z) {
    Scale(DirectX::XMFLOAT3(x, y, z));
}

void Transform::Scale(DirectX::XMFLOAT3 scale) {
    this->scale = DirectX::XMFLOAT3(
        this->scale.x * scale.x,
        this->scale.y * scale.y,
        this->scale.z * scale.z
    );
}
