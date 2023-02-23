#include "Entity.h"

Entity::Entity(std::shared_ptr<Mesh> mesh, std::shared_ptr<Material> mat) : mesh(mesh), material(mat) {
    transform = std::make_shared<Transform>();
}

Entity::~Entity() {
}

// --------------------------------------------------------
// Update the Constant Buffer
// --------------------------------------------------------
void Entity::UpdateConstantBuffer(
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> context,
	std::shared_ptr<Camera> camera,
	float totalTime) {

	//Store data in vertex shader
	std::shared_ptr<SimpleVertexShader> vs = material->GetVertexShader();

	//vs->SetFloat4("colorTint", material->GetColorTint());
	vs->SetMatrix4x4("world", transform->GetWorldMatrix());
	vs->SetMatrix4x4("view", camera->GetView());
	vs->SetMatrix4x4("projection", camera->GetProjection());

	std::shared_ptr<SimplePixelShader> ps = material->GetPixelShader();

	ps->SetFloat4("colorTint", material->GetColorTint());
	ps->SetFloat("time", totalTime);

	//Write data to constant buffer
	vs->CopyAllBufferData();
	ps->CopyAllBufferData();

	//Bind the constant buffer
	//context->VSSetConstantBuffers(
	//	0, // Which slot (register) to bind the buffer to?
	//	1, // How many are we activating? Can do multiple at once
	//	vsConstantBuffer.GetAddressOf()); // Array of buffers (or the address of one)
}

// --------------------------------------------------------
// Gets the smart pointer to the Mesh
// --------------------------------------------------------
std::shared_ptr<Mesh> Entity::GetMesh() {
    return mesh;
}

// --------------------------------------------------------
// Gets the smart pointer to the Transform
// --------------------------------------------------------
std::shared_ptr<Transform> Entity::GetTransform() {
    return transform;
}

std::shared_ptr<Material> Entity::GetMaterial() {
	return material;
}

void Entity::SetMaterial(std::shared_ptr<Material> mat) {
	material = mat;
}

// --------------------------------------------------------
// Updates the constant buffer and then draws the mesh
// --------------------------------------------------------
void Entity::Draw(
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> context,
	std::shared_ptr<Camera> camera,
	float totalTime) {

	material->GetVertexShader()->SetShader();
	material->GetPixelShader()->SetShader();

	//Update the constant buffer, then draw the entity
	UpdateConstantBuffer(
		context,
		camera,
		totalTime
	);

    mesh->Draw();
}
