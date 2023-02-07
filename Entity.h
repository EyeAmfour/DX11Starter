#pragma once

#include <memory>
#include <wrl/client.h> // Used for ComPtr - a smart pointer for COM objects

#include "Transform.h"
#include "Mesh.h"
#include "BufferStructs.h"

class Entity {
private:
	std::shared_ptr<Mesh> mesh;
	std::shared_ptr<Transform> transform;

	//Constant Buffer Helper
	void UpdateConstantBuffer(
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> context,
		Microsoft::WRL::ComPtr<ID3D11Buffer> vsConstantBuffer,
		DirectX::XMFLOAT4 tint,
		DirectX::XMFLOAT4X4 world
	);

public:
	//Constructor & Destructor
	Entity(std::shared_ptr<Mesh> mesh);
	~Entity();

	//Getters
	std::shared_ptr<Mesh> GetMesh();
	std::shared_ptr<Transform> GetTransform();

	void Draw(
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> context,
		Microsoft::WRL::ComPtr<ID3D11Buffer> vsConstantBuffer
	);
};

