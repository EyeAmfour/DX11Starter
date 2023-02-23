#pragma once

#include <memory>
#include <wrl/client.h> // Used for ComPtr - a smart pointer for COM objects

#include "Transform.h"
#include "Mesh.h"
#include "Camera.h"
#include "Material.h"

class Entity {
private:
	std::shared_ptr<Mesh> mesh;
	std::shared_ptr<Transform> transform;
	std::shared_ptr<Material> material;

	//Constant Buffer Helper
	void UpdateConstantBuffer(
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> context,
		std::shared_ptr<Camera> camera,
		float totalTime
	);

public:
	//Constructor & Destructor
	Entity(std::shared_ptr<Mesh> mesh, std::shared_ptr<Material> mat);
	~Entity();

	//Getters
	std::shared_ptr<Mesh> GetMesh();
	std::shared_ptr<Transform> GetTransform();
	std::shared_ptr<Material> GetMaterial();

	//Setters
	void SetMaterial(std::shared_ptr<Material> mat);

	//Draw
	void Draw(
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> context,
		std::shared_ptr<Camera> camera,
		float totalTime
	);
};

