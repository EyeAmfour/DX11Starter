#pragma once

#include "DXCore.h"
#include <DirectXMath.h>
#include <wrl/client.h> // Used for ComPtr - a smart pointer for COM objects

#include <vector>
#include <memory>

#include "Entity.h"
#include "Mesh.h"
#include "Camera.h"
#include "Sky.h"

#include "SimpleShader.h"
#include "Material.h"

#include "Lights.h"

class Game 
	: public DXCore
{

public:
	Game(HINSTANCE hInstance);
	~Game();

	// Overridden setup and game loop methods, which
	// will be called automatically
	void Init();
	void OnResize();
	void Update(float deltaTime, float totalTime);
	void Draw(float deltaTime, float totalTime);

private:

	// Initialization helper methods - feel free to customize, combine, remove, etc.
	void LoadShaders(); 
	void CreateMaterials();
	void CreateSky();
	void CreateLights();
	void CreateGeometry();
	void CreateShadowMap();
	void UpdateGui(float deltaTime);

	//ImGui Window Creation Methods
	void CreateWindowInfoGui();
	void CreateInspectorGui();

	// Note the usage of ComPtr below
	//  - This is a smart pointer for objects that abide by the
	//     Component Object Model, which DirectX objects do
	//  - More info here: https://github.com/Microsoft/DirectXTK/wiki/ComPtr

	// Buffers to hold actual geometry data
	Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;
	
	// Shaders and shader-related constructs
	std::vector<std::shared_ptr<SimplePixelShader>> pixelShaders;
	std::vector<std::shared_ptr<SimpleVertexShader>> vertexShaders;

	//Mesh Assignment variables
	std::vector<std::shared_ptr<Entity>> entities;
	std::vector<std::shared_ptr<Mesh>> meshes;

	//Camera field
	std::vector<std::shared_ptr<Camera>> cameras;
	int selectedCameraIndex;

	//Material field
	std::vector<std::shared_ptr<Material>> materials;

	//Light fields
	DirectX::XMFLOAT3 ambientColor;
	std::vector<Light*> lights;
	Light directionalLight1;
	Light directionalLight2;
	Light directionalLight3;
	Light pointLight1;
	Light pointLight2;

	//Sky fields
	std::shared_ptr<Sky> sky;

	//Shadow fields
	int shadowMapResolution;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> shadowDSV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shadowSRV;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> shadowRasterizer;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> shadowSampler;
	DirectX::XMFLOAT4X4 lightViewMatrix;
	DirectX::XMFLOAT4X4 lightProjectionMatrix;
};

