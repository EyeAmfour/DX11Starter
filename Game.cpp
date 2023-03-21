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

	//Set initial light variables
	ambientColor = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
	directionalLight1 = {};
	directionalLight2 = {};
	directionalLight3 = {};
	pointLight1 = {};
	pointLight2 = {};
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

	//Load Textures
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> brokenTilesSRV;
	textureSRVs.insert({"SurfaceTexture", brokenTilesSRV});
	CreateWICTextureFromFile(
		device.Get(),
		context.Get(),
		FixPath(L"../../Assets/Textures/brokentiles.png").c_str(),
		0,
		textureSRVs.at("SurfaceTexture").GetAddressOf()
	);

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> brokenTilesSpecularSRV;
	textureSRVs.insert({ "SurfaceTextureSpecular", brokenTilesSpecularSRV });
	CreateWICTextureFromFile(
		device.Get(),
		context.Get(),
		FixPath(L"../../Assets/Textures/brokentiles_specular.png").c_str(),
		0,
		textureSRVs.at("SurfaceTextureSpecular").GetAddressOf()
	);

	Microsoft::WRL::ComPtr<ID3D11SamplerState> brokenTilesSampler;
	samplerOptions.insert({ "BasicSampler", brokenTilesSampler });

	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	samplerDesc.MaxAnisotropy = 16;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	device->CreateSamplerState(&samplerDesc, samplerOptions.begin()->second.GetAddressOf());

	//(0) Create White Material
	materials.push_back(std::make_shared<Material>(DirectX::XMFLOAT4(1, 1, 1, 1), 0.051f, vertexShader, pixelShaders[0]));
	
	//(0) Create Red Material
	materials.push_back(std::make_shared<Material>(DirectX::XMFLOAT4(1, 0, 0, 1), 0.0f, vertexShader, pixelShaders[0]));

	//(1) Create Green Material
	materials.push_back(std::make_shared<Material>(DirectX::XMFLOAT4(0, 1, 0, 1), 0.0f, vertexShader, pixelShaders[0]));

	//(2) Create Blue Material
	materials.push_back(std::make_shared<Material>(DirectX::XMFLOAT4(0, 0, 1, 1), 0.0f, vertexShader, pixelShaders[0]));

	//(3) Create Material with CustomPS
	materials.push_back(std::make_shared<Material>(DirectX::XMFLOAT4(1, 1, 1, 1), 1.0f, vertexShader, pixelShaders[1]));

	//Lights
	//ambientColor = DirectX::XMFLOAT3(0.1f, 0.1f, 0.25f);
	ambientColor = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);

	//Red light pointing right
	directionalLight1 = {};
	directionalLight1.Type = LIGHT_TYPE_DIRECTIONAL;
	directionalLight1.Direction = DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f);
	//directionalLight1.Color = DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f);
	directionalLight1.Color = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
	directionalLight1.Intensity = 1.0f;

	//Yellow light pointing left and down
	directionalLight2 = {};
	directionalLight2.Type = LIGHT_TYPE_DIRECTIONAL;
	directionalLight2.Direction = DirectX::XMFLOAT3(-1.0f, -1.0f, 0.0f);
	directionalLight1.Color = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
	//directionalLight2.Color = DirectX::XMFLOAT3(1.0f, 1.0f, 0.0f);
	directionalLight2.Intensity = 1.0f;

	//Teal light pointing up and left and forwards
	directionalLight3 = {};
	directionalLight3.Type = LIGHT_TYPE_DIRECTIONAL;
	directionalLight3.Direction = DirectX::XMFLOAT3(-1.0f, 1.0f, 1.0f);
	//directionalLight3.Color = DirectX::XMFLOAT3(0.0f, 1.0f, 1.0f);
	directionalLight1.Color = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
	directionalLight3.Intensity = 1.0f;

	//Purple point light
	pointLight1 = {};
	pointLight1.Type = LIGHT_TYPE_POINT;
	pointLight1.Range = 10.0f;
	pointLight1.Position = DirectX::XMFLOAT3(-3.0f, 1.0f, -5.0f);
	//pointLight1.Color = DirectX::XMFLOAT3(1.0f, 0.0f, 1.0f);
	directionalLight1.Color = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
	pointLight1.Intensity = 1.0f;

	//Green point light
	pointLight2 = {};
	pointLight2.Type = LIGHT_TYPE_POINT;
	pointLight2.Range = 10.0f;
	pointLight2.Position = DirectX::XMFLOAT3(0.0f, 5.0f, 0.0f);
	//pointLight2.Color = DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f);
	directionalLight1.Color = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
	pointLight2.Intensity = 1.0f;

	//Add lights to lights vector
	lights.push_back(&directionalLight1);
	lights.push_back(&directionalLight2);
	lights.push_back(&directionalLight3);
	lights.push_back(&pointLight1);
	lights.push_back(&pointLight2);

	CreateGeometry();

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
	vertexShader = std::make_shared<SimpleVertexShader>(device, context,
		FixPath(L"VertexShader.cso").c_str());

	pixelShaders.push_back(std::make_shared<SimplePixelShader>(device, context,
		FixPath(L"PixelShader.cso").c_str()));

	pixelShaders.push_back(std::make_shared<SimplePixelShader>(device, context,
		FixPath(L"CustomPS.cso").c_str()));
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

	//(1) Cylinder Mesh
	meshes.push_back(std::make_shared<Mesh>(FixPath(L"../../Assets/Models/cylinder.obj").c_str(), device, context));

	//(2) Helix Mesh
	meshes.push_back(std::make_shared<Mesh>(FixPath(L"../../Assets/Models/helix.obj").c_str(), device, context));

	//(3) Quad Mesh
	meshes.push_back(std::make_shared<Mesh>(FixPath(L"../../Assets/Models/quad.obj").c_str(), device, context));

	//(4) Double Sided Quad Mesh
	meshes.push_back(std::make_shared<Mesh>(FixPath(L"../../Assets/Models/quad_double_sided.obj").c_str(), device, context));

	//(5) Sphere Mesh
	meshes.push_back(std::make_shared<Mesh>(FixPath(L"../../Assets/Models/sphere.obj").c_str(), device, context));

	//(6) Torus Mesh
	meshes.push_back(std::make_shared<Mesh>(FixPath(L"../../Assets/Models/torus.obj").c_str(), device, context));

	//Create entities using the meshes and materials
	//(0) Cube Entity
	entities.push_back(std::make_shared<Entity>(meshes[0], materials[0]));
	entities[0]->GetTransform()->SetPosition(-9.0f, 0.0f, 0.0f);

	//(1) Cylinder Entity
	entities.push_back(std::make_shared<Entity>(meshes[1], materials[0]));
	entities[1]->GetTransform()->SetPosition(-6.0f, 0.0f, 0.0f);

	//(2) Helix Entity
	entities.push_back(std::make_shared<Entity>(meshes[2], materials[0]));
	entities[2]->GetTransform()->SetPosition(-3.0f, 0.0f, 0.0f);

	//(3) Quad Entity
	entities.push_back(std::make_shared<Entity>(meshes[3], materials[0]));
	entities[3]->GetTransform()->SetPosition(0.0f, 0.0f, 0.0f);

	//(4) Double Sided Quad Entity
	entities.push_back(std::make_shared<Entity>(meshes[4], materials[0]));
	entities[4]->GetTransform()->SetPosition(3.0f, 0.0f, 0.0f);

	//(5) Sphere Entity
	entities.push_back(std::make_shared<Entity>(meshes[5], materials[0]));
	entities[5]->GetTransform()->SetPosition(6.0f, 0.0f, 0.0f);

	//(6) Torus Entity
	entities.push_back(std::make_shared<Entity>(meshes[6], materials[0]));
	entities[6]->GetTransform()->SetPosition(9.0f, 0.0f, 0.0f);

}


// --------------------------------------------------------
// Handle resizing to match the new window size.
//  - DXCore needs to resize the back buffer
//  - Eventually, we'll want to update our 3D camera
// --------------------------------------------------------
void Game::OnResize() {
	// Handle base-level DX resize stuff
	DXCore::OnResize();

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
	/*entities[0]->GetTransform()->SetScale((sin(totalTime) + 2.0f) / 2.0f, (sin(totalTime) + 2.0f) / 2.0f, 0.0f);
	entities[1]->GetTransform()->SetPosition(sin(totalTime), 0.0f, 0.0f);
	entities[2]->GetTransform()->Rotate(0.0f, 0.0f, 1.0f * deltaTime);
	entities[4]->GetTransform()->SetPosition(0.0f, sin(totalTime), 0.0f);*/

	for (std::shared_ptr<Entity> entity : entities) {
		entity->GetTransform()->Rotate(0.0f, 0.25f * deltaTime, 0.0f);
	}

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
		ImGui::DragFloat3("Ambient Light", &ambientColor.x, 0.005f);

		int index = 0;

		//Loop through each light and make a node for it with child properties
		for (Light* light : lights) {
			if (ImGui::TreeNode((void*)(intptr_t)index, "Light %d", index)) {
				ImGui::Text("Type: %s", (light->Type == 0 ? "Directional" : (light->Type == 1 ? "Point" : "Spot")));
				ImGui::DragFloat3("Direction", &light->Direction.x, 0.005f);
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

	ImGui::End();
}

// --------------------------------------------------------
// Clear the screen, redraw everything, present to the user
// --------------------------------------------------------
void Game::Draw(float deltaTime, float totalTime) {
	// Frame START
	// - These things should happen ONCE PER FRAME
	// - At the beginning of Game::Draw() before drawing *anything*
	{
		// Clear the back buffer (erases what's on the screen)
		const float bgColor[4] = { 0.4f, 0.6f, 0.75f, 1.0f }; // Cornflower Blue
		context->ClearRenderTargetView(backBufferRTV.Get(), bgColor);

		// Clear the depth buffer (resets per-pixel occlusion information)
		context->ClearDepthStencilView(depthBufferDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
	}

	//Loop through the entity vector and draw the entities
	for (std::shared_ptr<Entity> entity : entities) {
		entity->GetMaterial()->GetPixelShader()->SetFloat3("ambient", ambientColor);

		entity->GetMaterial()->GetPixelShader()->SetData("lights", lights[0], sizeof(Light) * (int)lights.size());

		/*entity->GetMaterial()->GetPixelShader()->SetShaderResourceView("SurfaceTexture", textureSRV);
		entity->GetMaterial()->GetPixelShader()->SetSamplerState("BasicSampler", samplerOptions);*/
		entity->GetMaterial()->PrepareMaterial(textureSRVs, samplerOptions);

		entity->Draw(context, cameras[selectedCameraIndex], totalTime);
		
	}

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
}