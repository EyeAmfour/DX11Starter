#include "Game.h"
#include "Vertex.h"
#include "Input.h"
#include "Helpers.h"
#include "BufferStructs.h"

//ImGui imports
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_dx11.h"
#include "ImGui/imgui_impl_win32.h"

// Needed for a helper function to load pre-compiled shader files
#pragma comment(lib, "d3dcompiler.lib")
#include <d3dcompiler.h>

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
	CreateGeometry();

	//Create cameras
	cameras.push_back(std::make_shared<Camera>(
		0.0f, 0.0f, -5.0f,
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

	//Set initial selected camera
	selectedCameraIndex = 0;

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
		context->IASetInputLayout(inputLayout.Get());

		// Set the active vertex and pixel shaders
		//  - Once you start applying different shaders to different objects,
		//    these calls will need to happen multiple times per frame
		context->VSSetShader(vertexShader.Get(), 0, 0);
		context->PSSetShader(pixelShader.Get(), 0, 0);
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
	
	// Get size as the next multiple of 16 (instead of hardcoding a size here!)
	unsigned int size = sizeof(VertexShaderExternalData);
	size = (size + 15) / 16 * 16; // This will work even if the struct size changes

	// Describe the constant buffer
	D3D11_BUFFER_DESC cbDesc = {}; // Sets struct to all zeros
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbDesc.ByteWidth = size; // Must be a multiple of 16
	cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbDesc.Usage = D3D11_USAGE_DYNAMIC;

	device->CreateBuffer(&cbDesc, 0, vsConstantBuffer.GetAddressOf());
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
	// BLOBs (or Binary Large OBjects) for reading raw data from external files
	// - This is a simplified way of handling big chunks of external data
	// - Literally just a big array of bytes read from a file
	ID3DBlob* pixelShaderBlob;
	ID3DBlob* vertexShaderBlob;

	// Loading shaders
	//  - Visual Studio will compile our shaders at build time
	//  - They are saved as .cso (Compiled Shader Object) files
	//  - We need to load them when the application starts
	{
		// Read our compiled shader code files into blobs
		// - Essentially just "open the file and plop its contents here"
		// - Uses the custom FixPath() helper from Helpers.h to ensure relative paths
		// - Note the "L" before the string - this tells the compiler the string uses wide characters
		D3DReadFileToBlob(FixPath(L"PixelShader.cso").c_str(), &pixelShaderBlob);
		D3DReadFileToBlob(FixPath(L"VertexShader.cso").c_str(), &vertexShaderBlob);

		// Create the actual Direct3D shaders on the GPU
		device->CreatePixelShader(
			pixelShaderBlob->GetBufferPointer(),	// Pointer to blob's contents
			pixelShaderBlob->GetBufferSize(),		// How big is that data?
			0,										// No classes in this shader
			pixelShader.GetAddressOf());			// Address of the ID3D11PixelShader pointer

		device->CreateVertexShader(
			vertexShaderBlob->GetBufferPointer(),	// Get a pointer to the blob's contents
			vertexShaderBlob->GetBufferSize(),		// How big is that data?
			0,										// No classes in this shader
			vertexShader.GetAddressOf());			// The address of the ID3D11VertexShader pointer
	}

	// Create an input layout 
	//  - This describes the layout of data sent to a vertex shader
	//  - In other words, it describes how to interpret data (numbers) in a vertex buffer
	//  - Doing this NOW because it requires a vertex shader's byte code to verify against!
	//  - Luckily, we already have that loaded (the vertex shader blob above)
	{
		D3D11_INPUT_ELEMENT_DESC inputElements[2] = {};

		// Set up the first element - a position, which is 3 float values
		inputElements[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;				// Most formats are described as color channels; really it just means "Three 32-bit floats"
		inputElements[0].SemanticName = "POSITION";							// This is "POSITION" - needs to match the semantics in our vertex shader input!
		inputElements[0].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;	// How far into the vertex is this?  Assume it's after the previous element

		// Set up the second element - a color, which is 4 more float values
		inputElements[1].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;			// 4x 32-bit floats
		inputElements[1].SemanticName = "COLOR";							// Match our vertex shader input!
		inputElements[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;	// After the previous element

		// Create the input layout, verifying our description against actual shader code
		device->CreateInputLayout(
			inputElements,							// An array of descriptions
			2,										// How many elements in that array?
			vertexShaderBlob->GetBufferPointer(),	// Pointer to the code of a shader that uses this layout
			vertexShaderBlob->GetBufferSize(),		// Size of the shader code that uses this layout
			inputLayout.GetAddressOf());			// Address of the resulting ID3D11InputLayout pointer
	}
}



// --------------------------------------------------------
// Creates the geometry we're going to draw - a single triangle for now
// --------------------------------------------------------
void Game::CreateGeometry() {
	// Create some temporary variables to represent colors
	// - Not necessary, just makes things more readable
	XMFLOAT4 red = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
	XMFLOAT4 green = XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);
	XMFLOAT4 blue = XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);
	XMFLOAT4 yellow = XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f);

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
	
	//Triangle
	Vertex triangleVerts[] = {
		{ XMFLOAT3(+0.0f, +0.5f, +0.0f), red },
		{ XMFLOAT3(+0.5f, -0.5f, +0.0f), blue },
		{ XMFLOAT3(-0.5f, -0.5f, +0.0f), green },
	};

	//Square
	Vertex squareVerts[] = {
		{ XMFLOAT3(-0.75f, +0.75f, +0.0f), red }, //top left (0)
		{ XMFLOAT3(-0.75f, +0.25f, +0.0f), green }, //bottom left (1)
		{ XMFLOAT3(-0.5f, +0.75f, +0.0f), blue }, //top right (2)
		{ XMFLOAT3(-0.5f, +0.25f, +0.0f), red }, //bottom right (3)
	};

	//Star
	Vertex starVerts[] =
	{
		{ XMFLOAT3(+0.5f, +0.5f, +0.0f), yellow }, //(0)
		{ XMFLOAT3(+0.65f, +0.5f, +0.0f), yellow }, //(1)
		{ XMFLOAT3(+0.7f, +0.7f, +0.0f), yellow }, //(2)
		{ XMFLOAT3(+0.75f, +0.5f, +0.0f), yellow }, //(3)
		{ XMFLOAT3(+0.9f, +0.5f, +0.0f), yellow }, //(4)
		{ XMFLOAT3(+0.775f, +0.4f, +0.0f), yellow }, //(5)
		{ XMFLOAT3(+0.825f, +0.2f, +0.0f), yellow }, //(6)
		{ XMFLOAT3(+0.7f, +0.3f, +0.0f), yellow }, //(7)
		{ XMFLOAT3(+0.575f, +0.2f, +0.0f), yellow }, //(8)
		{ XMFLOAT3(+0.625f, +0.4f, +0.0f), yellow }, //(9)
	};

	// Set up indices, which tell us which vertices to use and in which order
	// - This is redundant for just 3 vertices, but will be more useful later
	// - Indices are technically not required if the vertices are in the buffer 
	//    in the correct order and each one will be used exactly once
	// - But just to see how it's done...

	//Triangle
	unsigned int triangleIndices[] = {
		0, 1, 2,
	};
	
	//Square
	unsigned int squareIndices[] = {
		0, 2, 3,
		0, 3, 1,
	};

	//Star
	unsigned int starIndices[] = {	
		0, 1, 9,
		1, 2, 3,
		3, 4, 5,
		5, 6, 7,
		7, 8, 9,
		1, 3, 9,
		3, 7, 9,
		3, 5, 7
	};

	//Create Mesh smart pointers in the meshes vector
	//Add the Triangle Mesh
	meshes.push_back(
		std::make_shared<Mesh>(
			triangleVerts,
			sizeof(triangleVerts) / sizeof(Vertex),
			triangleIndices,
			sizeof(triangleIndices) / sizeof(unsigned int),
			device,
			context
		)
	);

	//Add the Square Mesh
	meshes.push_back(
		std::make_shared<Mesh>(
			squareVerts,
			sizeof(squareVerts) / sizeof(Vertex),
			squareIndices,
			sizeof(squareIndices) / sizeof(unsigned int),
			device,
			context
		)
	);

	//Add the Star Mesh
	meshes.push_back(
		std::make_shared<Mesh>(
			starVerts,
			sizeof(starVerts) / sizeof(Vertex),
			starIndices,
			sizeof(starIndices) / sizeof(unsigned int),
			device,
			context
		)
	);

	//Create entities
	entities.push_back(std::make_shared<Entity>(meshes[0]));
	entities.push_back(std::make_shared<Entity>(meshes[1]));
	entities.push_back(std::make_shared<Entity>(meshes[1]));
	entities.push_back(std::make_shared<Entity>(meshes[2]));
	entities.push_back(std::make_shared<Entity>(meshes[2]));
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
	entities[0]->GetTransform()->SetScale((sin(totalTime) + 2.0f) / 2.0f, (sin(totalTime) + 2.0f) / 2.0f, 0.0f);
	entities[1]->GetTransform()->SetPosition(sin(totalTime), 0.0f, 0.0f);
	entities[2]->GetTransform()->Rotate(0.0f, 0.0f, 1.0f * deltaTime);
	entities[4]->GetTransform()->SetPosition(0.0f, sin(totalTime), 0.0f);

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
		//Loop through each mesh and made a node for it with child properties
		for (std::shared_ptr<Entity> entity : entities) {
			if (ImGui::TreeNode((void*)(intptr_t)index, "Entity %d (%d indices)", index, entity->GetMesh()->GetIndexCount())) {
				auto entityPosition = entity->GetTransform()->GetPosition();
				auto entityRotation = entity->GetTransform()->GetPitchYawRoll();
				auto entityScale = entity->GetTransform()->GetScale();

				//Mesh Color
				ImGui::ColorEdit4("Tint", &entity->GetMesh()->meshTint.x);

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

		//Create previous camera button
		if (ImGui::Button("Select Previous Camera")) {
			//Check for wrapping to last camera in list.
			//Otherwise, just go to the previous camera
			if (selectedCameraIndex - 1 < 0) {
				selectedCameraIndex = cameras.size() - 1;
			} else {
				selectedCameraIndex--;
			}
		}

		//Create next camera button
		if (ImGui::Button("Select Next Camera")) {
			//Check for wrapping to first camera in list.
			//Otherwise, just go to the next camera
			if (selectedCameraIndex + 1 >= cameras.size()) {
				selectedCameraIndex = 0;
			} else {
				selectedCameraIndex++;
			}
		}

		int index = 0;

		//Loop through each mesh and made a node for it with child properties
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
		entity->Draw(context, vsConstantBuffer, cameras[selectedCameraIndex]);
		
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