#pragma once

#include "DXCore.h"
#include <DirectXMath.h>
#include <wrl/client.h> // Used for ComPtr - a smart pointer for COM objects

#include <vector>
#include <memory>

#include "Entity.h"
#include "Mesh.h"
#include "Camera.h"

#include "SimpleShader.h"
#include "Material.h"

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
	void CreateGeometry();
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
	std::shared_ptr<SimpleVertexShader> vertexShader;

	//Mesh Assignment variables
	std::vector<std::shared_ptr<Entity>> entities;
	std::vector<std::shared_ptr<Mesh>> meshes;

	//Camera field
	std::vector<std::shared_ptr<Camera>> cameras;
	int selectedCameraIndex;

	//Material field
	std::vector<std::shared_ptr<Material>> materials;
};

