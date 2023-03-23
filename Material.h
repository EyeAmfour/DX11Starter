#pragma once

#include <DirectXMath.h>
#include <memory>
#include <wrl/client.h> // Used for ComPtr - a smart pointer for COM objects

#include "SimpleShader.h"

class Material {
private:
	//Fields
	DirectX::XMFLOAT4 colorTint;
	float roughness;
	std::shared_ptr<SimpleVertexShader> vertexShader;
	std::shared_ptr<SimplePixelShader> pixelShader;

	//Texture fields
	std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> textureSRVs;
	std::unordered_map <std::string, Microsoft::WRL::ComPtr<ID3D11SamplerState>> samplerOptions;

public:
	//Constructor/Destructor
	Material(DirectX::XMFLOAT4 tint, float roughness, std::shared_ptr<SimpleVertexShader> vs, std::shared_ptr<SimplePixelShader> ps);
	~Material();

	//Getters
	DirectX::XMFLOAT4 GetColorTint();
	float GetRoughness();
	std::shared_ptr<SimpleVertexShader> GetVertexShader();
	std::shared_ptr<SimplePixelShader> GetPixelShader();

	//Setters
	void SetColorTint(DirectX::XMFLOAT4 tint);
	void SetRoughness(float roughness);
	void SetVertexShader(std::shared_ptr<SimpleVertexShader> vs);
	void SetPixelShader(std::shared_ptr<SimplePixelShader> ps);

	void AddTextureSRV(std::string name, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv);
	void AddSampler(std::string name, Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerState);

	void PrepareMaterial();
};

