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
int Width = 1000;// width and height of the window
int Height = 1000;
int WidthRT = 1000;
int HeightRT = 1000;
int WidthRtFluid = 1000;
int HeightRtFluid = 1000;
int JacobiIteration = 40;
bool FluidSimulation = true;
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
Frame mFramePostProcessH;
Frame mFramePostProcessV;
Frame mFrameFluidAdvect;
Frame mFrameFluidSplat;
Frame mFrameFluidDivergence;
Frame mFrameFluidJacobi;
Frame mFrameFluidGradient;
OrbitCamera mCamera(4.f, 0.f, 0.f, XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), (float)Width, (float)Height, 45.0f, 0.1f, 1000.0f);
Camera mDummyCamera(XMFLOAT3{ 4.f,0,0 }, XMFLOAT3{ 0,0,0 }, XMFLOAT3{ 0,1,0 }, WidthRT, HeightRT, 45.0f, 0.1f, 1000.f);
Camera mDummyCameraFluid(XMFLOAT3{ 4.f,0,0 }, XMFLOAT3{ 0,0,0 }, XMFLOAT3{ 0,1,0 }, WidthRtFluid, HeightRtFluid, 45.0f, 0.1f, 1000.f);
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
Shader mFluidAdvectPS(Shader::ShaderType::PixelShader, L"AdvectPS.hlsl");
Shader mFluidSplatPS(Shader::ShaderType::PixelShader, L"SplatPS.hlsl");
Shader mFluidComputeDivergencePS(Shader::ShaderType::PixelShader, L"ComputeDivergencePS.hlsl");
Shader mFluidJacobiPS(Shader::ShaderType::PixelShader, L"JacobiPS.hlsl");
Shader mFluidSubtractGradientPS(Shader::ShaderType::PixelShader, L"SubtractGradientPS.hlsl");
//Texture mTextureFlowmap(L"flow2.jpg");
Texture mTextureAlbedo(L"checkerboard.jpg");
Texture mTextureObstacle(L"ob1.jpg");
RenderTexture mRenderTextureWaveParticle(WidthRT, HeightRT);
RenderTexture mRenderTexturePostProcessH1(WidthRT, HeightRT);
RenderTexture mRenderTexturePostProcessH2(WidthRT, HeightRT);
RenderTexture mRenderTexturePostProcessV1(WidthRT, HeightRT);
RenderTexture mRenderTexturePostProcessV2(WidthRT, HeightRT);
RenderTexture mRenderTextureFluidDivergence(WidthRtFluid, HeightRtFluid);
RenderTexture mRenderTextureFluidVelocity1(WidthRtFluid, HeightRtFluid);
RenderTexture mRenderTextureFluidVelocity2(WidthRtFluid, HeightRtFluid);
RenderTexture mRenderTextureFluidDensity1(WidthRtFluid, HeightRtFluid);
RenderTexture mRenderTextureFluidDensity2(WidthRtFluid, HeightRtFluid);
RenderTexture mRenderTextureFluidPressure1(WidthRtFluid, HeightRtFluid);
RenderTexture mRenderTextureFluidPressure2(WidthRtFluid, HeightRtFluid);
RenderTexture *pRtVelocityPing;
RenderTexture *pRtVelocityPong;
RenderTexture *pRtDensityPing;
RenderTexture *pRtDensityPong;
RenderTexture *pRtPressurePing;
RenderTexture *pRtPressurePong;

//imgui stuff
ID3D12DescriptorHeap* g_pd3dSrvDescHeap = NULL;
bool show_demo_window = true;
bool show_another_window = false;
ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

void SwitchPingPong(RenderTexture** ping, RenderTexture** pong)
{
	RenderTexture* temp = *ping;
	*ping = *pong;
	*pong = temp;
}

bool CreateScene()
{
	// A. Frame

	// 1. fluid

	// advect velocity
	mFrameFluidAdvect.AddCamera(&mDummyCameraFluid);
	mFrameFluidAdvect.AddMesh(&mQuad);

	// splat velocity
	mFrameFluidSplat.AddCamera(&mDummyCameraFluid);
	mFrameFluidSplat.AddMesh(&mQuad);

	// compute divergence
	mFrameFluidDivergence.AddCamera(&mDummyCameraFluid);
	mFrameFluidDivergence.AddMesh(&mQuad);

	// jacobi
	mFrameFluidJacobi.AddCamera(&mDummyCameraFluid);
	mFrameFluidJacobi.AddMesh(&mQuad);

	// subtract gradient
	mFrameFluidGradient.AddCamera(&mDummyCameraFluid);
	mFrameFluidGradient.AddMesh(&mQuad);

	// 2. wave particle

	// wave particle update
	mFrameWaveParticle.AddCamera(&mDummyCamera);
	mFrameWaveParticle.AddMesh(&mWaveParticle);
	mFrameWaveParticle.AddRenderTexture(&mRenderTextureWaveParticle);

	// filter
	mFramePostProcessH.AddCamera(&mDummyCamera);
	mFramePostProcessH.AddMesh(&mQuad);
	mFramePostProcessH.AddTexture(&mRenderTextureWaveParticle);
	mFramePostProcessH.AddRenderTexture(&mRenderTexturePostProcessH1);
	mFramePostProcessH.AddRenderTexture(&mRenderTexturePostProcessH2);

	mFramePostProcessV.AddCamera(&mDummyCamera);
	mFramePostProcessV.AddMesh(&mQuad);
	mFramePostProcessV.AddTexture(&mRenderTexturePostProcessH1);
	mFramePostProcessV.AddTexture(&mRenderTexturePostProcessH2);
	mFramePostProcessV.AddRenderTexture(&mRenderTexturePostProcessV1);
	mFramePostProcessV.AddRenderTexture(&mRenderTexturePostProcessV2);

	// tessellation
	mFrameGraphics.AddCamera(&mCamera);
	mFrameGraphics.AddMesh(&mWaterSurface);
	mFrameGraphics.AddTexture(&mTextureAlbedo);//t0
	//mFrameGraphics.AddTexture(&mRenderTextureFluidPressure1);//t1
	mFrameGraphics.AddTexture(&mRenderTextureFluidVelocity1);//t1
	mFrameGraphics.AddTexture(&mRenderTexturePostProcessV1);//t2
	mFrameGraphics.AddTexture(&mRenderTexturePostProcessV2);//t3

	mFramePostProcessH.SetUniformTime(0);
	mFramePostProcessV.SetUniformTime(0);
	mFrameWaveParticle.SetUniformTime(0);

	// B. Data

	// ping pong
	pRtVelocityPing = &mRenderTextureFluidVelocity1;
	pRtVelocityPong = &mRenderTextureFluidVelocity2;
	pRtDensityPing = &mRenderTextureFluidDensity1;
	pRtDensityPong = &mRenderTextureFluidDensity2;
	pRtPressurePing = &mRenderTextureFluidPressure1;
	pRtPressurePong = &mRenderTextureFluidPressure2;

	// frame
	mScene.AddFrame(&mFrameFluidAdvect);
	mScene.AddFrame(&mFrameFluidSplat);
	mScene.AddFrame(&mFrameFluidDivergence);
	mScene.AddFrame(&mFrameFluidJacobi);
	mScene.AddFrame(&mFrameFluidGradient);
	mScene.AddFrame(&mFrameGraphics);
	mScene.AddFrame(&mFrameWaveParticle);
	mScene.AddFrame(&mFramePostProcessH);
	mScene.AddFrame(&mFramePostProcessV);

	// camera
	mScene.AddCamera(&mCamera);
	mScene.AddCamera(&mDummyCamera);
	mScene.AddCamera(&mDummyCameraFluid);

	// mesh
	mScene.AddMesh(&mQuad);
	mScene.AddMesh(&mWaveParticle);
	mScene.AddMesh(&mWaterSurface);

	// shader
	mScene.AddShader(&mFluidAdvectPS);
	mScene.AddShader(&mFluidSplatPS);
	mScene.AddShader(&mFluidComputeDivergencePS);
	mScene.AddShader(&mFluidJacobiPS);
	mScene.AddShader(&mFluidSubtractGradientPS);
	mScene.AddShader(&mVertexShader);
	mScene.AddShader(&mHullShader);
	mScene.AddShader(&mDomainShader);
	mScene.AddShader(&mPixelShader);
	mScene.AddShader(&mPostProcessVS);
	mScene.AddShader(&mPostProcessPS_H);
	mScene.AddShader(&mPostProcessPS_V);
	mScene.AddShader(&mWaveParticleVS);
	mScene.AddShader(&mWaveParticlePS);

	// texture
	mScene.AddTexture(&mTextureAlbedo);
	mScene.AddTexture(&mTextureObstacle);
	
	// render texture
	mScene.AddRenderTexture(&mRenderTextureWaveParticle);
	mScene.AddRenderTexture(&mRenderTexturePostProcessH1);
	mScene.AddRenderTexture(&mRenderTexturePostProcessH2);
	mScene.AddRenderTexture(&mRenderTexturePostProcessV1);
	mScene.AddRenderTexture(&mRenderTexturePostProcessV2);
	mScene.AddRenderTexture(&mRenderTextureFluidVelocity1);
	mScene.AddRenderTexture(&mRenderTextureFluidVelocity2);
	mScene.AddRenderTexture(&mRenderTextureFluidDensity1);
	mScene.AddRenderTexture(&mRenderTextureFluidDensity2);
	mScene.AddRenderTexture(&mRenderTextureFluidPressure1);
	mScene.AddRenderTexture(&mRenderTextureFluidPressure2);
	mScene.AddRenderTexture(&mRenderTextureFluidDivergence);

	// set uniform
	mScene.SetUniformHeightScale(1.3);
	mScene.SetUniformWaveParticleSpeedScale(0.0001);
	mScene.SetUniformFlowSpeed(0.0001);
	mScene.SetUniformDxScale(0.03);
	mScene.SetUniformDzScale(0.03);
	mScene.SetUniformTimeScale(1.0);
	mScene.SetUniformTimeStepFluid(0.0084070);
	mScene.SetUniformJacobiObstacleScale(13);
	mScene.SetUniformFluidCellSize(1.3);
	mScene.SetUniformJacobiInvBeta(0.2);
	mScene.SetUniformFluidDissipation(0.992308);
	mScene.SetUniformGradientScale(0.972198);
	mScene.SetUniformSplatDirU(0.450549);
	mScene.SetUniformSplatDirV(0.5);
	mScene.SetUniformSplatScale(0.01);
	mScene.SetUniformTextureWidthHeight(WidthRT, HeightRT);
	mScene.SetUniformTextureWidthHeightFluid(WidthRtFluid /3, HeightRtFluid / 3);
	mScene.SetUniformEdgeTessFactor(4);
	mScene.SetUniformInsideTessFactor(2);
	mScene.SetUniformBlurRadius(50);
	mScene.SetUniformMode(1);

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

	// fluid advection
	for (int i = 0; i < FrameBufferCount; i++)
	{
		if (!mRenderer.CreateFluidRootSignature(
			device,
			mRenderer.GetFluidRootSignaturePtr(i, static_cast<int>(Renderer::FluidStage::Advection)),
			3))
		{
			printf("Create Fluid RS advection failed\n");
			return false;
		}

		if (!mRenderer.CreateHeap(
			device,
			mRenderer.GetFluidDescriptorHeapPtr(i, static_cast<int>(Renderer::FluidStage::Advection)),
			mRenderer.GetFluidRtvDescriptorHeapPtr(i, static_cast<int>(Renderer::FluidStage::Advection)),
			3,
			1))
		{
			printf("Create Fluid Heap advection failed\n");
			return false;
		}

		if (!mRenderer.CreatePSO(
			device,
			mRenderer.GetFluidPsoPtr(i, static_cast<int>(Renderer::FluidStage::Advection)),
			mRenderer.GetFluidRootSignature(i, static_cast<int>(Renderer::FluidStage::Advection)),
			D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
			CD3DX12_BLEND_DESC(D3D12_DEFAULT),
			Renderer::NoDepthTest(),
			DXGI_FORMAT_R16G16B16A16_FLOAT,
			1,
			&mPostProcessVS,
			nullptr,
			nullptr,
			nullptr,
			&mFluidAdvectPS))
		{
			printf("Create Fluid Pipeline PSO advection failed\n");
			return false;
		}
	}

	// fluid splat
	for (int i = 0; i < FrameBufferCount; i++)
	{
		if (!mRenderer.CreateFluidRootSignature(
			device,
			mRenderer.GetFluidRootSignaturePtr(i, static_cast<int>(Renderer::FluidStage::Splat)),
			1))
		{
			printf("Create Fluid RS splat failed\n");
			return false;
		}

		if (!mRenderer.CreateHeap(
			device,
			mRenderer.GetFluidDescriptorHeapPtr(i, static_cast<int>(Renderer::FluidStage::Splat)),
			mRenderer.GetFluidRtvDescriptorHeapPtr(i, static_cast<int>(Renderer::FluidStage::Splat)),
			1,
			1))
		{
			printf("Create Fluid Heap splat failed\n");
			return false;
		}

		if (!mRenderer.CreatePSO(
			device,
			mRenderer.GetFluidPsoPtr(i, static_cast<int>(Renderer::FluidStage::Splat)),
			mRenderer.GetFluidRootSignature(i, static_cast<int>(Renderer::FluidStage::Splat)),
			D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
			mRenderer.AdditiveBlend(),
			Renderer::NoDepthTest(),
			DXGI_FORMAT_R16G16B16A16_FLOAT,
			1,
			&mPostProcessVS,
			nullptr,
			nullptr,
			nullptr,
			&mFluidSplatPS))
		{
			printf("Create Fluid Pipeline PSO splat failed\n");
			return false;
		}
	}

	// fluid divergence
	for (int i = 0; i < FrameBufferCount; i++)
	{
		if (!mRenderer.CreateFluidRootSignature(
			device,
			mRenderer.GetFluidRootSignaturePtr(i, static_cast<int>(Renderer::FluidStage::ComputeDivergence)),
			2))
		{
			printf("Create Fluid RS divergence failed\n");
			return false;
		}

		if (!mRenderer.CreateHeap(
			device,
			mRenderer.GetFluidDescriptorHeapPtr(i, static_cast<int>(Renderer::FluidStage::ComputeDivergence)),
			mRenderer.GetFluidRtvDescriptorHeapPtr(i, static_cast<int>(Renderer::FluidStage::ComputeDivergence)),
			2,
			1))
		{
			printf("Create Fluid Heap divergence failed\n");
			return false;
		}

		if (!mRenderer.CreatePSO(
			device,
			mRenderer.GetFluidPsoPtr(i, static_cast<int>(Renderer::FluidStage::ComputeDivergence)),
			mRenderer.GetFluidRootSignature(i, static_cast<int>(Renderer::FluidStage::ComputeDivergence)),
			D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
			CD3DX12_BLEND_DESC(D3D12_DEFAULT),
			Renderer::NoDepthTest(),
			DXGI_FORMAT_R16G16B16A16_FLOAT,
			1,
			&mPostProcessVS,
			nullptr,
			nullptr,
			nullptr,
			&mFluidComputeDivergencePS))
		{
			printf("Create Fluid Pipeline PSO divergence failed\n");
			return false;
		}
	}

	// fluid jacobi
	for (int i = 0; i < FrameBufferCount; i++)
	{
		if (!mRenderer.CreateFluidRootSignature(
			device,
			mRenderer.GetFluidRootSignaturePtr(i, static_cast<int>(Renderer::FluidStage::Jacobi)),
			3))
		{
			printf("Create Fluid RS jacobi failed\n");
			return false;
		}

		if (!mRenderer.CreateHeap(
			device,
			mRenderer.GetFluidDescriptorHeapPtr(i, static_cast<int>(Renderer::FluidStage::Jacobi)),
			mRenderer.GetFluidRtvDescriptorHeapPtr(i, static_cast<int>(Renderer::FluidStage::Jacobi)),
			3,
			1))
		{
			printf("Create Fluid Heap jacobi failed\n");
			return false;
		}

		if (!mRenderer.CreatePSO(
			device,
			mRenderer.GetFluidPsoPtr(i, static_cast<int>(Renderer::FluidStage::Jacobi)),
			mRenderer.GetFluidRootSignature(i, static_cast<int>(Renderer::FluidStage::Jacobi)),
			D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
			CD3DX12_BLEND_DESC(D3D12_DEFAULT),
			Renderer::NoDepthTest(),
			DXGI_FORMAT_R16G16B16A16_FLOAT,
			1,
			&mPostProcessVS,
			nullptr,
			nullptr,
			nullptr,
			&mFluidJacobiPS))
		{
			printf("Create Fluid Pipeline PSO jacobi failed\n");
			return false;
		}
	}

	// fluid gradient
	for (int i = 0; i < FrameBufferCount; i++)
	{
		if (!mRenderer.CreateFluidRootSignature(
			device,
			mRenderer.GetFluidRootSignaturePtr(i, static_cast<int>(Renderer::FluidStage::SubtractGradient)),
			3))
		{
			printf("Create Fluid RS gradient failed\n");
			return false;
		}

		if (!mRenderer.CreateHeap(
			device,
			mRenderer.GetFluidDescriptorHeapPtr(i, static_cast<int>(Renderer::FluidStage::SubtractGradient)),
			mRenderer.GetFluidRtvDescriptorHeapPtr(i, static_cast<int>(Renderer::FluidStage::SubtractGradient)),
			3,
			1))
		{
			printf("Create Fluid Heap gradient failed\n");
			return false;
		}

		if (!mRenderer.CreatePSO(
			device,
			mRenderer.GetFluidPsoPtr(i, static_cast<int>(Renderer::FluidStage::SubtractGradient)),
			mRenderer.GetFluidRootSignature(i, static_cast<int>(Renderer::FluidStage::SubtractGradient)),
			D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
			CD3DX12_BLEND_DESC(D3D12_DEFAULT),
			Renderer::NoDepthTest(),
			DXGI_FORMAT_R16G16B16A16_FLOAT,
			1,
			&mPostProcessVS,
			nullptr,
			nullptr,
			nullptr,
			&mFluidSubtractGradientPS))
		{
			printf("Create Fluid Pipeline PSO gradient failed\n");
			return false;
		}
	}


	// wave particle
	if (!mRenderer.CreateWaveParticleRootSignature(
		device,
		mRenderer.GetWaveParticleRootSignaturePtr(static_cast<int>(Renderer::WaveParticleStage::Default)),
		mFrameWaveParticle.GetTextureVec().size()))
	{
		printf("CreateWaveParticle RS failed\n");
		return false;
	}

	if (!mRenderer.CreateHeapBindTexture(
		device,
		mRenderer.GetWaveParticleDescriptorHeapPtr(static_cast<int>(Renderer::WaveParticleStage::Default)),
		mRenderer.GetWaveParticleRtvDescriptorHeapPtr(static_cast<int>(Renderer::WaveParticleStage::Default)),
		mFrameWaveParticle.GetTextureVec(),
		mFrameWaveParticle.GetRenderTextureVec()))
	{
		printf("CreateWaveParticle Heap failed\n");
		return false;
	}

	if (!mRenderer.CreatePSO(
		device,
		mRenderer.GetWaveParticlePsoPtr(static_cast<int>(Renderer::WaveParticleStage::Default)),
		mRenderer.GetWaveParticleRootSignature(static_cast<int>(Renderer::WaveParticleStage::Default)),
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

	// post process H
	if (!mRenderer.CreatePostProcessRootSignature(
		device,
		mRenderer.GetPostProcessRootSignaturePtr(static_cast<int>(Renderer::PostProcessStage::Horizontal)),
		mFramePostProcessH.GetTextureVec().size()))
	{
		printf("CreatePostProcess H RS failed\n");
		return false;
	}

	if (!mRenderer.CreateHeapBindTexture(
		device,
		mRenderer.GetPostProcessDescriptorHeapPtr(static_cast<int>(Renderer::PostProcessStage::Horizontal)),
		mRenderer.GetPostProcessRtvDescriptorHeapPtr(static_cast<int>(Renderer::PostProcessStage::Horizontal)),
		mFramePostProcessH.GetTextureVec(),
		mFramePostProcessH.GetRenderTextureVec()))
	{
		printf("CreatePostProcess H Heap failed\n");
		return false;
	}

	if (!mRenderer.CreatePSO(
		device,
		mRenderer.GetPostProcessPsoPtr(static_cast<int>(Renderer::PostProcessStage::Horizontal)),
		mRenderer.GetPostProcessRootSignature(static_cast<int>(Renderer::PostProcessStage::Horizontal)),
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
		printf("CreatePostProcessPipeline PSO H failed\n");
		return false;
	}

	// post process V
	if (!mRenderer.CreatePostProcessRootSignature(
		device,
		mRenderer.GetPostProcessRootSignaturePtr(static_cast<int>(Renderer::PostProcessStage::Vertical)),
		mFramePostProcessV.GetTextureVec().size()))
	{
		printf("CreatePostProcess V RS failed\n");
		return false;
	}

	if (!mRenderer.CreateHeapBindTexture(
		device,
		mRenderer.GetPostProcessDescriptorHeapPtr(static_cast<int>(Renderer::PostProcessStage::Vertical)),
		mRenderer.GetPostProcessRtvDescriptorHeapPtr(static_cast<int>(Renderer::PostProcessStage::Vertical)),
		mFramePostProcessV.GetTextureVec(),
		mFramePostProcessV.GetRenderTextureVec()))
	{
		printf("CreatePostProcess V Heap failed\n");
		return false;
	}

	if (!mRenderer.CreatePSO(
		device,
		mRenderer.GetPostProcessPsoPtr(static_cast<int>(Renderer::PostProcessStage::Vertical)),
		mRenderer.GetPostProcessRootSignature(static_cast<int>(Renderer::PostProcessStage::Vertical)),
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
		printf("CreatePostProcessPipeline PSO V failed\n");
		return false;
	}

	// graphics
	if (!mRenderer.CreateGraphicsRootSignature(
		device,
		mRenderer.GetGraphicsRootSignaturePtr(static_cast<int>(Renderer::GraphicsStage::Default)),
		mFrameGraphics.GetTextureVec().size()))
	{
		printf("CreateGraphicsPipeline RS failed\n");
		return false;
	}

	if (!mRenderer.CreateHeapBindTexture(
		device,
		mRenderer.GetGraphicsDescriptorHeapPtr(static_cast<int>(Renderer::GraphicsStage::Default)),
		mRenderer.GetGraphicsRtvDescriptorHeapPtr(static_cast<int>(Renderer::GraphicsStage::Default)),
		mFrameGraphics.GetTextureVec(),
		mFrameGraphics.GetRenderTextureVec()))
	{
		printf("CreateGraphicsPipeline Heap failed\n");
		return false;
	}

	if (!mRenderer.CreatePSO(
		device,
		mRenderer.GetGraphicsPsoPtr(static_cast<int>(Renderer::GraphicsStage::Default)),
		mRenderer.GetGraphicsRootSignature(static_cast<int>(Renderer::GraphicsStage::Default)),
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

	if (FluidSimulation)
	{

		vector<D3D12_RESOURCE_BARRIER> barrierAdvectVelocity = {
			CD3DX12_RESOURCE_BARRIER::Transition(pRtVelocityPing->GetTextureBuffer(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, 0),
			CD3DX12_RESOURCE_BARRIER::Transition(pRtVelocityPong->GetTextureBuffer(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET, 0)
		};
		commandList->ResourceBarrier(barrierAdvectVelocity.size(), barrierAdvectVelocity.data());

		///////// MY FLUID PIPELINE advect velocity /////////
		//vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv//
		mRenderer.BindRenderTextureToRtvDescriptorHeap(
			device,
			mRenderer.GetFluidRtvDescriptorHeap(frameIndex, static_cast<int>(Renderer::FluidStage::Advection)),
			pRtVelocityPong,
			0);//rtv
		mRenderer.BindTextureToDescriptorHeap(
			device,
			mRenderer.GetFluidDescriptorHeap(frameIndex, static_cast<int>(Renderer::FluidStage::Advection)),
			&mTextureObstacle,
			0);//t0
		mRenderer.BindTextureToDescriptorHeap(
			device,
			mRenderer.GetFluidDescriptorHeap(frameIndex, static_cast<int>(Renderer::FluidStage::Advection)),
			pRtVelocityPing,
			1);//t1
		mRenderer.BindTextureToDescriptorHeap(
			device,
			mRenderer.GetFluidDescriptorHeap(frameIndex, static_cast<int>(Renderer::FluidStage::Advection)),
			pRtVelocityPing,
			2);//t2
		mRenderer.RecordPipeline(
			commandList,
			mRenderer.GetFluidPSO(frameIndex, static_cast<int>(Renderer::FluidStage::Advection)),
			mRenderer.GetFluidRootSignature(frameIndex, static_cast<int>(Renderer::FluidStage::Advection)),
			mRenderer.GetFluidDescriptorHeap(frameIndex, static_cast<int>(Renderer::FluidStage::Advection)),
			pRtVelocityPong->GetRtvHandle(),//rtv
			&mFrameFluidAdvect,
			&mScene,
			D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		SwitchPingPong(&pRtVelocityPing, &pRtVelocityPong);//ping pong
		//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^//
		///////// MY FLUID PIPELINE advect velocity /////////

		//vector<D3D12_RESOURCE_BARRIER> barrierAdvectDensity = {
		//	CD3DX12_RESOURCE_BARRIER::Transition(pRtVelocityPing->GetTextureBuffer(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, 0),
		//	CD3DX12_RESOURCE_BARRIER::Transition(pRtDensityPing->GetTextureBuffer(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, 0),
		//	CD3DX12_RESOURCE_BARRIER::Transition(pRtDensityPong->GetTextureBuffer(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET, 0)
		//};
		//commandList->ResourceBarrier(barrierAdvectDensity.size(), barrierAdvectDensity.data());

		/////////// MY FLUID PIPELINE advect density /////////
		////vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv//
		//mRenderer.BindRenderTextureToRtvDescriptorHeap(
		//	device,
		//	mRenderer.GetFluidRtvDescriptorHeap(frameIndex, static_cast<int>(Renderer::FluidStage::Advection)),
		//	pRtDensityPong,
		//	0);//rtv
		//mRenderer.BindTextureToDescriptorHeap(
		//	device,
		//	mRenderer.GetFluidDescriptorHeap(frameIndex, static_cast<int>(Renderer::FluidStage::Advection)),
		//	&mTextureObstacle,
		//	0);//t0
		//mRenderer.BindTextureToDescriptorHeap(
		//	device,
		//	mRenderer.GetFluidDescriptorHeap(frameIndex, static_cast<int>(Renderer::FluidStage::Advection)),
		//	pRtVelocityPing,
		//	1);//t1
		//mRenderer.BindTextureToDescriptorHeap(
		//	device,
		//	mRenderer.GetFluidDescriptorHeap(frameIndex, static_cast<int>(Renderer::FluidStage::Advection)),
		//	pRtDensityPing,
		//	2);//t2
		//mRenderer.RecordPipeline(
		//	commandList,
		//	mRenderer.GetFluidPSO(frameIndex, static_cast<int>(Renderer::FluidStage::Advection)),
		//	mRenderer.GetFluidRootSignature(frameIndex, static_cast<int>(Renderer::FluidStage::Advection)),
		//	mRenderer.GetFluidDescriptorHeap(frameIndex, static_cast<int>(Renderer::FluidStage::Advection)),
		//	pRtDensityPong->GetRtvHandle(),//rtv
		//	&mFrameFluidAdvect,
		//	&mScene,
		//	D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		//SwitchPingPong(&pRtDensityPing, &pRtDensityPong);//ping pong
		////^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^//
		/////////// MY FLUID PIPELINE advect density /////////

		vector<D3D12_RESOURCE_BARRIER> barrierSplat = {
			CD3DX12_RESOURCE_BARRIER::Transition(pRtVelocityPong->GetTextureBuffer(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET, 0)
		};
		commandList->ResourceBarrier(barrierSplat.size(), barrierSplat.data());

		///////// MY FLUID PIPELINE splat /////////
		//vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv//
		mRenderer.BindRenderTextureToRtvDescriptorHeap(
			device,
			mRenderer.GetFluidRtvDescriptorHeap(frameIndex, static_cast<int>(Renderer::FluidStage::Splat)),
			pRtVelocityPong,
			0);//rtv
		mRenderer.BindTextureToDescriptorHeap(
			device,
			mRenderer.GetFluidDescriptorHeap(frameIndex, static_cast<int>(Renderer::FluidStage::Splat)),
			&mTextureObstacle,
			0);//t0
		mRenderer.RecordPipelineNoClear(
			commandList,
			mRenderer.GetFluidPSO(frameIndex, static_cast<int>(Renderer::FluidStage::Splat)),
			mRenderer.GetFluidRootSignature(frameIndex, static_cast<int>(Renderer::FluidStage::Splat)),
			mRenderer.GetFluidDescriptorHeap(frameIndex, static_cast<int>(Renderer::FluidStage::Splat)),
			pRtVelocityPong->GetRtvHandle(),//rtv
			&mFrameFluidSplat,
			&mScene,
			D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		SwitchPingPong(&pRtVelocityPing, &pRtVelocityPong);//ping pong
		//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^//
		///////// MY FLUID PIPELINE splat /////////

		vector<D3D12_RESOURCE_BARRIER> barrierSplat2 = {
			CD3DX12_RESOURCE_BARRIER::Transition(pRtVelocityPong->GetTextureBuffer(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET, 0)
		};
		commandList->ResourceBarrier(barrierSplat2.size(), barrierSplat2.data());

		///////// MY FLUID PIPELINE splat 2 /////////
		//vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv//
		mRenderer.BindRenderTextureToRtvDescriptorHeap(
			device,
			mRenderer.GetFluidRtvDescriptorHeap(frameIndex, static_cast<int>(Renderer::FluidStage::Splat)),
			pRtVelocityPong,
			0);//rtv
		mRenderer.BindTextureToDescriptorHeap(
			device,
			mRenderer.GetFluidDescriptorHeap(frameIndex, static_cast<int>(Renderer::FluidStage::Splat)),
			&mTextureObstacle,
			0);//t0
		mRenderer.RecordPipelineNoClear(
			commandList,
			mRenderer.GetFluidPSO(frameIndex, static_cast<int>(Renderer::FluidStage::Splat)),
			mRenderer.GetFluidRootSignature(frameIndex, static_cast<int>(Renderer::FluidStage::Splat)),
			mRenderer.GetFluidDescriptorHeap(frameIndex, static_cast<int>(Renderer::FluidStage::Splat)),
			pRtVelocityPong->GetRtvHandle(),//rtv
			&mFrameFluidSplat,
			&mScene,
			D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		SwitchPingPong(&pRtVelocityPing, &pRtVelocityPong);//ping pong
		//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^//
		///////// MY FLUID PIPELINE splat 2 /////////

		vector<D3D12_RESOURCE_BARRIER> barrierDivergence = {
			CD3DX12_RESOURCE_BARRIER::Transition(mRenderTextureFluidDivergence.GetTextureBuffer(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET, 0),
			CD3DX12_RESOURCE_BARRIER::Transition(pRtVelocityPing->GetTextureBuffer(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, 0)
		};
		commandList->ResourceBarrier(barrierDivergence.size(), barrierDivergence.data());

		///////// MY FLUID PIPELINE divergence /////////
		//vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv//
		mRenderer.BindRenderTextureToRtvDescriptorHeap(
			device,
			mRenderer.GetFluidRtvDescriptorHeap(frameIndex, static_cast<int>(Renderer::FluidStage::ComputeDivergence)),
			&mRenderTextureFluidDivergence,
			0);//rtv
		mRenderer.BindTextureToDescriptorHeap(
			device,
			mRenderer.GetFluidDescriptorHeap(frameIndex, static_cast<int>(Renderer::FluidStage::ComputeDivergence)),
			&mTextureObstacle,
			0);//t0
		mRenderer.BindTextureToDescriptorHeap(
			device,
			mRenderer.GetFluidDescriptorHeap(frameIndex, static_cast<int>(Renderer::FluidStage::ComputeDivergence)),
			pRtVelocityPing,
			1);//t1
		mRenderer.RecordPipeline(
			commandList,
			mRenderer.GetFluidPSO(frameIndex, static_cast<int>(Renderer::FluidStage::ComputeDivergence)),
			mRenderer.GetFluidRootSignature(frameIndex, static_cast<int>(Renderer::FluidStage::ComputeDivergence)),
			mRenderer.GetFluidDescriptorHeap(frameIndex, static_cast<int>(Renderer::FluidStage::ComputeDivergence)),
			mRenderTextureFluidDivergence.GetRtvHandle(),//rtv
			&mFrameFluidDivergence,
			&mScene,
			D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^//
		///////// MY FLUID PIPELINE divergence /////////

		///////// MY FLUID PIPELINE jacobi /////////
		//vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv//
		vector<D3D12_RESOURCE_BARRIER> barrierPreJacobi = {
			CD3DX12_RESOURCE_BARRIER::Transition(pRtPressurePing->GetTextureBuffer(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET, 0)
		};
		commandList->ResourceBarrier(barrierPreJacobi.size(), barrierPreJacobi.data());

		mRenderer.BindRenderTextureToRtvDescriptorHeap(
			device,
			mRenderer.GetFluidRtvDescriptorHeap(frameIndex, static_cast<int>(Renderer::FluidStage::Jacobi)),
			pRtPressurePing,
			0);//rtv
		mRenderer.Clear(
			commandList,
			pRtPressurePing->GetRtvHandle());

		for (int i = 0; i < JacobiIteration; i++)
		{
			vector<D3D12_RESOURCE_BARRIER> barrierJacobi = {
				CD3DX12_RESOURCE_BARRIER::Transition(pRtPressurePong->GetTextureBuffer(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET, 0),
				CD3DX12_RESOURCE_BARRIER::Transition(pRtPressurePing->GetTextureBuffer(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, 0),
				CD3DX12_RESOURCE_BARRIER::Transition(mRenderTextureFluidDivergence.GetTextureBuffer(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, 0)
			};
			commandList->ResourceBarrier(barrierJacobi.size(), barrierJacobi.data());

			mRenderer.BindRenderTextureToRtvDescriptorHeap(
				device,
				mRenderer.GetFluidRtvDescriptorHeap(frameIndex, static_cast<int>(Renderer::FluidStage::Jacobi)),
				pRtPressurePong,
				0);//rtv
			mRenderer.BindTextureToDescriptorHeap(
				device,
				mRenderer.GetFluidDescriptorHeap(frameIndex, static_cast<int>(Renderer::FluidStage::Jacobi)),
				&mTextureObstacle,
				0);//t0
			mRenderer.BindTextureToDescriptorHeap(
				device,
				mRenderer.GetFluidDescriptorHeap(frameIndex, static_cast<int>(Renderer::FluidStage::Jacobi)),
				pRtPressurePing,
				1);//t1
			mRenderer.BindTextureToDescriptorHeap(
				device,
				mRenderer.GetFluidDescriptorHeap(frameIndex, static_cast<int>(Renderer::FluidStage::Jacobi)),
				&mRenderTextureFluidDivergence,
				2);//t2
			mRenderer.RecordPipeline(
				commandList,
				mRenderer.GetFluidPSO(frameIndex, static_cast<int>(Renderer::FluidStage::Jacobi)),
				mRenderer.GetFluidRootSignature(frameIndex, static_cast<int>(Renderer::FluidStage::Jacobi)),
				mRenderer.GetFluidDescriptorHeap(frameIndex, static_cast<int>(Renderer::FluidStage::Jacobi)),
				pRtPressurePong->GetRtvHandle(),//rtv
				&mFrameFluidJacobi,
				&mScene,
				D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			SwitchPingPong(&pRtPressurePing, &pRtPressurePong);
		}
		//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^//
		///////// MY FLUID PIPELINE jacobi /////////

		vector<D3D12_RESOURCE_BARRIER> barrierGradient = {
				CD3DX12_RESOURCE_BARRIER::Transition(pRtVelocityPong->GetTextureBuffer(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET, 0),
				CD3DX12_RESOURCE_BARRIER::Transition(pRtPressurePing->GetTextureBuffer(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, 0),
				CD3DX12_RESOURCE_BARRIER::Transition(pRtVelocityPing->GetTextureBuffer(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, 0)
		};
		commandList->ResourceBarrier(barrierGradient.size(), barrierGradient.data());

		///////// MY FLUID PIPELINE gradient /////////
		//vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv//
		mRenderer.BindRenderTextureToRtvDescriptorHeap(
			device,
			mRenderer.GetFluidRtvDescriptorHeap(frameIndex, static_cast<int>(Renderer::FluidStage::SubtractGradient)),
			pRtVelocityPong,
			0);//rtv
		mRenderer.BindTextureToDescriptorHeap(
			device,
			mRenderer.GetFluidDescriptorHeap(frameIndex, static_cast<int>(Renderer::FluidStage::SubtractGradient)),
			&mTextureObstacle,
			0);//t0
		mRenderer.BindTextureToDescriptorHeap(
			device,
			mRenderer.GetFluidDescriptorHeap(frameIndex, static_cast<int>(Renderer::FluidStage::SubtractGradient)),
			pRtPressurePing,
			1);//t1
		mRenderer.BindTextureToDescriptorHeap(
			device,
			mRenderer.GetFluidDescriptorHeap(frameIndex, static_cast<int>(Renderer::FluidStage::SubtractGradient)),
			pRtVelocityPing,
			2);//t2
		mRenderer.RecordPipeline(
			commandList,
			mRenderer.GetFluidPSO(frameIndex, static_cast<int>(Renderer::FluidStage::SubtractGradient)),
			mRenderer.GetFluidRootSignature(frameIndex, static_cast<int>(Renderer::FluidStage::SubtractGradient)),
			mRenderer.GetFluidDescriptorHeap(frameIndex, static_cast<int>(Renderer::FluidStage::SubtractGradient)),
			pRtVelocityPong->GetRtvHandle(),//rtv
			&mFrameFluidGradient,
			&mScene,
			D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		SwitchPingPong(&pRtVelocityPing, &pRtVelocityPong);
		//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^//
		///////// MY FLUID PIPELINE gradient /////////
	}

	vector<D3D12_RESOURCE_BARRIER> barrierGraphicsToWaveParticle = {
		CD3DX12_RESOURCE_BARRIER::Transition(mRenderTextureWaveParticle.GetTextureBuffer(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET, 0)
	};
	commandList->ResourceBarrier(barrierGraphicsToWaveParticle.size(), barrierGraphicsToWaveParticle.data());

	///////// MY WAVE PARTICLE PIPELINE /////////
	//vvvvvvvvvvvvvvvvvvvvvvvvvvv//
	mRenderer.RecordPipeline(
		commandList,
		mRenderer.GetWaveParticlePSO(static_cast<int>(Renderer::WaveParticleStage::Default)),
		mRenderer.GetWaveParticleRootSignature(static_cast<int>(Renderer::WaveParticleStage::Default)),
		mRenderer.GetWaveParticleDescriptorHeap(static_cast<int>(Renderer::WaveParticleStage::Default)),
		mRenderer.GetRtvHandle(frameIndex),
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
	mRenderer.RecordPipeline(
		commandList,
		mRenderer.GetPostProcessPSO(static_cast<int>(Renderer::PostProcessStage::Horizontal)),
		mRenderer.GetPostProcessRootSignature(static_cast<int>(Renderer::PostProcessStage::Horizontal)),
		mRenderer.GetPostProcessDescriptorHeap(static_cast<int>(Renderer::PostProcessStage::Horizontal)),
		mRenderer.GetRtvHandle(frameIndex),
		&mFramePostProcessH,
		&mScene,
		D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//^^^^^^^^^^^^^^^^^^^^^^^^^^^//
	///////// MY POSTPROCESS H PIPELINE /////////

	vector<D3D12_RESOURCE_BARRIER> barrierPostprocessHToPostprocessV = {
		CD3DX12_RESOURCE_BARRIER::Transition(mRenderTexturePostProcessH1.GetTextureBuffer(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, 0),
		CD3DX12_RESOURCE_BARRIER::Transition(mRenderTexturePostProcessH2.GetTextureBuffer(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, 0),
		CD3DX12_RESOURCE_BARRIER::Transition(mRenderTexturePostProcessV1.GetTextureBuffer(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET, 0),
		CD3DX12_RESOURCE_BARRIER::Transition(mRenderTexturePostProcessV2.GetTextureBuffer(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET, 0),
	};
	commandList->ResourceBarrier(barrierPostprocessHToPostprocessV.size(), barrierPostprocessHToPostprocessV.data());

	///////// MY POSTPROCESS V PIPELINE /////////
	//vvvvvvvvvvvvvvvvvvvvvvvvvvv//
	mRenderer.RecordPipeline(
		commandList,
		mRenderer.GetPostProcessPSO(static_cast<int>(Renderer::PostProcessStage::Vertical)),
		mRenderer.GetPostProcessRootSignature(static_cast<int>(Renderer::PostProcessStage::Vertical)),
		mRenderer.GetPostProcessDescriptorHeap(static_cast<int>(Renderer::PostProcessStage::Vertical)),
		mRenderer.GetRtvHandle(frameIndex),
		&mFramePostProcessV,
		&mScene,
		D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//^^^^^^^^^^^^^^^^^^^^^^^^^^^//
	///////// MY POSTPROCESS V PIPELINE /////////

	vector<D3D12_RESOURCE_BARRIER> barrierPostprocessVToGraphics = {
		CD3DX12_RESOURCE_BARRIER::Transition(mRenderTexturePostProcessV1.GetTextureBuffer(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, 0),
		CD3DX12_RESOURCE_BARRIER::Transition(mRenderTexturePostProcessV2.GetTextureBuffer(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, 0),
		CD3DX12_RESOURCE_BARRIER::Transition(pRtVelocityPing->GetTextureBuffer(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, 0),
		//CD3DX12_RESOURCE_BARRIER::Transition(pRtPressurePing->GetTextureBuffer(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, 0)
	};
	commandList->ResourceBarrier(barrierPostprocessVToGraphics.size(), barrierPostprocessVToGraphics.data());
	
	///////// MY GRAPHICS PIPELINE /////////
	//vvvvvvvvvvvvvvvvvvvvvvvvvvv//
	mRenderer.BindTextureToDescriptorHeap(
		device,
		mRenderer.GetGraphicsDescriptorHeap(static_cast<int>(Renderer::GraphicsStage::Default)),
		pRtVelocityPing,
		//pRtPressurePing,
		1);//t1
	mRenderer.RecordPipeline(
		commandList,
		mRenderer.GetGraphicsPSO(static_cast<int>(Renderer::GraphicsStage::Default)),
		mRenderer.GetGraphicsRootSignature(static_cast<int>(Renderer::GraphicsStage::Default)),
		mRenderer.GetGraphicsDescriptorHeap(static_cast<int>(Renderer::GraphicsStage::Default)),
		mRenderer.GetRtvHandle(frameIndex),
		&mFrameGraphics,
		&mScene,
		D3D_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST,
		XMFLOAT4(0.0, 0.2, 0.4, 1.0));
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

	static int mode = mScene.GetUniformMode();
	static float heightScale = mScene.GetUniformHeightScale();
	static float dxScale = mScene.GetUniformDxScale();
	static float dzScale = mScene.GetUniformDzScale();
	static int blurRadius = mScene.GetUniformBlurRadius();
	static float waveParticleSpeedScale = mScene.GetUniformWaveParticleSpeedScale();
	static float flowSpeed = mScene.GetUniformFlowSpeed();
	static float timeScale = mScene.GetUniformTimeScale();
	static float timeStepFluid = mScene.GetUniformTimeStepFluid();
	static float jacobiObstacleScale = mScene.GetUniformJacobiObstacleScale();
	static float fluidCellSize = mScene.GetUniformFluidCellSize();
	static float jacobiInvBeta = mScene.GetUniformJacobiInvBeta();
	static float fluidDissipation = mScene.GetUniformFluidDissipation();
	static float gradientScale = mScene.GetUniformGradientScale();
	static float splatDirU = mScene.GetUniformSplatDirU();
	static float splatDirV = mScene.GetUniformSplatDirV();
	static float splatScale = mScene.GetUniformSplatScale();
	static int edgeTess = mScene.GetUniformEdgeTessFactor();
	static int insideTess = mScene.GetUniformInsideTessFactor();
	static int celldiv = 3;
	bool needToUpdateSceneUniform = false;

	ImGui::SetNextWindowPos(ImVec2(0, 0));

	ImGui::Begin("Control Panel ");                        
	ImGui::Text("Wave Particles Scale ");

	if (ImGui::Combo("mode", &mode, "default\0flow map\0flow map driven texture\0wave particle\0horizontal blur\0vertical blur\0horizontal and vertical blur\0normal\0\0"))
	{
		mScene.SetUniformMode(mode);
		needToUpdateSceneUniform = true;
	}

	if (ImGui::SliderFloat("height ", &heightScale, 0.0f, 3.0f, "%.6f"))
	{
		mScene.SetUniformHeightScale(heightScale);
		needToUpdateSceneUniform = true;
	}

	if (ImGui::SliderFloat("dx ", &dxScale, 0.0f, 0.13f, "%.6f"))
	{
		mScene.SetUniformDxScale(dxScale);
		needToUpdateSceneUniform = true;
	}
	
	if (ImGui::SliderFloat("dz ", &dzScale, 0.0f, 0.13f, "%.6f"))
	{
		mScene.SetUniformDzScale(dzScale);
		needToUpdateSceneUniform = true;
	}

	if (ImGui::SliderInt("blur radius ", &blurRadius, 0, 500))
	{
		mScene.SetUniformBlurRadius(blurRadius);
		needToUpdateSceneUniform = true;
	}

	if (ImGui::SliderFloat("wpSpeed ", &waveParticleSpeedScale, 0.0f, 0.0005f, "%.6f"))
	{
		mScene.SetUniformWaveParticleSpeedScale(waveParticleSpeedScale);
		needToUpdateSceneUniform = true;
	}

	if (ImGui::SliderFloat("flowSpeed ", &flowSpeed, 0.f, 0.0005f, "%.6f"))
	{
		mScene.SetUniformFlowSpeed(flowSpeed);
		needToUpdateSceneUniform = true;
	}

	if (ImGui::SliderFloat("timeScale ", &timeScale, 0.0f, 100.0f, "%.6f"))
	{
		mScene.SetUniformTimeScale(timeScale);
		needToUpdateSceneUniform = true;
	}

	ImGui::Checkbox("fluidSim ", &FluidSimulation);

	if (ImGui::SliderFloat("timeScaleFluid ", &timeStepFluid, 0.0f, 0.03, "%.6f"))
	{
		mScene.SetUniformTimeStepFluid(timeStepFluid);
		needToUpdateSceneUniform = true;
	}

	if (ImGui::SliderFloat("fluidDissipation ", &fluidDissipation, 0.9f, 1.1f, "%.6f"))
	{
		mScene.SetUniformFluidDissipation(fluidDissipation);
		needToUpdateSceneUniform = true;
	}

	if (ImGui::SliderFloat("gradientScale ", &gradientScale, 0.01f, 2.0f, "%.6f"))
	{
		mScene.SetUniformGradientScale(gradientScale);
		needToUpdateSceneUniform = true;
	}

	if (ImGui::SliderFloat("fluidCellSize ", &fluidCellSize, 0.9f, 1.3f, "%.6f"))
	{
		mScene.SetUniformFluidCellSize(fluidCellSize);
		needToUpdateSceneUniform = true;
	}

	ImGui::SliderInt("JacobiIteration ", &JacobiIteration, 10, 100);

	if (ImGui::SliderFloat("jacobiObstacleScale ", &jacobiObstacleScale, 0.0f, 20.0f, "%.6f"))
	{
		mScene.SetUniformJacobiObstacleScale(jacobiObstacleScale);
		needToUpdateSceneUniform = true;
	}

	if (ImGui::SliderFloat("jacobiInvBeta ", &jacobiInvBeta, 0.0f, 0.5f, "%.6f"))
	{
		mScene.SetUniformJacobiInvBeta(jacobiInvBeta);
		needToUpdateSceneUniform = true;
	}

	if (ImGui::SliderFloat("splatDirU ", &splatDirU, -1.0f, 1.0f, "%.6f"))
	{
		mScene.SetUniformSplatDirU(splatDirU);
		needToUpdateSceneUniform = true;
	}

	if (ImGui::SliderFloat("splatDirV ", &splatDirV, -1.0f, 1.0f, "%.6f"))
	{
		mScene.SetUniformSplatDirV(splatDirV);
		needToUpdateSceneUniform = true;
	}

	if (ImGui::SliderFloat("splatScale ", &splatScale, 0.0f, 0.02f, "%.6f"))
	{
		mScene.SetUniformSplatScale(splatScale);
		needToUpdateSceneUniform = true;
	}

	if (ImGui::SliderInt("edge tess ", &edgeTess, 0, 32))
	{
		mScene.SetUniformEdgeTessFactor(edgeTess);
		needToUpdateSceneUniform = true;
	}

	if (ImGui::SliderInt("inside tess ", &insideTess, 0, 32))
	{
		mScene.SetUniformInsideTessFactor(insideTess);
		needToUpdateSceneUniform = true;
	}

	if (ImGui::SliderInt("fluidtexsize", &celldiv, 1, 100))
	{
		mScene.SetUniformTextureWidthHeightFluid(WidthRtFluid / celldiv, HeightRtFluid / celldiv);
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