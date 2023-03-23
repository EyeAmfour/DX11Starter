#pragma once

#include "DXCore.h"
#include <DirectXMath.h>
#include <wrl/client.h> // Used for ComPtr - a smart pointer for COM objects

#include <vector>
#include <memory>

#include "Mesh.h"
#include "Camera.h"
#include "SimpleShader.h"

class Sky {
private:
	Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerOptions;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> textureSRV;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depthStencilState;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizerState;

	std::shared_ptr<Mesh> skyMesh;
	std::shared_ptr<SimpleVertexShader> vertexShader;
	std::shared_ptr<SimplePixelShader> pixelShader;

	Microsoft::WRL::ComPtr<ID3D11Device> device;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext>	context;

	// Helper for creating a cubemap from 6 individual textures
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> CreateCubemap(
		const wchar_t* right,
		const wchar_t* left,
		const wchar_t* up,
		const wchar_t* down,
		const wchar_t* front,
		const wchar_t* back);

public:
	Sky(
		std::shared_ptr<Mesh> mesh,
		Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerState,
		Microsoft::WRL::ComPtr<ID3D11Device> device,
		Microsoft::WRL::ComPtr<ID3D11DeviceContext>	context,
		std::shared_ptr<SimpleVertexShader> vs,
		std::shared_ptr<SimplePixelShader> ps,
		std::vector<std::wstring> filesToLoad
	);
	~Sky();

	void Draw(std::shared_ptr<Camera> camera);
};

