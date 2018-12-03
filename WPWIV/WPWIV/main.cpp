#include <windows.h>
#include <wincodec.h>
#include <dinput.h>
#include "Renderer.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx12.h"
#include "Scene.h"

HWND hwnd = NULL;// Handle to the window
LPCTSTR WindowName = L"WPWIV";// name of the window (not the title)
LPCTSTR WindowTitle = L"WPWIV_1.0";// title of the window
int Width = 1024;// width and height of the window
int Height = 768;
IDirectInputDevice8* DIKeyboard;
IDirectInputDevice8* DIMouse;
DIMOUSESTATE mouseLastState;
BYTE keyboardLastState[256];
bool mouseAcquired = false;
LPDIRECTINPUT8 DirectInput;
bool FullScreen = false; // is window full screen?
bool Running = true; // we will exit the program when this becomes false
IDXGIFactory4* dxgiFactory;
ID3D12Device* device; // direct3d device
IDXGISwapChain3* swapChain; // swapchain used to switch between render targets
ID3D12CommandQueue* commandQueue; // container for command lists
ID3D12CommandAllocator* commandAllocator[FrameBufferCount]; // we want enough allocators for each buffer * number of threads (we only have one thread)
ID3D12GraphicsCommandList* commandList; // a command list we can record commands into, then execute them to render the frame
ID3D12Fence* fence[FrameBufferCount];    
HANDLE fenceEvent; // a handle to an event when our fence is unlocked by the gpu
UINT64 fenceValue[FrameBufferCount]; // this value is incremented each frame. each fence will have its own value
int frameIndex; // current rtv we are on
uint32_t frameCount = 0;

Renderer mRenderer;
Scene mScene;
Frame mFrameGraphics;
Frame mFrameWaveParticle;
Frame mFramePostProcess;
OrbitCamera mCamera(4.f, 0.f, 0.f, XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), (float)Width, (float)Height, 45.0f, 0.1f, 1000.0f);
OrbitCamera mCameraRenderTexture(4.f, 0.f, 0.f, XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), 1000, 1000, 45.0f, 0.1f, 1000.0f);
Camera mDummyCamera(XMFLOAT3{ 4.f,0,0 }, XMFLOAT3{ 0,0,0 }, XMFLOAT3{ 0,1,0 }, 500, 500, 45.0f, 0.1f, 1000.f);
Mesh mPlane(Mesh::MeshType::Plane, XMFLOAT3(0, 0, 0), XMFLOAT3(0, 0, 0), XMFLOAT3(1, 1, 1));
Mesh mWaterSurface(Mesh::MeshType::WaterSurface, 100, 100, XMFLOAT3(-5, 0, -5), XMFLOAT3(0, 0, 0), XMFLOAT3(10, 1, 10));
Mesh mQuad(Mesh::MeshType::FullScreenQuad, XMFLOAT3(0, 0, 0), XMFLOAT3(0, 0, 0), XMFLOAT3(1, 1, 1));
Mesh mWaveParticle(Mesh::MeshType::WaveParticle, 100, XMFLOAT3(0, 0, 0), XMFLOAT3(0, 0, 0), XMFLOAT3(1, 1, 1));
Shader mVertexShader(Shader::ShaderType::VertexShader, L"VertexShader.hlsl");
Shader mHullShader(Shader::ShaderType::HullShader, L"HullShader.hlsl");
Shader mDomainShader(Shader::ShaderType::DomainShader, L"DomainShader.hlsl");
Shader mPixelShader(Shader::ShaderType::PixelShader, L"PixelShader.hlsl");
Shader mPostProcessVS(Shader::ShaderType::VertexShader, L"PostProcessVS.hlsl");
Shader mPostProcessPS_H(Shader::ShaderType::PixelShader, L"PostProcessPS_H.hlsl");
Shader mPostProcessPS_V(Shader::ShaderType::PixelShader, L"PostProcessPS_V.hlsl");
Shader mWaveParticleVS(Shader::ShaderType::VertexShader, L"WaveParticleVS.hlsl");
Shader mWaveParticlePS(Shader::ShaderType::PixelShader, L"WaveParticlePS.hlsl");
Texture mTextureFlowmap(L"rd.jpg");
Texture mTextureAlbedo(L"checkerboard.jpg");
RenderTexture mRenderTextureWaveParticle(500, 500);
RenderTexture mRenderTexturePostProcessH1(500, 500);
RenderTexture mRenderTexturePostProcessH2(500, 500);
RenderTexture mRenderTexturePostProcessV1(500, 500);
RenderTexture mRenderTexturePostProcessV2(500, 500);

//imgui stuff
ID3D12DescriptorHeap* g_pd3dSrvDescHeap = NULL;
bool show_demo_window = true;
bool show_another_window = false;
ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

bool CreateScene()
{
	// the adding order of textures is the register number

	mFrameWaveParticle.AddCamera(&mDummyCamera);
	mFrameWaveParticle.AddMesh(&mWaveParticle);
	mFrameWaveParticle.AddRenderTexture(&mRenderTextureWaveParticle);

	mFramePostProcess.AddCamera(&mDummyCamera);
	mFramePostProcess.AddMesh(&mQuad);
	mFramePostProcess.AddTexture(&mRenderTextureWaveParticle);
	mFramePostProcess.AddTexture(&mRenderTexturePostProcessH1);
	mFramePostProcess.AddTexture(&mRenderTexturePostProcessH2);
	mFramePostProcess.AddRenderTexture(&mRenderTexturePostProcessH1);
	mFramePostProcess.AddRenderTexture(&mRenderTexturePostProcessH2);
	mFramePostProcess.AddRenderTexture(&mRenderTexturePostProcessV1);
	mFramePostProcess.AddRenderTexture(&mRenderTexturePostProcessV2);

	mFrameGraphics.AddCamera(&mCamera);
	mFrameGraphics.AddMesh(&mWaterSurface);// (&mPlane);
	mFrameGraphics.AddTexture(&mTextureAlbedo);
	mFrameGraphics.AddTexture(&mTextureFlowmap);
	mFrameGraphics.AddTexture(&mRenderTexturePostProcessV1);
	mFrameGraphics.AddTexture(&mRenderTexturePostProcessV2);

	mFrameGraphics.SetUniformTime(0);
	mFramePostProcess.SetUniformTime(0);
	mFrameWaveParticle.SetUniformTime(0);

	mScene.AddFrame(&mFrameGraphics);
	mScene.AddFrame(&mFrameWaveParticle);
	mScene.AddFrame(&mFramePostProcess);
	mScene.AddCamera(&mCamera);
	mScene.AddCamera(&mCameraRenderTexture);
	mScene.AddCamera(&mDummyCamera);
	mScene.AddMesh(&mPlane);
	mScene.AddMesh(&mQuad);
	mScene.AddMesh(&mWaveParticle);
	mScene.AddMesh(&mWaterSurface);
	mScene.AddShader(&mVertexShader);
	mScene.AddShader(&mHullShader);
	mScene.AddShader(&mDomainShader);
	mScene.AddShader(&mPixelShader);
	mScene.AddShader(&mPostProcessVS);
	mScene.AddShader(&mPostProcessPS_H);
	mScene.AddShader(&mPostProcessPS_V);
	mScene.AddShader(&mWaveParticleVS);
	mScene.AddShader(&mWaveParticlePS);
	mScene.AddTexture(&mTextureAlbedo);
	mScene.AddTexture(&mTextureFlowmap);
	mScene.AddRenderTexture(&mRenderTextureWaveParticle);
	mScene.AddRenderTexture(&mRenderTexturePostProcessH1);
	mScene.AddRenderTexture(&mRenderTexturePostProcessH2);
	mScene.AddRenderTexture(&mRenderTexturePostProcessV1);
	mScene.AddRenderTexture(&mRenderTexturePostProcessV2);

	mScene.SetUniformEdgeTessFactor(4);
	mScene.SetUniformInsideTessFactor(2);
	mScene.SetUniformHeightScale(1.3);
	mScene.SetUniformWaveParticleSpeedScale(0.0001);
	mScene.SetUniformFlowSpeed(0.0001);
	mScene.SetUniformTexutureWidthHeight(500, 500);
	mScene.SetUniformBlurRadius(65);
	mScene.SetUniformDxScale(0.03);
	mScene.SetUniformDzScale(0.03);
	mScene.SetUniformTimeScale(6);
	mScene.SetUniformMode(8);
	mScene.SetUniformLighthight(6.35);
	mScene.Setextinctcoeff(-0.4);
	mScene.Setshiness(240);

	if (!mScene.LoadScene())
		return false;
	
	return true;
}

bool InitScene()
{
	return mScene.InitScene(device);
}

void InitConsole()
{
	AllocConsole();
	AttachConsole(GetCurrentProcessId());
	FILE* stream;
	freopen_s(&stream, "CON", "w", stdout);
}

bool InitDirectInput(HINSTANCE hInstance)
{
	HRESULT hr;

	hr = DirectInput8Create(hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&DirectInput, NULL);
	if (!CheckError(hr, nullptr)) return false;

	hr = DirectInput->CreateDevice(GUID_SysKeyboard, &DIKeyboard, NULL);
	if (!CheckError(hr, nullptr)) return false;

	hr = DirectInput->CreateDevice(GUID_SysMouse, &DIMouse, NULL);
	if (!CheckError(hr, nullptr)) return false;

	hr = DIKeyboard->SetDataFormat(&c_dfDIKeyboard);
	if (!CheckError(hr, nullptr)) return false;

	hr = DIKeyboard->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
	if (!CheckError(hr, nullptr)) return false;

	hr = DIMouse->SetDataFormat(&c_dfDIMouse);
	if (!CheckError(hr, nullptr)) return false;

	hr = DIMouse->SetCooperativeLevel(hwnd, DISCL_EXCLUSIVE | DISCL_NOWINKEY | DISCL_FOREGROUND);
	if (!CheckError(hr, nullptr)) return false;

	return true;
}

void DetectInput()
{
	BYTE keyboardCurrState[256];

	DIKeyboard->Acquire();
	
	DIKeyboard->GetDeviceState(sizeof(keyboardCurrState), (LPVOID)&keyboardCurrState);

	//keyboard control
	if (KEYDOWN(keyboardCurrState, DIK_ESCAPE))
	{
		PostMessage(hwnd, WM_DESTROY, 0, 0);
	}

	//mouse control
	if (KEYDOWN(keyboardCurrState, DIK_C))//control camera
	{
		if (!mouseAcquired)
		{
			DIMouse->Acquire();
			mouseAcquired = true;
		}

	}
	else
	{
		DIMouse->Unacquire();
		mouseAcquired = false;
	}

	if (mouseAcquired)
	{
		DIMOUSESTATE mouseCurrState;
		DIMouse->GetDeviceState(sizeof(DIMOUSESTATE), &mouseCurrState);
		if (mouseCurrState.lX != 0)
		{
			mCamera.SetHorizontalAngle(mCamera.GetHorizontalAngle() + mouseCurrState.lX * 0.1);
		}
		if (mouseCurrState.lY != 0)
		{
			float tempVerticalAngle = mCamera.GetVerticalAngle() + mouseCurrState.lY * 0.1;
			if (tempVerticalAngle > 90 - EPSILON) tempVerticalAngle = 89 - EPSILON;
			if (tempVerticalAngle < -90 + EPSILON) tempVerticalAngle = -89 + EPSILON;
			mCamera.SetVerticalAngle(tempVerticalAngle);
		}
		if (mouseCurrState.lZ != 0)
		{
			float tempDistance = mCamera.GetDistance() - mouseCurrState.lZ * 0.01;
			if (tempDistance < 0 + EPSILON) tempDistance = 0.1 + EPSILON;
			mCamera.SetDistance(tempDistance);
		}
		mouseLastState = mouseCurrState;
		mCamera.UpdateUniform();
		mCamera.UpdateUniformBuffer();
	}

	memcpy(keyboardLastState, keyboardCurrState, 256 * sizeof(BYTE));

	return;
}

bool InitDevice()
{
	HRESULT hr;

	// -- Create the Device -- //

	hr = CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory));
	if (FAILED(hr))
	{
		return false;
	}

	IDXGIAdapter1* adapter; // adapters are the graphics card (this includes the embedded graphics on the motherboard)

	int adapterIndex = 0; // we'll start looking for directx 12  compatible graphics devices starting at index 0

	bool adapterFound = false; // set this to true when a good one was found

							   // find first hardware gpu that supports d3d 12
	while (dxgiFactory->EnumAdapters1(adapterIndex, &adapter) != DXGI_ERROR_NOT_FOUND)
	{
		DXGI_ADAPTER_DESC1 desc;
		adapter->GetDesc1(&desc);

		if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
		{
			// we dont want a software device
			continue;
		}

		// we want a device that is compatible with direct3d 12 (feature level 11 or higher)
		hr = D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr);
		if (SUCCEEDED(hr))
		{
			adapterFound = true;
			break;
		}

		adapterIndex++;
	}

	if (!adapterFound)
	{
		Running = false;
		return false;
	}

	// Create the device
	hr = D3D12CreateDevice(
		adapter,
		D3D_FEATURE_LEVEL_11_0,
		IID_PPV_ARGS(&device)
	);
	if (FAILED(hr))
	{
		Running = false;
		return false;
	}

	return true;
}

bool InitCommandQueue()
{
	HRESULT hr;

	// -- Create a direct command queue -- //

	D3D12_COMMAND_QUEUE_DESC cqDesc = {};
	cqDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	cqDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT; // direct means the gpu can directly execute this command queue

	hr = device->CreateCommandQueue(&cqDesc, IID_PPV_ARGS(&commandQueue)); // create the command queue
	if (FAILED(hr))
	{
		Running = false;
		return false;
	}

	return true;
}

bool InitSwapChain()
{
	HRESULT hr;
	// -- Create the Swap Chain (double/tripple buffering) -- //

	DXGI_MODE_DESC backBufferDesc = {}; // this is to describe our display mode
	backBufferDesc.Width = Width; // buffer width
	backBufferDesc.Height = Height; // buffer height
	backBufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // format of the buffer (rgba 32 bits, 8 bits for each chanel)

														// describe our multi-sampling. We are not multi-sampling, so we set the count to 1 (we need at least one sample of course)

	DXGI_SAMPLE_DESC sampleDesc = {};//this way the first member variable will be initialized
	sampleDesc.Count = MultiSampleCount; // multisample count (no multisampling, so we just put 1, since we still need 1 sample)

						  // Describe and create the swap chain.
	DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
	swapChainDesc.BufferCount = FrameBufferCount; // number of buffers we have
	swapChainDesc.BufferDesc = backBufferDesc; // our back buffer description
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; // this says the pipeline will render to this swap chain
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; // dxgi will discard the buffer (data) after we call present
	swapChainDesc.OutputWindow = hwnd; // handle to our window
	swapChainDesc.SampleDesc = sampleDesc; // our multi-sampling description
	swapChainDesc.Windowed = !FullScreen; // set to true, then if in fullscreen must call SetFullScreenState with true for full screen to get uncapped fps

	IDXGISwapChain* tempSwapChain;

	hr = dxgiFactory->CreateSwapChain(
		commandQueue, // the queue will be flushed once the swap chain is created
		&swapChainDesc, // give it the swap chain description we created above
		&tempSwapChain // store the created swap chain in a temp IDXGISwapChain interface
	);
	if (FAILED(hr))
	{
		CheckError(hr);
		Running = false;
		return false;
	}

	swapChain = static_cast<IDXGISwapChain3*>(tempSwapChain);

	frameIndex = swapChain->GetCurrentBackBufferIndex();

	return true;
}

bool InitCommandList()
{

	HRESULT hr;

	// -- Create the Command Allocators -- //

	for (int i = 0; i < FrameBufferCount; i++)
	{
		hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator[i]));
		if (FAILED(hr))
		{
			Running = false;
			return false;
		}
	}

	// -- Create a Command List -- //

	// create the command list with the first allocator
	hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator[frameIndex], NULL, IID_PPV_ARGS(&commandList));
	if (FAILED(hr))
	{
		Running = false;
		return false;
	}

	return true;
}

bool InitFence()
{

	HRESULT hr;

	// -- Create a Fence & Fence Event -- //

	// create the fences
	for (int i = 0; i < FrameBufferCount; i++)
	{
		hr = device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence[i]));
		if (FAILED(hr))
		{
			Running = false;
			return false;
		}
		fenceValue[i] = 0; // set the initial fence value to 0
	}

	// create a handle to a fence event
	fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	if (fenceEvent == nullptr)
	{
		Running = false;
		return false;
	}

	return true;
}

bool FlushCommand()
{
	HRESULT hr;
	// Now we execute the command list to upload the initial assets (triangle data)
	commandList->Close();
	ID3D12CommandList* ppCommandLists[] = { commandList };
	commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	// increment the fence value now, otherwise the buffer might not be uploaded by the time we start drawing
	fenceValue[frameIndex]++;
	hr = commandQueue->Signal(fence[frameIndex], fenceValue[frameIndex]);
	if (FAILED(hr))
	{
		Running = false;
		return false;
	}

	return true;
}

bool InitImgui()
{
	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	desc.NumDescriptors = 1;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	if (device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&g_pd3dSrvDescHeap)) != S_OK)
		return false;

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls

	// Setup Platform/Renderer bindings
	ImGui_ImplWin32_Init(hwnd);
	ImGui_ImplDX12_Init(device, FrameBufferCount,
		DXGI_FORMAT_R8G8B8A8_UNORM,
		g_pd3dSrvDescHeap->GetCPUDescriptorHandleForHeapStart(),
		g_pd3dSrvDescHeap->GetGPUDescriptorHandleForHeapStart());

	// Setup Style
	// ImGui::StyleColorsDark();
	ImGui::StyleColorsClassic();

	// Load Fonts
	// - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them. 
	// - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple. 
	// - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
	// - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
	// - Read 'misc/fonts/README.txt' for more instructions and details.
	// - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
	//io.Fonts->AddFontDefault();
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
	//ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
	//IM_ASSERT(font != NULL);
	return true;
}

bool InitD3D()
{
	// Create system level dx12 context

	if (!InitDevice())
	{
		printf("InitDevice failed\n");
		return false;
	}

	if (!InitCommandQueue())
	{
		printf("InitCommandQueue failed\n");
		return false;
	}

	if (!InitSwapChain())
	{
		printf("InitSwapChain failed\n");
		return false;
	}

	if (!InitCommandList())
	{
		printf("InitCommandList failed\n");
		return false;
	}

	if (!InitFence())
	{
		printf("InitFence failed\n");
		return false;
	}

	// GPU side, upload data to GPU
	if (!InitScene())
	{
		printf("InitScene failed\n");
		return false;
	}

	// GPU side, create GPU pipeline
	/////////////////////////////////////////////////////////
	if (!mRenderer.CreateRenderer(device, swapChain, Width, Height))
	{
		printf("CreateRenderer failed\n");
		return false;
	}

	// wave particle
	if (!mRenderer.CreateWaveParticlePipeline(
		device,
		mFrameWaveParticle.GetTextureVec(),
		mFrameWaveParticle.GetRenderTextureVec()))
	{
		printf("CreateWaveParticlePipeline failed\n");
		return false;
	}

	if (!mRenderer.CreatePSO(
		device,
		mRenderer.GetWaveParticlePsoPtr(0),
		mRenderer.GetWaveParticleRootSignature(),
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT,
		Renderer::AdditiveBlend(),
		Renderer::NoDepthTest(),
		DXGI_FORMAT_R16G16B16A16_FLOAT,
		1,
		&mWaveParticleVS,
		nullptr,
		nullptr,
		nullptr,
		&mWaveParticlePS))
	{
		printf("CreateWaveParticlePipeline PSO failed\n");
		return false;
	}

	// post process
	if (!mRenderer.CreatePostProcessPipeline(
		device,
		mFramePostProcess.GetTextureVec(),
		mFramePostProcess.GetRenderTextureVec()))
	{
		printf("CreatePostProcessPipeline failed\n");
		return false;
	}

	if (!mRenderer.CreatePSO(
		device,
		mRenderer.GetPostProcessPsoPtr(0),
		mRenderer.GetPostProcessRootSignature(),
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
		CD3DX12_BLEND_DESC(D3D12_DEFAULT),//Renderer::AdditiveBlend(),
		CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT),
		DXGI_FORMAT_R16G16B16A16_FLOAT,
		2,
		&mPostProcessVS,
		nullptr,
		nullptr,
		nullptr,
		&mPostProcessPS_H))
	{
		printf("CreatePostProcessPipeline PSO 1 failed\n");
		return false;
	}

	if (!mRenderer.CreatePSO(
		device,
		mRenderer.GetPostProcessPsoPtr(1),
		mRenderer.GetPostProcessRootSignature(),
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
		CD3DX12_BLEND_DESC(D3D12_DEFAULT),//Renderer::AdditiveBlend(),
		CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT),
		DXGI_FORMAT_R16G16B16A16_FLOAT,
		2,
		&mPostProcessVS,
		nullptr,
		nullptr,
		nullptr,
		&mPostProcessPS_V))
	{
		printf("CreatePostProcessPipeline PSO 2 failed\n");
		return false;
	}

	// graphics
	if (!mRenderer.CreateGraphicsPipeline(
		device,
		mFrameGraphics.GetTextureVec(),
		mFrameGraphics.GetRenderTextureVec()))
	{
		printf("CreateGraphicsPipeline failed\n");
		return false;
	}

	if (!mRenderer.CreatePSO(
		device,
		mRenderer.GetGraphicsPsoPtr(0),
		mRenderer.GetGraphicsRootSignature(),
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH,//D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
		CD3DX12_BLEND_DESC(D3D12_DEFAULT),// Renderer::NoBlend(),
		CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT),
		DXGI_FORMAT_R8G8B8A8_UNORM,
		1,
		&mVertexShader,
		&mHullShader,
		&mDomainShader,
		nullptr,
		&mPixelShader))
	{
		printf("CreateGraphicsPipeline PSO failed\n");
		return false;
	}

	/////////////////////////////////////////////////////////

	if (!FlushCommand())
	{
		printf("ExecuteCreateCommand failed\n");
		return false;
	}

	// Create imgui context
	if (!InitImgui())
	{
		printf("ExecuteCreateCommand failed\n");
		return false;
	}

	return true;
}

void Update()
{
	frameCount++;
	mFrameWaveParticle.SetUniformTime(frameCount);
	mFrameWaveParticle.UpdateUniformBuffer();
	mFrameGraphics.SetUniformTime(frameCount);
	mFrameGraphics.UpdateUniformBuffer();
}

void WaitForPreviousFrame()
{
	HRESULT hr;

	// swap the current rtv buffer index so we draw on the correct buffer
	frameIndex = swapChain->GetCurrentBackBufferIndex();

	// if the current fence value is still less than "fenceValue", then we know the GPU has not finished executing
	// the command queue since it has not reached the "commandQueue->Signal(fence, fenceValue)" command
	if (fence[frameIndex]->GetCompletedValue() < fenceValue[frameIndex])
	{
		// we have the fence create an event which is signaled once the fence's current value is "fenceValue"
		hr = fence[frameIndex]->SetEventOnCompletion(fenceValue[frameIndex], fenceEvent);
		if (FAILED(hr))
		{
			Running = false;
		}

		// We will wait until the fence has triggered the event that it's current value has reached "fenceValue". once it's value
		// has reached "fenceValue", we know the command queue has finished executing
		WaitForSingleObject(fenceEvent, INFINITE);
	}

	// increment fenceValue for next frame
	fenceValue[frameIndex]++;
}

void UpdatePipeline()
{
	HRESULT hr;

	// We have to wait for the gpu to finish with the command allocator before we reset it
	WaitForPreviousFrame();

	// we can only reset an allocator once the gpu is done with it
	// resetting an allocator frees the memory that the command list was stored in
	hr = commandAllocator[frameIndex]->Reset();
	if (FAILED(hr))
	{
		Running = false;
	}

	// reset the command list. by resetting the command list we are putting it into
	// a recording state so we can start recording commands into the command allocator.
	// the command allocator that we reference here may have multiple command lists
	// associated with it, but only one can be recording at any time. Make sure
	// that any other command lists associated to this command allocator are in
	// the closed state (not recording).
	// Here you will pass an initial pipeline state object as the second parameter,
	// but in this tutorial we are only clearing the rtv, and do not actually need
	// anything but an initial default pipeline, which is what we get by setting
	// the second parameter to NULL
	hr = commandList->Reset(commandAllocator[frameIndex], nullptr);
	if (FAILED(hr))
	{
		Running = false;
	}

	///////// RECORD GRAPHICS COMMANDS BEGIN /////////
	mRenderer.RecordBegin(frameIndex, commandList);
	///////// RECORD GRAPHICS COMMANDS BEGIN /////////

	vector<D3D12_RESOURCE_BARRIER> barrierGraphicsToWaveParticle = {
		CD3DX12_RESOURCE_BARRIER::Transition(mRenderTextureWaveParticle.GetTextureBuffer(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET, 0)
	};
	commandList->ResourceBarrier(barrierGraphicsToWaveParticle.size(), barrierGraphicsToWaveParticle.data());

	///////// MY WAVE PARTICLE PIPELINE /////////
	//vvvvvvvvvvvvvvvvvvvvvvvvvvv//
	commandList->SetPipelineState(mRenderer.GetWaveParticlePSO(0));
	mRenderer.RecordWaveParticlePipeline(
		mRenderTextureWaveParticle.GetRtvHandle(),
		mRenderer.GetDsvHandle(),
		commandList,
		mRenderer.GetWaveParticleRootSignature(),
		mRenderer.GetWaveParticleDescriptorHeap(),
		&mFrameWaveParticle,
		&mScene,
		D3D_PRIMITIVE_TOPOLOGY_POINTLIST);
	//^^^^^^^^^^^^^^^^^^^^^^^^^^^//
	///////// MY WAVE PARTICLE PIPELINE /////////

	vector<D3D12_RESOURCE_BARRIER> barrierWaveParticleToPostprocessH = {
		CD3DX12_RESOURCE_BARRIER::Transition(mRenderTextureWaveParticle.GetTextureBuffer(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, 0),
		CD3DX12_RESOURCE_BARRIER::Transition(mRenderTexturePostProcessH1.GetTextureBuffer(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET, 0),
		CD3DX12_RESOURCE_BARRIER::Transition(mRenderTexturePostProcessH2.GetTextureBuffer(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET, 0)
	};
	commandList->ResourceBarrier(barrierWaveParticleToPostprocessH.size(), barrierWaveParticleToPostprocessH.data());
	
	///////// MY POSTPROCESS H PIPELINE /////////
	//vvvvvvvvvvvvvvvvvvvvvvvvvvv//
	vector<RenderTexture*> rtvVec1 = {&mRenderTexturePostProcessH1, &mRenderTexturePostProcessH2};
	commandList->SetPipelineState(mRenderer.GetPostProcessPSO(0));
	mRenderer.RecordPostProcessPipeline(
		rtvVec1,
		mRenderer.GetDsvHandle(),
		commandList,
		mRenderer.GetPostProcessRootSignature(),
		mRenderer.GetPostProcessDescriptorHeap(),
		&mFramePostProcess,
		&mScene,
		D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//^^^^^^^^^^^^^^^^^^^^^^^^^^^//
	///////// MY POSTPROCESS H PIPELINE /////////

	vector<D3D12_RESOURCE_BARRIER> barrierPostprocessHToPostprocessV = {
		CD3DX12_RESOURCE_BARRIER::Transition(mRenderTexturePostProcessH1.GetTextureBuffer(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, 0),
		CD3DX12_RESOURCE_BARRIER::Transition(mRenderTexturePostProcessH2.GetTextureBuffer(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, 0),
		CD3DX12_RESOURCE_BARRIER::Transition(mRenderTexturePostProcessV1.GetTextureBuffer(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET, 0),
		CD3DX12_RESOURCE_BARRIER::Transition(mRenderTexturePostProcessV2.GetTextureBuffer(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET, 0)
	};
	commandList->ResourceBarrier(barrierPostprocessHToPostprocessV.size(), barrierPostprocessHToPostprocessV.data());

	///////// MY POSTPROCESS V PIPELINE /////////
	//vvvvvvvvvvvvvvvvvvvvvvvvvvv//
	vector<RenderTexture*> rtvVec2 = { &mRenderTexturePostProcessV1, &mRenderTexturePostProcessV2 };
	commandList->SetPipelineState(mRenderer.GetPostProcessPSO(1));
	mRenderer.RecordPostProcessPipeline(
		rtvVec2,
		mRenderer.GetDsvHandle(),
		commandList,
		mRenderer.GetPostProcessRootSignature(),
		mRenderer.GetPostProcessDescriptorHeap(),
		&mFramePostProcess,
		&mScene,
		D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//^^^^^^^^^^^^^^^^^^^^^^^^^^^//
	///////// MY POSTPROCESS V PIPELINE /////////

	vector<D3D12_RESOURCE_BARRIER> barrierPostprocessVToGraphics = {
		CD3DX12_RESOURCE_BARRIER::Transition(mRenderTexturePostProcessV1.GetTextureBuffer(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, 0),
		CD3DX12_RESOURCE_BARRIER::Transition(mRenderTexturePostProcessV2.GetTextureBuffer(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, 0)
	};
	commandList->ResourceBarrier(barrierPostprocessVToGraphics.size(), barrierPostprocessVToGraphics.data());
	
	///////// MY GRAPHICS PIPELINE /////////
	//vvvvvvvvvvvvvvvvvvvvvvvvvvv//
	commandList->SetPipelineState(mRenderer.GetGraphicsPSO(0));
	mRenderer.RecordGraphicsPipeline(
		mRenderer.GetRtvHandle(frameIndex),
		mRenderer.GetDsvHandle(),
		commandList,
		mRenderer.GetGraphicsRootSignature(),
		mRenderer.GetGraphicsDescriptorHeap(),
		&mFrameGraphics,
		&mScene,
		D3D_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);// D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);// 
	//^^^^^^^^^^^^^^^^^^^^^^^^^^^//
	///////// MY GRAPHICS PIPELINE /////////

	///////// IMGUI PIPELINE /////////
	//vvvvvvvvvvvvvvvvvvvvvvvvvvvvvv//
	commandList->SetDescriptorHeaps(1, &g_pd3dSrvDescHeap);
	ImGui::Render();
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList);
	///////// IMGUI PIPELINE /////////
	//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^//

	// RECORD GRAPHICS COMMANDS END //
	mRenderer.RecordEnd(frameIndex, commandList);
	// RECORD GRAPHICS COMMANDS END //

	hr = commandList->Close();
	if (FAILED(hr))
	{
		Running = false;
	}
}

void Render()
{
	HRESULT hr;

	UpdatePipeline(); // update the pipeline by sending commands to the commandqueue

	// create an array of command lists (only one command list here)
	ID3D12CommandList* ppCommandLists[] = { commandList };

	// execute the array of command lists
	commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	// this command goes in at the end of our command queue. we will know when our command queue 
	// has finished because the fence value will be set to "fenceValue" from the GPU since the command
	// queue is being executed on the GPU
	hr = commandQueue->Signal(fence[frameIndex], fenceValue[frameIndex]);
	if (FAILED(hr))
	{
		Running = false;
	}

	// present the current backbuffer
	hr = swapChain->Present(0, 0);
	if (FAILED(hr))
	{
		Running = false;
	}
}

void Gui()
{
	////////////////////////////////////
	////////// IMGUI EXAMPLES///////////
	//vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv//

	// Start the Dear ImGui frame
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	static float heightScale = mScene.GetUniformHeightScale();
	static float waveParticleSpeedScale = mScene.GetUniformWaveParticleSpeedScale();
	static float flowSpeed = mScene.GetUniformFlowSpeed();
	static float dxScale = mScene.GetUniformDxScale();
	static float dzScale = mScene.GetUniformDzScale();
	static float timeScale = mScene.GetUniformTimeScale();
	static int edgeTess = mScene.GetUniformEdgeTessFactor();
	static int insideTess = mScene.GetUniformInsideTessFactor();
	static int blurRadius = mScene.GetUniformBlurRadius();
	static int mode = mScene.GetUniformMode();
	static float lighthight = mScene.GetUniformLighthight();
	static float extinctcoeff = mScene.Getextinctcoeff();
	static float shiness = mScene.Getshiness();
	bool needToUpdateSceneUniform = false;

	ImGui::SetNextWindowPos(ImVec2(0, 0));

	ImGui::Begin("Control Panel ");                        
	ImGui::Text("Wave Particles Scale ");

	ImGui::Combo("mode", &mode, "default\0flow map\0flow map driven texture\0wave particle\0horizontal blur\0vertical blur\0horizontal and vertical blur\0normal\0sss\0\0");

	ImGui::SliderFloat("height ", &heightScale, 0.0f, 3.0f);
	ImGui::SliderFloat("dx ", &dxScale, 0.0f, 0.13f, "%.6f");
	ImGui::SliderFloat("dz ", &dzScale, 0.0f, 0.13f, "%.6f");
	ImGui::SliderInt("blur radius ", &blurRadius, 0, 500);
	ImGui::SliderFloat("wpSpeed ", &waveParticleSpeedScale, 0.0f, 0.0005f, "%.6f");
	ImGui::SliderFloat("flowSpeed ", &flowSpeed, 0.f, 0.0005f, "%.6f");
	ImGui::SliderFloat("timeScale ", &timeScale, 0.0f, 100.0f, "%.6f");
	ImGui::SliderInt("edge tess ", &edgeTess, 0, 32);
	ImGui::SliderInt("inside tess ", &insideTess, 0, 32);
	ImGui::SliderFloat("light hight", &lighthight, 1, 20);
	ImGui::SliderFloat("extinct coeff", &extinctcoeff, -0.7, 0);
	ImGui::SliderFloat("shiness", &shiness, 10, 600);

	if (shiness != mScene.Getshiness())
	{
		mScene.Setshiness(shiness);
		needToUpdateSceneUniform = true;
	}
	if (extinctcoeff != mScene.Getextinctcoeff())
	{
		mScene.Setextinctcoeff(extinctcoeff);
		needToUpdateSceneUniform = true;
	}

	if (lighthight != mScene.GetUniformLighthight())
	{
		mScene.SetUniformLighthight(lighthight);
		needToUpdateSceneUniform = true;
	}

	if (mode != mScene.GetUniformMode())
	{
		mScene.SetUniformMode(mode);
		needToUpdateSceneUniform = true;
	}

	if (heightScale != mScene.GetUniformHeightScale())
	{
		mScene.SetUniformHeightScale(heightScale);
		needToUpdateSceneUniform = true;
	}

	if (dxScale != mScene.GetUniformDxScale())
	{
		mScene.SetUniformDxScale(dxScale);
		needToUpdateSceneUniform = true;
	}

	if (dzScale != mScene.GetUniformDzScale())
	{
		mScene.SetUniformDzScale(dzScale);
		needToUpdateSceneUniform = true;
	}

	if (blurRadius != mScene.GetUniformBlurRadius())
	{
		mScene.SetUniformBlurRadius(blurRadius);
		needToUpdateSceneUniform = true;
	}

	if (waveParticleSpeedScale != mScene.GetUniformWaveParticleSpeedScale())
	{
		mScene.SetUniformWaveParticleSpeedScale(waveParticleSpeedScale);
		needToUpdateSceneUniform = true;
	}

	if (flowSpeed != mScene.GetUniformFlowSpeed())
	{
		mScene.SetUniformFlowSpeed(flowSpeed);
		needToUpdateSceneUniform = true;
	}

	if (timeScale != mScene.GetUniformTimeScale())
	{
		mScene.SetUniformTimeScale(timeScale);
		needToUpdateSceneUniform = true;
	}

	if (edgeTess != mScene.GetUniformEdgeTessFactor())
	{
		mScene.SetUniformEdgeTessFactor(edgeTess);
		needToUpdateSceneUniform = true;
	}

	if (insideTess != mScene.GetUniformInsideTessFactor())
	{
		mScene.SetUniformInsideTessFactor(insideTess);
		needToUpdateSceneUniform = true;
	}
	
	if (needToUpdateSceneUniform)
	{
		mScene.UpdateUniformBuffer();
	}

	ImGui::Text("%.3f ms/frame (%.1f FPS) ", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	ImGui::Text("Hold C and use mouse to rotate camera.");
	ImGui::End();

	//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^//
	////////// IMGUI EXAMPLES///////////
	////////////////////////////////////
}

void Cleanup()
{
	//imgui stuff
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	//direct input stuff
	DIKeyboard->Unacquire();
	DIMouse->Unacquire();
	DirectInput->Release();

	// wait for the gpu to finish all frames
	for (int i = 0; i < FrameBufferCount; ++i)
	{
		frameIndex = i;
		WaitForPreviousFrame();
	}

	// get swapchain out of full screen before exiting
	BOOL fs = false;
	swapChain->GetFullscreenState(&fs, NULL);
	if (fs == TRUE)
		swapChain->SetFullscreenState(false, NULL);

	SAFE_RELEASE(device);
	SAFE_RELEASE(swapChain);
	SAFE_RELEASE(commandQueue);
	SAFE_RELEASE(commandList);

	for (int i = 0; i < FrameBufferCount; ++i)
	{
		SAFE_RELEASE(commandAllocator[i]);
		SAFE_RELEASE(fence[i]);
	};
}

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WndProc(HWND hwnd,	UINT msg, WPARAM wParam, LPARAM lParam)
{
	//vv imgui vv//
	if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wParam, lParam))
		return true;
	//^^ imgui ^^//

	switch (msg)
	{
	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE) {
			if (MessageBox(0, L"Are you sure you want to exit?",
				L"Really?", MB_YESNO | MB_ICONQUESTION) == IDYES)
			{
				Running = false;
				DestroyWindow(hwnd);
			}
		}
		return 0;

	case WM_DESTROY: // x button on top right corner of window was pressed
		Running = false;
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd,
		msg,
		wParam,
		lParam);
}

// create and show the window
bool InitializeWindow(HINSTANCE hInstance, int ShowWnd,	bool fullscreen)
{
	if (fullscreen)
	{
		HMONITOR hmon = MonitorFromWindow(hwnd,
			MONITOR_DEFAULTTONEAREST);
		MONITORINFO mi = { sizeof(mi) };
		GetMonitorInfo(hmon, &mi);

		Width = mi.rcMonitor.right - mi.rcMonitor.left;
		Height = mi.rcMonitor.bottom - mi.rcMonitor.top;
	}

	WNDCLASSEX wc;

	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = NULL;
	wc.cbWndExtra = NULL;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 2);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = WindowName;
	wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	if (!RegisterClassEx(&wc))
	{
		MessageBox(NULL, L"Error registering class",
			L"Error", MB_OK | MB_ICONERROR);
		return false;
	}

	hwnd = CreateWindowEx(NULL,
		WindowName,
		WindowTitle,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		Width, Height,
		NULL,
		NULL,
		hInstance,
		NULL);

	if (!hwnd)
	{
		MessageBox(NULL, L"Error creating window",
			L"Error", MB_OK | MB_ICONERROR);
		return false;
	}

	if (fullscreen)
	{
		SetWindowLong(hwnd, GWL_STYLE, 0);
	}

	ShowWindow(hwnd, ShowWnd);
	UpdateWindow(hwnd);

	return true;
}

void mainloop() 
{
	MSG msg;
	ZeroMemory(&msg, sizeof(MSG));

	while (Running)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
				break;

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else {
			// run gui code
			Gui();
			// run game code
			DetectInput();
			Update(); // update the game logic
			Render(); // execute the command queue (rendering the frame is the result of the gpu executing the command lists)
		}
	}
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	InitConsole();

	// initialize data
	if (!CreateScene())
	{
		MessageBox(0, L"Failed to initialize data",	L"Error", MB_OK);
		return 1;
	}

	// create the window
	if (!InitializeWindow(hInstance, nShowCmd, FullScreen))
	{
		MessageBox(0, L"Window Initialization - Failed", L"Error", MB_OK);
		return 1;
	}

	//Initialize input device
	if (!InitDirectInput(hInstance))
	{
		MessageBox(0, L"Failed to initialize input", L"Error", MB_OK);
		return 1;
	}

	// initialize direct3d
	if (!InitD3D())
	{
		MessageBox(0, L"Failed to initialize direct3d 12", L"Error", MB_OK);
		Cleanup();
		return 1;
	}

	// start the main loop
	mainloop();

	// we want to wait for the gpu to finish executing the command list before we start releasing everything
	WaitForPreviousFrame();

	// close the fence event
	CloseHandle(fenceEvent);

	// clean up everything
	Cleanup();

	return 0;
}

bool CheckError(HRESULT hr, ID3D10Blob* error_message)
{
	if (FAILED(hr))
	{
		printf("FAILED:0x%x\n", hr);
		if (error_message != nullptr)
		{
			printf("return value: %d, error message: %s\n", hr, (char*)error_message->GetBufferPointer());
		}
		return false;
	}
	return true;
}