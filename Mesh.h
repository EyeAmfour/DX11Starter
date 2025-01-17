#pragma once

#include <d3d11.h>
#include <wrl/client.h>

#include "Vertex.h"

class Mesh {
private:
	//Pointers for vertex buffer and index buffer
	Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;

	//Pointer for draw commands
	Microsoft::WRL::ComPtr<ID3D11DeviceContext>	context;

	//Number of indices in the index buffer
	int numIndices;


	void CreateVertexBuffer(Vertex* vertices, int numVerts, Microsoft::WRL::ComPtr<ID3D11Device> device);
	void CreateIndexBuffer(unsigned int* indices, int numIndices, Microsoft::WRL::ComPtr<ID3D11Device> device);
	void CalculateTangents(Vertex* vertices, int numVerts, unsigned int* indices, int numIndices);

public:
	//Constructor
	Mesh(
		Vertex* vertices,
		int numVerts,
		unsigned int* indices,
		int numIndices,
		Microsoft::WRL::ComPtr<ID3D11Device> device,
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> context
	);

	Mesh(const wchar_t* fileToLoad, Microsoft::WRL::ComPtr<ID3D11Device> device, Microsoft::WRL::ComPtr<ID3D11DeviceContext> context);

	//Destructor
	~Mesh();

	//Gets the Vertex Buffer
	Microsoft::WRL::ComPtr<ID3D11Buffer> GetVertexBuffer();

	//Gets the Index Buffer
	Microsoft::WRL::ComPtr<ID3D11Buffer> GetIndexBuffer();

	//Mesh Property Variables
	DirectX::XMFLOAT4 meshTint;
	DirectX::XMFLOAT4X4 meshWorldMatrix;

	//Gets the number of indicies in the index buffer
	int GetIndexCount();

	//Draws the mesh
	void Draw();
};

