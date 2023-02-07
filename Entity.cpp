#include "Entity.h"

#include <iostream>

Entity::Entity(std::shared_ptr<Mesh> mesh) : mesh(mesh) {
    transform = std::make_shared<Transform>();
}

Entity::~Entity() {
}

// --------------------------------------------------------
// Update the Constant Buffer
// --------------------------------------------------------
void Entity::UpdateConstantBuffer(
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> context,
	Microsoft::WRL::ComPtr<ID3D11Buffer> vsConstantBuffer,
	DirectX::XMFLOAT4 tint,
	DirectX::XMFLOAT4X4 world) {

	//Create local instance of struct to hold data
	VertexShaderExternalData vsData;
	//vsData.colorTint = XMFLOAT4(0.1f, 0.9f, 0.3f, 1.0f);
	//vsData.offset = XMFLOAT3(0.15f, -0.3f, 0.0f);
	vsData.colorTint = tint;
	vsData.worldMatrix = world;

	//Write data to constant buffer
	D3D11_MAPPED_SUBRESOURCE mappedBuffer = {};

	//Lock buffer
	context->Map(vsConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedBuffer);

	//Copy data over
	memcpy(mappedBuffer.pData, &vsData, sizeof(vsData));

	//Unlock buffer
	context->Unmap(vsConstantBuffer.Get(), 0);

	//Bind the constant buffer
	context->VSSetConstantBuffers(
		0, // Which slot (register) to bind the buffer to?
		1, // How many are we activating? Can do multiple at once
		vsConstantBuffer.GetAddressOf()); // Array of buffers (or the address of one)
}

std::shared_ptr<Mesh> Entity::GetMesh() {
    return mesh;
}

std::shared_ptr<Transform> Entity::GetTransform() {
    return transform;
}

void Entity::Draw(
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> context,
	Microsoft::WRL::ComPtr<ID3D11Buffer> vsConstantBuffer) {

	UpdateConstantBuffer(context, vsConstantBuffer, mesh->meshTint, transform->GetWorldMatrix());
    mesh->Draw();
}
