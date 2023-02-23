#pragma once

#include <DirectXMath.h>
#include <memory>

#include "SimpleShader.h"

class Material {
private:
	//Fields
	DirectX::XMFLOAT4 colorTint;
	std::shared_ptr<SimpleVertexShader> vertexShader;
	std::shared_ptr<SimplePixelShader> pixelShader;

public:
	//Constructor/Destructor
	Material(DirectX::XMFLOAT4 tint, std::shared_ptr<SimpleVertexShader> vs, std::shared_ptr<SimplePixelShader> ps);
	~Material();

	//Getters
	DirectX::XMFLOAT4 GetColorTint();
	std::shared_ptr<SimpleVertexShader> GetVertexShader();
	std::shared_ptr<SimplePixelShader> GetPixelShader();

	//Setters
	void SetColorTint(DirectX::XMFLOAT4 tint);
	void SetVertexShader(std::shared_ptr<SimpleVertexShader> vs);
	void SetPixelShader(std::shared_ptr<SimplePixelShader> ps);
};

