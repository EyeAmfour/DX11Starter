#include "Mesh.h"
#include <iostream>

void Mesh::CreateVertexBuffer(Vertex* vertices, int numVerts, Microsoft::WRL::ComPtr<ID3D11Device> device) {
	// First, we need to describe the buffer we want Direct3D to make on the GPU
	D3D11_BUFFER_DESC vbd = {};
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex) * numVerts;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;

	// Create the proper struct to hold the initial vertex data
	D3D11_SUBRESOURCE_DATA initialVertexData = {};
	initialVertexData.pSysMem = vertices; // pSysMem = Pointer to System Memory

	// Actually create the buffer on the GPU with the initial data
	device->CreateBuffer(&vbd, &initialVertexData, vertexBuffer.GetAddressOf());
}

void Mesh::CreateIndexBuffer(unsigned int* indices, Microsoft::WRL::ComPtr<ID3D11Device> device) {
	// Describe the buffer, as we did above, with two major differences
	D3D11_BUFFER_DESC ibd = {};
	ibd.Usage = D3D11_USAGE_IMMUTABLE;	// Will NEVER change
	ibd.ByteWidth = sizeof(unsigned int) * numIndices;	// 3 = number of indices in the buffer
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;	// Tells Direct3D this is an index buffer
	ibd.CPUAccessFlags = 0;	// Note: We cannot access the data from C++ (this is good)
	ibd.MiscFlags = 0;
	ibd.StructureByteStride = 0;

	// Specify the initial data for this buffer, similar to above
	D3D11_SUBRESOURCE_DATA initialIndexData = {};
	initialIndexData.pSysMem = indices; // pSysMem = Pointer to System Memory

	// Actually create the buffer with the initial data
	device->CreateBuffer(&ibd, &initialIndexData, indexBuffer.GetAddressOf());
}

Mesh::Mesh(
	Vertex* vertices,
	int numVerts,
	unsigned int* indices,
	int numIndices,
	Microsoft::WRL::ComPtr<ID3D11Device> device,
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> context)
	: context(context), numIndices(numIndices) {

	CreateVertexBuffer(vertices, numVerts, device);
	CreateIndexBuffer(indices, device);

	meshTint = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	meshOffset = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
}

Mesh::~Mesh() {
}

/// <summary>
/// Gets the Vertex Buffer
/// </summary>
/// <returns>A CompPtr to the Vertex Buffer</returns>
Microsoft::WRL::ComPtr<ID3D11Buffer> Mesh::GetVertexBuffer() {
	return vertexBuffer;
}

/// <summary>
/// Gets the Index Buffer
/// </summary>
/// <returns>A CompPtr to the Index Buffer</returns>
Microsoft::WRL::ComPtr<ID3D11Buffer> Mesh::GetIndexBuffer() {
	return indexBuffer;
}

/// <summary>
/// Gets the number of indices in the index array
/// </summary>
/// <returns>The number of indices in the index array</returns>
int Mesh::GetIndexCount() {
	return numIndices;
}

/// <summary>
/// Draws the Mesh using the vertex and index buffers
/// </summary>
void Mesh::Draw() {
	// Draw geometries
	UINT stride = sizeof(Vertex);
	UINT offset = 0;

	// Set buffers in the input assembler (IA) stage
	context->IASetVertexBuffers(0, 1, vertexBuffer.GetAddressOf(), &stride, &offset);
	context->IASetIndexBuffer(indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

	// Tell Direct3D to draw
	context->DrawIndexed(numIndices, 0, 0);
}
