#include "Game.h"
#include "Vertex.h"
#include "Input.h"
#include "Helpers.h"

//ImGui imports
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_dx11.h"
#include "ImGui/imgui_impl_win32.h"

// Needed for a helper function to load pre-compiled shader files
#pragma comment(lib, "d3dcompiler.lib")
#include <d3dcompiler.h>

#include "WICTextureLoader.h"

// For the DirectX Math library
using namespace DirectX;

// --------------------------------------------------------
// Constructor
//
// DXCore (base class) constructor will set up underlying fields.
// Direct3D itself, and our window, are not ready at this point!
//
// hInstance - the application's OS-level handle (unique ID)
// --------------------------------------------------------
Game::Game(HINSTANCE hInstance)
	: DXCore(
		hInstance,			// The application's handle
		L"DirectX Game",	// Text for the window's title bar (as a wide-character string)
		1280,				// Width of the window's client area
		720,				// Height of the window's client area
		false,				// Sync the framerate to the monitor refresh? (lock framerate)
		true)				// Show extra stats (fps) in title bar?
{
#if defined(DEBUG) || defined(_DEBUG)
	// Do we want a console window?  Probably only in debug mode
	CreateConsoleWindow(500, 120, 32, 120);
	printf("Console window created successfully.  Feel free to printf() here.\n");
#endif

	//Set initial selected camera
	selectedCameraIndex = 0;

	//Pink ambient color
	//ambientColor = DirectX::XMFLOAT3(0.03f, 0.015f, 0.03f);
	ambientColor = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);

	//Set initial light variables
	directionalLight1 = {};
	directionalLight2 = {};
	directionalLight3 = {};
	pointLight1 = {};
	pointLight2 = {};

	//Set initial shadow map variables
	shadowMapResolution = 1024;
	lightProjectionMatrix = XMFLOAT4X4();
	lightViewMatrix = XMFLOAT4X4();

	blurRadius = 1;
}

// --------------------------------------------------------
// Destructor - Clean up anything our game has created:
//  - Delete all objects manually created within this class
//  - Release() all Direct3D objects created within this class
// --------------------------------------------------------
Game::~Game() {
	// Call delete or delete[] on any objects or arrays you've
	// created using new or new[] within this class
	// - Note: this is unnecessary if using smart pointers

	// Call Release() on any Direct3D objects made within this class
	// - Note: this is unnecessary for D3D objects stored in ComPtrs

	//ImGui clean up
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

// --------------------------------------------------------
// Called once per program, after Direct3D and the window
// are initialized but before the game loop.
// --------------------------------------------------------
void Game::Init() {
	// Helper methods for loading shaders, creating some basic
	// geometry to draw and some simple camera matrices.
	//  - You'll be expanding and/or replacing these later
	LoadShaders();

	CreateMaterials();

	CreateGeometry();

	CreateSky();

	CreateShadowMap();

	//ambientColor = DirectX::XMFLOAT3(0.1f, 0.1f, 0.25f);
	//ambientColor = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);

	CreateLights();

	CreatePostProcessResources();


	//Create cameras
	cameras.push_back(std::make_shared<Camera>(
		0.0f, 1.0f, -8.0f,
		5.0f,
		0.01f,
		DirectX::XM_PI / 4.0f,
		(float)this->windowWidth / this->windowHeight
	));

	cameras.push_back(std::make_shared<Camera>(
		3.0f, 10.0f, -12.0f,
		5.0f,
		0.01f,
		DirectX::XM_PI / 2.0f,
		(float)this->windowWidth / this->windowHeight
	));

	// Set initial graphics API state
	//  - These settings persist until we change them
	//  - Some of these, like the primitive topology & input layout, probably won't change
	//  - Others, like setting shaders, will need to be moved elsewhere later
	{
		// Tell the input assembler (IA) stage of the pipeline what kind of
		// geometric primitives (points, lines or triangles) we want to draw.  
		// Essentially: "What kind of shape should the GPU draw with our vertices?"
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// Ensure the pipeline knows how to interpret all the numbers stored in
		// the vertex buffer. For this course, all of your vertices will probably
		// have the same layout, so we can just set this once at startup.
		//context->IASetInputLayout(inputLayout.Get());

		// Set the active vertex and pixel shaders
		//  - Once you start applying different shaders to different objects,
		//    these calls will need to happen multiple times per frame
		//context->VSSetShader(vertexShader.Get(), 0, 0);
		//context->PSSetShader(pixelShader.Get(), 0, 0);
	}

	// Initialize ImGui itself & platform/renderer backends
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplWin32_Init(hWnd);
	ImGui_ImplDX11_Init(device.Get(), context.Get());

	// Pick a style (uncomment one of these 3)
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsLight();
	//ImGui::StyleColorsClassic();
}

// --------------------------------------------------------
// Loads shaders from compiled shader object (.cso) files
// and also created the Input Layout that describes our 
// vertex data to the rendering pipeline. 
// - Input Layout creation is done here because it must 
//    be verified against vertex shader byte code
// - We'll have that byte code already loaded below
// --------------------------------------------------------
void Game::LoadShaders() {
	//(0)
	vertexShaders.push_back(std::make_shared<SimpleVertexShader>(device, context,
		FixPath(L"VertexShader.cso").c_str()));

	//(1)
	vertexShaders.push_back(std::make_shared<SimpleVertexShader>(device, context,
		FixPath(L"SkyVertexShader.cso").c_str()));

	//(2)
	vertexShaders.push_back(std::make_shared<SimpleVertexShader>(device, context,
		FixPath(L"ShadowMapVS.cso").c_str()));

	//(3)
	vertexShaders.push_back(std::make_shared<SimpleVertexShader>(device, context,
		FixPath(L"PostProcessVS.cso").c_str()));

	ppVS = vertexShaders[3];

	//(0)
	pixelShaders.push_back(std::make_shared<SimplePixelShader>(device, context,
		FixPath(L"PixelShader.cso").c_str()));

	//(1)
	pixelShaders.push_back(std::make_shared<SimplePixelShader>(device, context,
		FixPath(L"CustomPS.cso").c_str()));

	//(2)
	pixelShaders.push_back(std::make_shared<SimplePixelShader>(device, context,
		FixPath(L"SkyPixelShader.cso").c_str()));
	
	//(3)
	pixelShaders.push_back(std::make_shared<SimplePixelShader>(device, context,
		FixPath(L"PostProcessBlurPS.cso").c_str()));

	ppPS = pixelShaders[3];
}

//Loads textures and creates materials
void Game::CreateMaterials() {
	//Load Textures
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> flatMapSRV;
	CreateWICTextureFromFile(
		device.Get(),
		context.Get(),
		FixPath(L"../../Assets/Textures/flat_normals.png").c_str(),
		0,
		flatMapSRV.GetAddressOf()
	);

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> defaultSpecularSRV;
	CreateWICTextureFromFile(
		device.Get(),
		context.Get(),
		FixPath(L"../../Assets/Textures/default_specular.png").c_str(),
		0,
		defaultSpecularSRV.GetAddressOf()
	);

	//COBBLESTONE PBR SRVs
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> cobbleAlbedoSRV;
	CreateWICTextureFromFile(
		device.Get(),
		context.Get(),
		FixPath(L"../../Assets/Textures/PBR/cobblestone_albedo.png").c_str(),
		0,
		cobbleAlbedoSRV.GetAddressOf()
	);

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> cobbleMetalSRV;
	CreateWICTextureFromFile(
		device.Get(),
		context.Get(),
		FixPath(L"../../Assets/Textures/PBR/cobblestone_metal.png").c_str(),
		0,
		cobbleMetalSRV.GetAddressOf()
	);

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> cobbleNormalSRV;
	CreateWICTextureFromFile(
		device.Get(),
		context.Get(),
		FixPath(L"../../Assets/Textures/PBR/cobblestone_normals.png").c_str(),
		0,
		cobbleNormalSRV.GetAddressOf()
	);

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> cobbleRoughnessSRV;
	CreateWICTextureFromFile(
		device.Get(),
		context.Get(),
		FixPath(L"../../Assets/Textures/PBR/cobblestone_roughness.png").c_str(),
		0,
		cobbleRoughnessSRV.GetAddressOf()
	);

	/*//BRONZE PBR SRVs
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> bronzeAlbedoSRV;
	CreateWICTextureFromFile(
		device.Get(),
		context.Get(),
		FixPath(L"../../Assets/Textures/PBR/bronze_albedo.png").c_str(),
		0,
		bronzeAlbedoSRV.GetAddressOf()
	);

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> bronzeMetalSRV;
	CreateWICTextureFromFile(
		device.Get(),
		context.Get(),
		FixPath(L"../../Assets/Textures/PBR/bronze_metal.png").c_str(),
		0,
		bronzeMetalSRV.GetAddressOf()
	);

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> bronzeNormalSRV;
	CreateWICTextureFromFile(
		device.Get(),
		context.Get(),
		FixPath(L"../../Assets/Textures/PBR/bronze_normals.png").c_str(),
		0,
		bronzeNormalSRV.GetAddressOf()
	);

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> bronzeRoughnessSRV;
	CreateWICTextureFromFile(
		device.Get(),
		context.Get(),
		FixPath(L"../../Assets/Textures/PBR/bronze_roughness.png").c_str(),
		0,
		bronzeRoughnessSRV.GetAddressOf()
	);

	//FLOOR PBR SRVs
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> floorAlbedoSRV;
	CreateWICTextureFromFile(
		device.Get(),
		context.Get(),
		FixPath(L"../../Assets/Textures/PBR/floor_albedo.png").c_str(),
		0,
		floorAlbedoSRV.GetAddressOf()
	);

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> floorMetalSRV;
	CreateWICTextureFromFile(
		device.Get(),
		context.Get(),
		FixPath(L"../../Assets/Textures/PBR/floor_metal.png").c_str(),
		0,
		floorMetalSRV.GetAddressOf()
	);

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> floorNormalSRV;
	CreateWICTextureFromFile(
		device.Get(),
		context.Get(),
		FixPath(L"../../Assets/Textures/PBR/floor_normals.png").c_str(),
		0,
		floorNormalSRV.GetAddressOf()
	);

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> floorRoughnessSRV;
	CreateWICTextureFromFile(
		device.Get(),
		context.Get(),
		FixPath(L"../../Assets/Textures/PBR/floor_roughness.png").c_str(),
		0,
		floorRoughnessSRV.GetAddressOf()
	);

	//PAINT PBR SRVs
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> paintAlbedoSRV;
	CreateWICTextureFromFile(
		device.Get(),
		context.Get(),
		FixPath(L"../../Assets/Textures/PBR/paint_albedo.png").c_str(),
		0,
		paintAlbedoSRV.GetAddressOf()
	);

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> paintMetalSRV;
	CreateWICTextureFromFile(
		device.Get(),
		context.Get(),
		FixPath(L"../../Assets/Textures/PBR/paint_metal.png").c_str(),
		0,
		paintMetalSRV.GetAddressOf()
	);

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> paintNormalSRV;
	CreateWICTextureFromFile(
		device.Get(),
		context.Get(),
		FixPath(L"../../Assets/Textures/PBR/paint_normals.png").c_str(),
		0,
		paintNormalSRV.GetAddressOf()
	);

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> paintRoughnessSRV;
	CreateWICTextureFromFile(
		device.Get(),
		context.Get(),
		FixPath(L"../../Assets/Textures/PBR/paint_roughness.png").c_str(),
		0,
		paintRoughnessSRV.GetAddressOf()
	);

	//ROUGH PBR SRVs
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> roughAlbedoSRV;
	CreateWICTextureFromFile(
		device.Get(),
		context.Get(),
		FixPath(L"../../Assets/Textures/PBR/rough_albedo.png").c_str(),
		0,
		roughAlbedoSRV.GetAddressOf()
	);

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> roughMetalSRV;
	CreateWICTextureFromFile(
		device.Get(),
		context.Get(),
		FixPath(L"../../Assets/Textures/PBR/rough_metal.png").c_str(),
		0,
		roughMetalSRV.GetAddressOf()
	);

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> roughNormalSRV;
	CreateWICTextureFromFile(
		device.Get(),
		context.Get(),
		FixPath(L"../../Assets/Textures/PBR/rough_normals.png").c_str(),
		0,
		roughNormalSRV.GetAddressOf()
	);

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> roughRoughnessSRV;
	CreateWICTextureFromFile(
		device.Get(),
		context.Get(),
		FixPath(L"../../Assets/Textures/PBR/rough_roughness.png").c_str(),
		0,
		roughRoughnessSRV.GetAddressOf()
	);

	//SCRATCHED PBR SRVs
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> scratchedAlbedoSRV;
	CreateWICTextureFromFile(
		device.Get(),
		context.Get(),
		FixPath(L"../../Assets/Textures/PBR/scratched_albedo.png").c_str(),
		0,
		scratchedAlbedoSRV.GetAddressOf()
	);

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> scratchedMetalSRV;
	CreateWICTextureFromFile(
		device.Get(),
		context.Get(),
		FixPath(L"../../Assets/Textures/PBR/scratched_metal.png").c_str(),
		0,
		scratchedMetalSRV.GetAddressOf()
	);

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> scratchedNormalSRV;
	CreateWICTextureFromFile(
		device.Get(),
		context.Get(),
		FixPath(L"../../Assets/Textures/PBR/scratched_normals.png").c_str(),
		0,
		scratchedNormalSRV.GetAddressOf()
	);

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> scratchedRoughnessSRV;
	CreateWICTextureFromFile(
		device.Get(),
		context.Get(),
		FixPath(L"../../Assets/Textures/PBR/scratched_roughness.png").c_str(),
		0,
		scratchedRoughnessSRV.GetAddressOf()
	);

	//WOOD PBR SRVs
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> woodAlbedoSRV;
	CreateWICTextureFromFile(
		device.Get(),
		context.Get(),
		FixPath(L"../../Assets/Textures/PBR/wood_albedo.png").c_str(),
		0,
		woodAlbedoSRV.GetAddressOf()
	);

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> woodMetalSRV;
	CreateWICTextureFromFile(
		device.Get(),
		context.Get(),
		FixPath(L"../../Assets/Textures/PBR/wood_metal.png").c_str(),
		0,
		woodMetalSRV.GetAddressOf()
	);

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> woodNormalSRV;
	CreateWICTextureFromFile(
		device.Get(),
		context.Get(),
		FixPath(L"../../Assets/Textures/PBR/wood_normals.png").c_str(),
		0,
		woodNormalSRV.GetAddressOf()
	);

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> woodRoughnessSRV;
	CreateWICTextureFromFile(
		device.Get(),
		context.Get(),
		FixPath(L"../../Assets/Textures/PBR/wood_roughness.png").c_str(),
		0,
		woodRoughnessSRV.GetAddressOf()
	);
	*/

	Microsoft::WRL::ComPtr<ID3D11SamplerState> defaultSampler;

	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	samplerDesc.MaxAnisotropy = 16;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	device->CreateSamplerState(&samplerDesc, defaultSampler.GetAddressOf());

	//(0) Create Bronze PBR Material
	//materials.push_back(std::make_shared<Material>(DirectX::XMFLOAT4(1, 1, 1, 1), 0.1f, vertexShaders[0], pixelShaders[0]));
	//materials[0]->AddTextureSRV("Albedo", bronzeAlbedoSRV);
	//materials[0]->AddTextureSRV("NormalMap", bronzeNormalSRV);
	//materials[0]->AddTextureSRV("RoughnessMap", bronzeRoughnessSRV);
	//materials[0]->AddTextureSRV("MetalnessMap", bronzeMetalSRV);
	//materials[0]->AddSampler("BasicSampler", defaultSampler);

	//(1) Create Cobblestone PBR Material
	materials.push_back(std::make_shared<Material>(DirectX::XMFLOAT4(1, 1, 1, 1), 0.1f, vertexShaders[0], pixelShaders[0]));
	materials[0]->AddTextureSRV("Albedo", cobbleAlbedoSRV);
	materials[0]->AddTextureSRV("NormalMap", cobbleNormalSRV);
	materials[0]->AddTextureSRV("RoughnessMap", cobbleRoughnessSRV);
	materials[0]->AddTextureSRV("MetalnessMap", cobbleMetalSRV);
	materials[0]->AddSampler("BasicSampler", defaultSampler);

	//(2) Create Floor PBR Material
	//materials.push_back(std::make_shared<Material>(DirectX::XMFLOAT4(1, 1, 1, 1), 0.1f, vertexShaders[0], pixelShaders[0]));
	//materials[2]->AddTextureSRV("Albedo", floorAlbedoSRV);
	//materials[2]->AddTextureSRV("NormalMap", floorNormalSRV);
	//materials[2]->AddTextureSRV("RoughnessMap", floorRoughnessSRV);
	//materials[2]->AddTextureSRV("MetalnessMap", floorMetalSRV);
	//materials[2]->AddSampler("BasicSampler", defaultSampler);

	//(3) Create Paint PBR Material
	/*materials.push_back(std::make_shared<Material>(DirectX::XMFLOAT4(1, 1, 1, 1), 0.1f, vertexShaders[0], pixelShaders[0]));
	materials[0]->AddTextureSRV("Albedo", paintAlbedoSRV);
	materials[0]->AddTextureSRV("NormalMap", paintNormalSRV);
	materials[0]->AddTextureSRV("RoughnessMap", paintRoughnessSRV);
	materials[0]->AddTextureSRV("MetalnessMap", paintMetalSRV);
	materials[0]->AddSampler("BasicSampler", defaultSampler);*/

	//(4) Create Rough PBR Material
	/*materials.push_back(std::make_shared<Material>(DirectX::XMFLOAT4(1, 1, 1, 1), 0.1f, vertexShaders[0], pixelShaders[0]));
	materials[1]->AddTextureSRV("Albedo", roughAlbedoSRV);
	materials[1]->AddTextureSRV("NormalMap", roughNormalSRV);
	materials[1]->AddTextureSRV("RoughnessMap", roughRoughnessSRV);
	materials[1]->AddTextureSRV("MetalnessMap", roughMetalSRV);
	materials[1]->AddSampler("BasicSampler", defaultSampler);*/

	//(5) Create Scratched PBR Material
	/*materials.push_back(std::make_shared<Material>(DirectX::XMFLOAT4(1, 1, 1, 1), 0.1f, vertexShaders[0], pixelShaders[0]));
	materials[2]->AddTextureSRV("Albedo", scratchedAlbedoSRV);
	materials[2]->AddTextureSRV("NormalMap", scratchedNormalSRV);
	materials[2]->AddTextureSRV("RoughnessMap", scratchedRoughnessSRV);
	materials[2]->AddTextureSRV("MetalnessMap", scratchedMetalSRV);
	materials[2]->AddSampler("BasicSampler", defaultSampler);*/

	//(6) Create Wood PBR Material
	/*materials.push_back(std::make_shared<Material>(DirectX::XMFLOAT4(1, 1, 1, 1), 0.1f, vertexShaders[0], pixelShaders[0]));
	materials[3]->AddTextureSRV("Albedo", woodAlbedoSRV);
	materials[3]->AddTextureSRV("NormalMap", woodNormalSRV);
	materials[3]->AddTextureSRV("RoughnessMap", woodRoughnessSRV);
	materials[3]->AddTextureSRV("MetalnessMap", woodMetalSRV);
	materials[3]->AddSampler("BasicSampler", defaultSampler);*/
}

void Game::CreateSky() {
	//Create the sample state
	Microsoft::WRL::ComPtr<ID3D11SamplerState> defaultSampler;

	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	samplerDesc.MaxAnisotropy = 16;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	device->CreateSamplerState(&samplerDesc, defaultSampler.GetAddressOf());

	//Create the Cube map
	std::vector<std::wstring> skyCubeMap;

	//CubeMap MUST be loaded in the following order:
	//right, left, up, down, front, back
	skyCubeMap.push_back(FixPath(L"../../Assets/Skies/Clouds Pink/right.png"));
	skyCubeMap.push_back(FixPath(L"../../Assets/Skies/Clouds Pink/left.png"));
	skyCubeMap.push_back(FixPath(L"../../Assets/Skies/Clouds Pink/up.png"));
	skyCubeMap.push_back(FixPath(L"../../Assets/Skies/Clouds Pink/down.png"));
	skyCubeMap.push_back(FixPath(L"../../Assets/Skies/Clouds Pink/front.png"));
	skyCubeMap.push_back(FixPath(L"../../Assets/Skies/Clouds Pink/back.png"));

	sky = std::make_shared<Sky>(
		meshes[0],			//Cube Mesh
		defaultSampler,		//Sampler State
		device,				//Device
		context,			//Context
		vertexShaders[1],	//SkyVertexShader
		pixelShaders[2],	//SkyPixelShader
		skyCubeMap			//List of Sky Cube Map images
	);
}

//Creates Lights
void Game::CreateLights() {
	//Directional light pointing right
	directionalLight1 = {};
	directionalLight1.Type = LIGHT_TYPE_DIRECTIONAL;
	//directionalLight1.Direction = DirectX::XMFLOAT3(0.6f, -1.0f, 1.15f);
	directionalLight1.Direction = DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f);
	directionalLight1.Color = DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f);
	directionalLight1.Intensity = 0.5f;

	//Directional light pointing down
	directionalLight2 = {};
	directionalLight2.Type = LIGHT_TYPE_DIRECTIONAL;
	directionalLight2.Direction = DirectX::XMFLOAT3(-1.0f, -0.25f, 0.15f);
	directionalLight2.Color = DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f);
	directionalLight2.Intensity = 0.5f;

	//Purple point light
	//pointLight1 = {};
	//pointLight1.Type = LIGHT_TYPE_POINT;
	//pointLight1.Range = 10.0f;
	//pointLight1.Position = DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f);
	//pointLight1.Color = DirectX::XMFLOAT3(1.0f, 0.0f, 1.0f);
	////directionalLight1.Color = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
	//pointLight1.Intensity = 1.0f;

	//Add lights to lights vector
	lights.push_back(&directionalLight1);
	lights.push_back(&directionalLight2);
	lights.push_back(&directionalLight3);
	lights.push_back(&pointLight1);
	lights.push_back(&pointLight2);

	XMMATRIX lightView = XMMatrixLookToLH(
		-DirectX::XMLoadFloat3(&directionalLight1.Direction) * 20,	// Position: "Backing up" 20 units from origin
		DirectX::XMLoadFloat3(&directionalLight1.Direction),		// Direction: light's direction
		XMVectorSet(0, 1, 0, 0));									// Up: World up vector (Y axis)

	float lightProjectionSize = 15.0f; // Tweak for your scene!
	XMMATRIX lightProjection = XMMatrixOrthographicLH(
		lightProjectionSize,
		lightProjectionSize,
		1.0f,
		100.0f);

	DirectX::XMStoreFloat4x4(&lightViewMatrix, lightView);
	DirectX::XMStoreFloat4x4(&lightProjectionMatrix, lightProjection);

	//Send the texture data to all materials
	for (std::shared_ptr<Material> material : materials) {
		material->AddTextureSRV("ShadowMap", shadowSRV);
		material->AddSampler("ShadowSampler", shadowSampler);
	}
}



// --------------------------------------------------------
// Creates the geometry we're going to draw - a single triangle for now
// --------------------------------------------------------
void Game::CreateGeometry() {
	// Create some temporary variables to represent colors
	// - Not necessary, just makes things more readable

	// Set up the vertices of the triangle we would like to draw
	// - We're going to copy this array, exactly as it exists in CPU memory
	//    over to a Direct3D-controlled data structure on the GPU (the vertex buffer)
	// - Note: Since we don't have a camera or really any concept of
	//    a "3d world" yet, we're simply describing positions within the
	//    bounds of how the rasterizer sees our screen: [-1 to +1] on X and Y
	// - This means (0,0) is at the very center of the screen.
	// - These are known as "Normalized Device Coordinates" or "Homogeneous 
	//    Screen Coords", which are ways to describe a position without
	//    knowing the exact size (in pixels) of the image/window/etc.  
	// - Long story short: Resizing the window also resizes the triangle,
	//    since we're describing the triangle in terms of the window itself
	
	//Add meshes from file into meshes vector
	//(0) Cube Mesh
	meshes.push_back(std::make_shared<Mesh>(FixPath(L"../../Assets/Models/cube.obj").c_str(), device, context));

	////(1) Cylinder Mesh
	//meshes.push_back(std::make_shared<Mesh>(FixPath(L"../../Assets/Models/cylinder.obj").c_str(), device, context));

	//(2) Helix Mesh
	//meshes.push_back(std::make_shared<Mesh>(FixPath(L"../../Assets/Models/helix.obj").c_str(), device, context));

	////(3) Quad Mesh
	//meshes.push_back(std::make_shared<Mesh>(FixPath(L"../../Assets/Models/quad.obj").c_str(), device, context));

	////(4) Double Sided Quad Mesh
	//meshes.push_back(std::make_shared<Mesh>(FixPath(L"../../Assets/Models/quad_double_sided.obj").c_str(), device, context));

	//(5) Sphere Mesh
	meshes.push_back(std::make_shared<Mesh>(FixPath(L"../../Assets/Models/sphere.obj").c_str(), device, context));

	////(6) Torus Mesh
	//meshes.push_back(std::make_shared<Mesh>(FixPath(L"../../Assets/Models/torus.obj").c_str(), device, context));

	//Create entities using the meshes and materials
	//Floor
	//entities.push_back(std::make_shared<Entity>(meshes[0], materials[3]));
	//entities[0]->GetTransform()->SetPosition(0.0f, 0.0f, 0.0f);
	//entities[0]->GetTransform()->SetScale(25.0f, 0.01f, 25.0f);

	//Sphere Entity
	entities.push_back(std::make_shared<Entity>(meshes[1], materials[0]));
	entities[0]->GetTransform()->SetPosition(-0.0f, 1.25f, 0.0f);

	//Helix Entity
	//entities.push_back(std::make_shared<Entity>(meshes[1], materials[0]));
	//entities[2]->GetTransform()->SetPosition(0.0f, 1.25f, 0.0f);

	//Cube Entity
	//entities.push_back(std::make_shared<Entity>(meshes[0], materials[1]));
	//entities[3]->GetTransform()->SetPosition(5.0f, 1.25f, 0.0f);
}

void Game::CreateShadowMap() {
	// Create the actual texture that will be the shadow map
	D3D11_TEXTURE2D_DESC shadowDesc = {};
	shadowDesc.Width = shadowMapResolution; // Ideally a power of 2 (like 1024)
	shadowDesc.Height = shadowMapResolution; // Ideally a power of 2 (like 1024)
	shadowDesc.ArraySize = 1;
	shadowDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	shadowDesc.CPUAccessFlags = 0;
	shadowDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	shadowDesc.MipLevels = 1;
	shadowDesc.MiscFlags = 0;
	shadowDesc.SampleDesc.Count = 1;
	shadowDesc.SampleDesc.Quality = 0;
	shadowDesc.Usage = D3D11_USAGE_DEFAULT;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> shadowTexture;
	device->CreateTexture2D(&shadowDesc, 0, shadowTexture.GetAddressOf());

	// Create the depth/stencil view
	D3D11_DEPTH_STENCIL_VIEW_DESC shadowDSDesc = {};
	shadowDSDesc.Format = DXGI_FORMAT_D32_FLOAT;
	shadowDSDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	shadowDSDesc.Texture2D.MipSlice = 0;
	device->CreateDepthStencilView(
		shadowTexture.Get(),
		&shadowDSDesc,
		shadowDSV.GetAddressOf());

	// Create the SRV for the shadow map
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.MostDetailedMip = 0;
	device->CreateShaderResourceView(
		shadowTexture.Get(),
		&srvDesc,
		shadowSRV.GetAddressOf());

	//Create rasterizer state for depth biasing (fixes shadow acne)
	D3D11_RASTERIZER_DESC shadowRastDesc = {};
	shadowRastDesc.FillMode = D3D11_FILL_SOLID;
	shadowRastDesc.CullMode = D3D11_CULL_BACK;
	shadowRastDesc.DepthClipEnable = true;
	shadowRastDesc.DepthBias = 1000; // Min. precision units, not world units!
	shadowRastDesc.SlopeScaledDepthBias = 1.0f; // Bias more based on slope
	device->CreateRasterizerState(&shadowRastDesc, &shadowRasterizer);

	//Create sampler for comparison
	D3D11_SAMPLER_DESC shadowSampDesc = {};
	shadowSampDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
	shadowSampDesc.ComparisonFunc = D3D11_COMPARISON_LESS;
	shadowSampDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSampDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSampDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSampDesc.BorderColor[0] = 1.0f; // Only need the first component
	device->CreateSamplerState(&shadowSampDesc, &shadowSampler);
}

void Game::CreatePostProcessResources() {
	// Sampler state for post processing
	D3D11_SAMPLER_DESC ppSampDesc = {};
	ppSampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	ppSampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	ppSampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	ppSampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	ppSampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	device->CreateSamplerState(&ppSampDesc, ppSampler.GetAddressOf());

	CreatePostProcessTexture();
}

void Game::CreatePostProcessTexture() {
	// Describe the texture we're creating
	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.Width = windowWidth;
	textureDesc.Height = windowHeight;
	textureDesc.ArraySize = 1;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.MipLevels = 1;
	textureDesc.MiscFlags = 0;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;

	// Create the resource (no need to track it after the views are created below)
	Microsoft::WRL::ComPtr<ID3D11Texture2D> ppTexture;
	device->CreateTexture2D(&textureDesc, 0, ppTexture.GetAddressOf());

	// Create the Render Target View
	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = textureDesc.Format;
	rtvDesc.Texture2D.MipSlice = 0;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	device->CreateRenderTargetView(
		ppTexture.Get(),
		&rtvDesc,
		ppRTV.ReleaseAndGetAddressOf());

	// Create the Shader Resource View
	// By passing it a null description for the SRV, we
	// get a "default" SRV that has access to the entire resource
	device->CreateShaderResourceView(
		ppTexture.Get(),
		0,
		ppSRV.ReleaseAndGetAddressOf());
}


// --------------------------------------------------------
// Handle resizing to match the new window size.
//  - DXCore needs to resize the back buffer
//  - Eventually, we'll want to update our 3D camera
// --------------------------------------------------------
void Game::OnResize() {
	// Handle base-level DX resize stuff
	DXCore::OnResize();

	//Reset Screen Sized Render Targets
	ppSRV.Reset();
	ppRTV.Reset();

	//Recreate the post process texture, RTV, and SRV using new screen sizes
	CreatePostProcessTexture();

	//Update all camera projection matrices
	for (std::shared_ptr<Camera> camera : cameras) {
		camera->UpdateProjectionMatrix((float)this->windowWidth / this->windowHeight);
	}
}

// --------------------------------------------------------
// Update your game here - user input, move objects, AI, etc.
// --------------------------------------------------------
void Game::Update(float deltaTime, float totalTime) {
	//Update ImGui
	UpdateGui(deltaTime);

	//Create the ImGui windows
	CreateWindowInfoGui();
	CreateInspectorGui();

	//Apply transformations to entities
	//entities[0]->GetTransform()->SetScale((sin(totalTime) + 2.0f) / 2.0f, (sin(totalTime) + 2.0f) / 2.0f, 0.0f);
	entities[0]->GetTransform()->Rotate(0.0f, 1.0f * deltaTime, 0.0f);

	//Update the selected camera
	cameras[selectedCameraIndex]->Update(deltaTime);

	// Example input checking: Quit if the escape key is pressed
	if (Input::GetInstance().KeyDown(VK_ESCAPE))
		Quit();
}

// --------------------------------------------------------
// Update ImGui
// --------------------------------------------------------
void Game::UpdateGui(float deltaTime) {
	// Feed fresh input data to ImGui
	ImGuiIO& io = ImGui::GetIO();
	io.DeltaTime = deltaTime;
	io.DisplaySize.x = (float)this->windowWidth;
	io.DisplaySize.y = (float)this->windowHeight;

	// Reset the frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	// Determine new input capture
	Input& input = Input::GetInstance();
	input.SetKeyboardCapture(io.WantCaptureKeyboard);
	input.SetMouseCapture(io.WantCaptureMouse);

	// Show the demo window
	//ImGui::ShowDemoWindow();
}

// --------------------------------------------------------
// Create Window Information Gui
// --------------------------------------------------------
void Game::CreateWindowInfoGui() {
	ImGui::Begin("Program Information");

	//Display window dimensions
	ImGui::Text("Window Dimensions: %ux%u", windowWidth, windowHeight);

	//Display FPS
	ImGui::Text("FPS: %u", ImGui::GetIO().Framerate);

	//List of items in dropdown.
	//Cannot be deleted or manipulated, and will not cause memory leak
	//(https://stackoverflow.com/questions/2001286/const-char-s-in-c#:~:text=It%20cannot%20be%20deleted%2C%20and%20should%20not%20be%20manipulated.)

	//Deciding against this dropdown for now, will revisit later
	/*
	const char* items[] = { "Unlimited", "30", "60", "120", "144" };

	static int currentItem = 0;

	ImGui::Combo("FPS Limit", &currentItem, items, IM_ARRAYSIZE(items));
	*/

	//Create VSync Checkbox
	ImGui::Checkbox("VSync", &vsync);

	//Create Title Bar Stats Update Checkbox
	ImGui::Checkbox("Update Title Bar Stats", &titleBarStats);

	ImGui::End();
}

// --------------------------------------------------------
// Create Mesh Gui
// --------------------------------------------------------
void Game::CreateInspectorGui() {
	ImGui::Begin("Game Inspector");

	//Create the root node for entities
	if (ImGui::TreeNode("Entities")) {
		int index = 0;
		//Loop through each mesh and make a node for it with child properties
		for (std::shared_ptr<Entity> entity : entities) {
			if (ImGui::TreeNode((void*)(intptr_t)index, "Entity %d (%d indices)", index, entity->GetMesh()->GetIndexCount())) {
				auto entityTint = entity->GetMaterial()->GetColorTint();
				auto entityPosition = entity->GetTransform()->GetPosition();
				auto entityRotation = entity->GetTransform()->GetPitchYawRoll();
				auto entityScale = entity->GetTransform()->GetScale();

				//Mesh Color
				if (ImGui::ColorEdit4("Tint", &entityTint.x)) {
					entity->GetMaterial()->SetColorTint(entityTint);
				}

				//Entity Position
				if (ImGui::DragFloat3("Position", &entityPosition.x, 0.005f)) {
					entity->GetTransform()->SetPosition(entityPosition);
				}

				//Entity Rotation
				if (ImGui::DragFloat3("Rotation", &entityRotation.x, 0.005f)) {
					entity->GetTransform()->SetRotation(entityRotation);
				}

				//Entity Scale
				if (ImGui::DragFloat3("Scale", &entityScale.x, 0.005f)) {
					entity->GetTransform()->SetScale(entityScale);
				}

				ImGui::TreePop();
			}

			index++;
		}
		ImGui::TreePop();
	}

	//Create the root node for cameras
	if (ImGui::TreeNode("Cameras")) {
		int size = static_cast<int>(cameras.size());

		//Create previous camera button
		if (ImGui::Button("Select Previous Camera")) {
			//Check for wrapping to last camera in list.
			//Otherwise, just go to the previous camera
			if (selectedCameraIndex - 1 < 0) {
				selectedCameraIndex = size - 1;
			} else {
				selectedCameraIndex--;
			}
		}

		//Create next camera button
		if (ImGui::Button("Select Next Camera")) {
			//Check for wrapping to first camera in list.
			//Otherwise, just go to the next camera
			if (selectedCameraIndex + 1 >= size) {
				selectedCameraIndex = 0;
			} else {
				selectedCameraIndex++;
			}
		}

		int index = 0;

		//Loop through each camera and make a node for it with child properties
		for (std::shared_ptr<Camera> camera : cameras) {
			if (ImGui::TreeNode((void*)(intptr_t)index, "Camera %d %s", index, (selectedCameraIndex == index ? "(Selected)" : ""))) {
				auto cameraPosition = camera->GetTransform()->GetPosition();
				auto cameraRotation = camera->GetTransform()->GetPitchYawRoll();
				auto cameraMoveSpeed = camera->GetMoveSpeed();
				auto cameraRotationSpeed = camera->GetRotationSpeed();
				auto cameraFieldOfView = camera->GetFieldOfView();
				/*auto cameraRight = camera->GetTransform().GetRight();
				auto cameraUp = camera->GetTransform().GetUp();
				auto cameraForward = camera->GetTransform().GetForward();*/

				//Camera Position
				ImGui::Text("Position: [%f, %f, %f]", cameraPosition.x, cameraPosition.y, cameraPosition.z);

				//Camera Rotation
				ImGui::Text("Rotation: [%f, %f, %f]", cameraRotation.x, cameraRotation.y, cameraRotation.z);

				ImGui::Text("Field Of View: %f", cameraFieldOfView);

				if (ImGui::DragFloat("Move Speed", &cameraMoveSpeed, 0.005f)) {
					camera->SetMoveSpeed(cameraMoveSpeed);
				}

				if (ImGui::DragFloat("Rotation Speed", &cameraRotationSpeed, 0.0001f)) {
					camera->SetRotationSpeed(cameraRotationSpeed);
				}

				/*ImGui::DragFloat3("Right", &cameraRight.x, 0.005f);
				ImGui::DragFloat3("Up", &cameraUp.x, 0.005f);
				ImGui::DragFloat3("Forward", &cameraForward.x, 0.005f);*/

				ImGui::TreePop();
			}

			index++;
		}


		ImGui::TreePop();
	}

	//Create the root node for lights
	if (ImGui::TreeNode("Lights")) {
		ImGui::ColorEdit3("Ambient Light", &ambientColor.x);

		int index = 0;

		//Loop through each light and make a node for it with child properties
		for (Light* light : lights) {
			if (ImGui::TreeNode((void*)(intptr_t)index, "Light %d", index)) {
				ImGui::Text("Type: %s", (light->Type == 0 ? "Directional" : (light->Type == 1 ? "Point" : "Spot")));
				if (ImGui::DragFloat3("Direction", &light->Direction.x, 0.005f)) {
					XMMATRIX lightView = XMMatrixLookToLH(
						-DirectX::XMLoadFloat3(&light->Direction) * 20,	// Position: "Backing up" 20 units from origin
						DirectX::XMLoadFloat3(&light->Direction),		// Direction: light's direction
						XMVectorSet(0, 1, 0, 0));						// Up: World up vector (Y axis)

					DirectX::XMStoreFloat4x4(&lightViewMatrix, lightView);
				}
				ImGui::DragFloat("Range", &light->Range, 0.005f);
				ImGui::DragFloat3("Position", &light->Position.x, 0.005f);
				ImGui::DragFloat("Intensity", &light->Intensity, 0.005f);
				ImGui::ColorEdit3("Color", &light->Color.x);
				ImGui::DragFloat("Spot Falloff", &light->SpotFalloff, 0.005f);
				ImGui::DragFloat3("Padding", &light->Padding.x, 0.005f);
				ImGui::TreePop();
			}

			index++;
		}

		ImGui::TreePop();
	}

	//Create the root node for post processing
	if (ImGui::TreeNode("Post Processing")) {
		ImGui::DragInt("Blur Radius", &blurRadius, 1.0f, 0, 10);
		ImGui::TreePop();
	}

	//Shadow Map Inspector
	ImGui::Image(shadowSRV.Get(), ImVec2(512, 512));

	ImGui::End();
}

// --------------------------------------------------------
// Clear the screen, redraw everything, present to the user
// --------------------------------------------------------
void Game::Draw(float deltaTime, float totalTime) {

	//Clear the shadow map
	context->ClearDepthStencilView(shadowDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

	//Set up output merger stage
	ID3D11RenderTargetView* nullRTV{};
	context->OMSetRenderTargets(1, &nullRTV, shadowDSV.Get());

	//Deactivate pixel shader
	context->PSSetShader(0, 0, 0);

	//Change viewport
	D3D11_VIEWPORT viewport = {};
	viewport.Width = (float)shadowMapResolution;
	viewport.Height = (float)shadowMapResolution;
	viewport.MaxDepth = 1.0f;
	context->RSSetViewports(1, &viewport);

	//Before rendering shadow map, enable rasterizer state
	context->RSSetState(shadowRasterizer.Get());

	//Entity render loop
	vertexShaders[2]->SetShader();
	vertexShaders[2]->SetMatrix4x4("view", lightViewMatrix);
	vertexShaders[2]->SetMatrix4x4("projection", lightProjectionMatrix);

	// Loop and draw all entities
	for (auto& e : entities) {
		vertexShaders[2]->SetMatrix4x4("world", e->GetTransform()->GetWorldMatrix());
		vertexShaders[2]->CopyAllBufferData();

		// Draw the mesh directly to avoid the entity's material
		// Note: Your code may differ significantly here!
		e->GetMesh()->Draw();
	}

	//Disable shadow map rasterizer state
	context->RSSetState(0);

	//Reset the pipeline
	viewport.Width = (float)this->windowWidth;
	viewport.Height = (float)this->windowHeight;
	context->RSSetViewports(1, &viewport);
	context->OMSetRenderTargets(
		1,
		backBufferRTV.GetAddressOf(),
		depthBufferDSV.Get());

	// Frame START
	// - These things should happen ONCE PER FRAME
	// - At the beginning of Game::Draw() before drawing *anything*
	{
		// Clear the back buffer (erases what's on the screen)
		const float bgColor[4] = { 0.4f, 0.6f, 0.75f, 1.0f }; // Cornflower Blue
		context->ClearRenderTargetView(backBufferRTV.Get(), bgColor);

		// Clear the depth buffer (resets per-pixel occlusion information)
		context->ClearDepthStencilView(depthBufferDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

		//Clear extra render targets
		context->ClearRenderTargetView(ppRTV.Get(), bgColor);
	}

	//Set render targets for post processing
	context->OMSetRenderTargets(1, ppRTV.GetAddressOf(), depthBufferDSV.Get());

	//Loop through the entity vector and draw the entities
	for (std::shared_ptr<Entity> entity : entities) {
		entity->GetMaterial()->GetVertexShader()->SetMatrix4x4("lightView", lightViewMatrix);
		entity->GetMaterial()->GetVertexShader()->SetMatrix4x4("lightProjection", lightProjectionMatrix);

		entity->GetMaterial()->GetPixelShader()->SetFloat3("ambient", ambientColor);

		entity->GetMaterial()->GetPixelShader()->SetData("lights", lights[0], sizeof(Light) * (int)lights.size());

		/*entity->GetMaterial()->GetPixelShader()->SetShaderResourceView("SurfaceTexture", textureSRV);
		entity->GetMaterial()->GetPixelShader()->SetSamplerState("BasicSampler", samplerOptions);*/
		//entity->GetMaterial()->PrepareMaterial(textureSRVs, samplerOptions);
		entity->GetMaterial()->PrepareMaterial();

		entity->Draw(context, cameras[selectedCameraIndex], totalTime);
		
	}

	//Draw the sky AFTER drawing the entities
	sky->Draw(cameras[selectedCameraIndex]);

	//Restore back buffer for post processing
	context->OMSetRenderTargets(1, backBufferRTV.GetAddressOf(), 0);

	// Activate shaders and bind resources
	ppVS->SetShader();
	ppPS->SetShader();
	ppPS->SetInt("blurRadius", blurRadius);
	ppPS->SetFloat("pixelWidth", 1.0f / windowWidth);
	ppPS->SetFloat("pixelHeight", 1.0f / windowHeight);
	ppPS->SetShaderResourceView("Pixels", ppSRV.Get());
	ppPS->SetSamplerState("ClampSampler", ppSampler.Get());
	ppPS->CopyAllBufferData();

	context->Draw(3, 0); // Draw exactly 3 vertices (one triangle)

	//Prepare ImGui buffers
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	// Frame END
	// - These should happen exactly ONCE PER FRAME
	// - At the very end of the frame (after drawing *everything*)
	{
		// Present the back buffer to the user
		//  - Puts the results of what we've drawn onto the window
		//  - Without this, the user never sees anything
		bool vsyncNecessary = vsync || !deviceSupportsTearing || isFullscreen;
		swapChain->Present(
			vsyncNecessary ? 1 : 0,
			vsyncNecessary ? 0 : DXGI_PRESENT_ALLOW_TEARING);

		// Must re-bind buffers after presenting, as they become unbound
		context->OMSetRenderTargets(1, backBufferRTV.GetAddressOf(), depthBufferDSV.Get());
	}

	//Unbind SRVs (prevents shadow map SRV/Depth Buffer issue)
	ID3D11ShaderResourceView* nullSRVs[128] = {};
	context->PSSetShaderResources(0, 128, nullSRVs);
}