#include <windows.h>
#include <wincodec.h>
#include <dinput.h>
#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx12.h"
#include "Scene.h"
#include "Renderer.h"

HWND hwnd = NULL; // Handle to the window
LPCTSTR WindowName = L"WPWIV"; // name of the window (not the title)
LPCTSTR WindowTitle = L"WPWIV_1.0"; // title of the window
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
#ifdef MY_DEBUG
ID3D12Debug* debugController;
#endif
ID3D12CommandAllocator* commandAllocator[FrameBufferCount]; // we want enough allocators for each buffer * number of threads (we only have one thread)
ID3D12GraphicsCommandList* commandList; // a command list we can record commands into, then execute them to render the frame
ID3D12Fence* fence[FrameBufferCount];    
HANDLE fenceEvent; // a handle to an event when our fence is unlocked by the gpu
UINT64 fenceValue[FrameBufferCount]; // this value is incremented each frame. each fence will have its own value
int frameIndex; // current rtv we are on
uint32_t frameCount = 0;

int Width = 1024; // width and height of the window
int Height = 768;
int WidthRT = 500; // can not exceed min(Width, Height) because we only have one DSV for all pipeline
int HeightRT = 500; // can not exceed min(Width, Height) because we only have one DSV for all pipeline
int WidthRtFluid = 500; // can not exceed min(Width, Height) because we only have one DSV for all pipeline
int HeightRtFluid = 500; // can not exceed min(Width, Height) because we only have one DSV for all pipeline
bool FluidSimulation = true;
int FluidSimulationStep = 30;
int fluidSimulationStep = 0;
const float WaterSurfaceScaleX = 10;
const float WaterSurfaceScaleZ = 10;
const float WaterSurfacePosX = -5;
const float WaterSurfacePosZ = -5;
bool CreateObstacle = false;
bool ClearObstacle = false;

Renderer mRenderer;
Scene mScene;
Frame mFrameCreateObstacle;
Frame mFrameObstacleHorizontal;
Frame mFrameObstacleVertical;
Frame mFrameObstacle;
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
Mesh mWaterSurface(Mesh::MeshType::TileableSurface, 100, 100, XMFLOAT3(WaterSurfacePosX, 0, WaterSurfacePosZ), XMFLOAT3(0, 0, 0), XMFLOAT3(WaterSurfaceScaleX, 1, WaterSurfaceScaleZ));
Mesh mObstacleSurface(Mesh::MeshType::TileableSurface, 100, 100, XMFLOAT3(WaterSurfacePosX, -0.1, WaterSurfacePosZ), XMFLOAT3(0, 0, 0), XMFLOAT3(WaterSurfaceScaleX, 1, WaterSurfaceScaleZ));
Mesh mQuad(Mesh::MeshType::FullScreenQuad, XMFLOAT3(0, 0, 0), XMFLOAT3(0, 0, 0), XMFLOAT3(1, 1, 1));
Mesh mWaveParticle(Mesh::MeshType::WaveParticle, 6000, XMFLOAT3(0, 0, 0), XMFLOAT3(0, 0, 0), XMFLOAT3(1, 1, 1));
Mesh mCircle(Mesh::MeshType::Circle, 16, XMFLOAT3{ 0,0,0 }, XMFLOAT3{ 0,0,0 }, XMFLOAT3{ 1,1,1 });
Shader mCreateObstacleVS(Shader::ShaderType::VertexShader, L"CreateObstacleVS.hlsl");
Shader mCreateObstaclePS(Shader::ShaderType::PixelShader, L"CreateObstaclePS.hlsl");
Shader mObstacleHorizontalPS(Shader::ShaderType::PixelShader, L"PostProcessObstaclePS_H.hlsl");
Shader mObstacleVerticalPS(Shader::ShaderType::PixelShader, L"PostProcessObstaclePS_V.hlsl");
Shader mVertexShader(Shader::ShaderType::VertexShader, L"VertexShader.hlsl");
Shader mHullShader(Shader::ShaderType::HullShader, L"HullShader.hlsl");
Shader mDomainShader(Shader::ShaderType::DomainShader, L"DomainShader.hlsl");
Shader mPixelShader(Shader::ShaderType::PixelShader, L"PixelShader.hlsl");
Shader mObstacleVS(Shader::ShaderType::VertexShader, L"ObstacleVS.hlsl");
Shader mObstacleHS(Shader::ShaderType::HullShader, L"ObstacleHS.hlsl");
Shader mObstacleDS(Shader::ShaderType::DomainShader, L"ObstacleDS.hlsl");
Shader mObstaclePS(Shader::ShaderType::PixelShader, L"ObstaclePS.hlsl");
Shader mPostProcessVS(Shader::ShaderType::VertexShader, L"PostProcessVS.hlsl");
Shader mPostProcessPS_H(Shader::ShaderType::PixelShader, L"PostProcessPS_H.hlsl");
Shader mPostProcessPS_V(Shader::ShaderType::PixelShader, L"PostProcessPS_V.hlsl");
Shader mWaveParticleVS(Shader::ShaderType::VertexShader, L"WaveParticleVS.hlsl");
Shader mWaveParticlePS(Shader::ShaderType::PixelShader, L"WaveParticlePS.hlsl");
Shader mFluidAdvectPS(Shader::ShaderType::PixelShader, L"AdvectPS.hlsl");
Shader mFluidSplatVelocityPS(Shader::ShaderType::PixelShader, L"SplatVelocityWithVorticityPS.hlsl");
Shader mFluidSplatDensityPS(Shader::ShaderType::PixelShader, L"SplatDensityPS.hlsl");
Shader mFluidComputeDivergencePS(Shader::ShaderType::PixelShader, L"ComputeDivergencePS.hlsl");
Shader mFluidJacobiPS(Shader::ShaderType::PixelShader, L"JacobiPS.hlsl");
Shader mFluidSubtractGradientPS(Shader::ShaderType::PixelShader, L"SubtractGradientPS.hlsl");
Texture mTextureAlbedo(L"foam.jpg");
RenderTexture mRenderTextureWaveParticle(WidthRT, HeightRT, L"WaveParticle", DXGI_FORMAT_R16G16B16A16_FLOAT);
RenderTexture mRenderTexturePostProcessH1(WidthRT, HeightRT, L"PostProcessH1", DXGI_FORMAT_R16G16B16A16_FLOAT);
RenderTexture mRenderTexturePostProcessH2(WidthRT, HeightRT, L"PostProcessH2", DXGI_FORMAT_R16G16B16A16_FLOAT);
RenderTexture mRenderTexturePostProcessV1(WidthRT, HeightRT, L"PostProcessV1", DXGI_FORMAT_R16G16B16A16_FLOAT);
RenderTexture mRenderTexturePostProcessV2(WidthRT, HeightRT, L"PostProcessV2", DXGI_FORMAT_R16G16B16A16_FLOAT);
RenderTexture mRenderTextureObstacleCreate(WidthRtFluid, HeightRtFluid, L"ObstacleCreate", DXGI_FORMAT_R8G8B8A8_UNORM);
RenderTexture mRenderTextureObstacleBlur(WidthRtFluid, HeightRtFluid, L"ObstacleBlur", DXGI_FORMAT_R8G8B8A8_UNORM);
RenderTexture mRenderTextureObstacle(WidthRtFluid, HeightRtFluid, L"Obstacle", DXGI_FORMAT_R8G8B8A8_UNORM);
RenderTexture mRenderTextureFluidDivergence(WidthRtFluid, HeightRtFluid, L"FluidDivergence", DXGI_FORMAT_R16G16B16A16_FLOAT);
RenderTexture mRenderTextureFluidVelocity1(WidthRtFluid, HeightRtFluid, L"FluidVelocity1", DXGI_FORMAT_R16G16B16A16_FLOAT);
RenderTexture mRenderTextureFluidVelocity2(WidthRtFluid, HeightRtFluid, L"FluidVelocity2", DXGI_FORMAT_R16G16B16A16_FLOAT);
RenderTexture mRenderTextureFluidDensity1(WidthRtFluid, HeightRtFluid, L"FluidDensity1", DXGI_FORMAT_R16G16B16A16_FLOAT);
RenderTexture mRenderTextureFluidDensity2(WidthRtFluid, HeightRtFluid, L"FluidDensity2", DXGI_FORMAT_R16G16B16A16_FLOAT);
RenderTexture mRenderTextureFluidPressure1(WidthRtFluid, HeightRtFluid, L"FluidPressure1", DXGI_FORMAT_R16G16B16A16_FLOAT);
RenderTexture mRenderTextureFluidPressure2(WidthRtFluid, HeightRtFluid, L"FluidPressure2", DXGI_FORMAT_R16G16B16A16_FLOAT);
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
	mFrameWaveParticle.SetUniformTime(0);

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
	mFrameGraphics.AddTexture(&mRenderTextureFluidVelocity1);//t1
	mFrameGraphics.AddTexture(&mRenderTexturePostProcessV1);//t2
	mFrameGraphics.AddTexture(&mRenderTexturePostProcessV2);//t3
	mFrameGraphics.AddTexture(&mRenderTextureFluidDensity1);//t4
	mFrameGraphics.AddTexture(&mRenderTextureFluidPressure1);//t5
	mFrameGraphics.AddTexture(&mRenderTextureFluidDivergence);//t6
	mFrameGraphics.AddTexture(&mRenderTextureObstacle);//t7
	mFrameGraphics.SetUniformTime(0);

	// create obstacle
	mFrameCreateObstacle.AddCamera(&mDummyCameraFluid);//same view port rect as fluid
	mFrameCreateObstacle.AddMesh(&mCircle);
	mFrameCreateObstacle.AddRenderTexture(&mRenderTextureObstacleCreate);

	// obstacle
	mFrameObstacle.AddCamera(&mCamera);
	mFrameObstacle.AddMesh(&mObstacleSurface);
	mFrameObstacle.AddTexture(&mRenderTextureObstacle);//t0
	mFrameObstacle.AddTexture(&mRenderTextureObstacleCreate);//t1
	mFrameObstacle.AddTexture(&mRenderTextureObstacleBlur);//t2

	// obstacle filter
	mFrameObstacleHorizontal.AddCamera(&mDummyCameraFluid);
	mFrameObstacleHorizontal.AddMesh(&mQuad);
	mFrameObstacleHorizontal.AddTexture(&mRenderTextureObstacleCreate);
	mFrameObstacleHorizontal.AddRenderTexture(&mRenderTextureObstacleBlur);

	mFrameObstacleVertical.AddCamera(&mDummyCameraFluid);
	mFrameObstacleVertical.AddMesh(&mQuad);
	mFrameObstacleVertical.AddTexture(&mRenderTextureObstacleBlur);
	mFrameObstacleVertical.AddRenderTexture(&mRenderTextureObstacle);

	// B. Data

	// ping pong
	pRtVelocityPing = &mRenderTextureFluidVelocity1;
	pRtVelocityPong = &mRenderTextureFluidVelocity2;
	pRtDensityPing = &mRenderTextureFluidDensity1;
	pRtDensityPong = &mRenderTextureFluidDensity2;
	pRtPressurePing = &mRenderTextureFluidPressure1;
	pRtPressurePong = &mRenderTextureFluidPressure2;

	// frame
	mScene.AddFrame(&mFrameObstacleHorizontal);
	mScene.AddFrame(&mFrameObstacleVertical);
	mScene.AddFrame(&mFrameObstacle);
	mScene.AddFrame(&mFrameCreateObstacle);
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
	mScene.AddMesh(&mObstacleSurface);
	mScene.AddMesh(&mCircle);
	mScene.AddMesh(&mQuad);
	mScene.AddMesh(&mWaveParticle);
	mScene.AddMesh(&mWaterSurface);

	// shader
	mScene.AddShader(&mObstacleHorizontalPS);
	mScene.AddShader(&mObstacleVerticalPS);
	mScene.AddShader(&mObstacleVS);
	mScene.AddShader(&mObstacleHS);
	mScene.AddShader(&mObstacleDS);
	mScene.AddShader(&mObstaclePS);
	mScene.AddShader(&mCreateObstacleVS);
	mScene.AddShader(&mCreateObstaclePS);
	mScene.AddShader(&mFluidAdvectPS);
	mScene.AddShader(&mFluidSplatVelocityPS);
	mScene.AddShader(&mFluidSplatDensityPS);
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
	//mScene.AddTexture(&mTextureObstacle);
	
	// render texture
	mScene.AddRenderTexture(&mRenderTextureObstacleCreate);
	mScene.AddRenderTexture(&mRenderTextureObstacleBlur);
	mScene.AddRenderTexture(&mRenderTextureObstacle);
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
	mScene.SetUniformHeightScale(0.14);
	mScene.SetUniformWaveParticleSpeedScale(0.00005);
	mScene.SetUniformFlowSpeed(0.000931);
	mScene.SetUniformDxScale(0.03);
	mScene.SetUniformDzScale(0.03);
	mScene.SetUniformTimeScale(1.3);
	mScene.SetUniformFoamScale(5.0);

	mScene.SetUniformTimeStepFluid(0.03);
	mScene.SetUniformFluidCellSize(0.6);
	mScene.SetUniformFluidDissipation(0.994);
	mScene.SetUniformVorticityScale(0.64);
	mScene.SetUniformSplatDirU(1.0);
	mScene.SetUniformSplatDirV(0.0);

	mScene.SetUniformSplatScale(0.00593);
	mScene.SetUniformSplatDensityU(0.0);
	mScene.SetUniformSplatDensityV(0.0);
	mScene.SetUniformSplatDensityRadius(0.1);
	mScene.SetUniformSplatDensityScale(0.01);

	mScene.SetUniformBrushScale(0.1);
	mScene.SetUniformBrushStrength(1.0);
	mScene.SetUniformBrushOffsetU(0.0);
	mScene.SetUniformBrushOffsetV(0.0);

	mScene.SetUniformObstacleScale(1.8);
	mScene.SetUniformObstacleThresholdFluid(0.3);
	mScene.SetUniformObstacleThresholdWave(0.12);

	mScene.SetUniformTextureWidthHeight(WidthRT, HeightRT);
	mScene.SetUniformTextureWidthHeightFluid(WidthRtFluid / 3.5, HeightRtFluid / 3.5);
	mScene.SetUniformEdgeTessFactor(7);
	mScene.SetUniformInsideTessFactor(5);
	mScene.SetUniformBlurRadius(15);
	mScene.SetUniformMode(11);

	mScene.SetUniformLightHight(9.35);
	mScene.SetUniformExtinctcoeff(-0.41);
	mScene.SetShiness(340);
	mScene.SetBias(0);
	mScene.SetFPow(3);
	mScene.SetFScale(0.68);
	mScene.SetFoamScale(9.6);

	if (!mScene.LoadScene())
		return false;
	
	return true;
}

#ifdef MY_DEBUG
bool EnableDebugLayer()
{
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
	{
		debugController->EnableDebugLayer();
		return true;
	}
	return false;
}
#endif

#ifdef MY_DEBUG
bool EnableShaderBasedValidation()
{
	ID3D12Debug* spDebugController0;
	ID3D12Debug1* spDebugController1;
	HRESULT hr;

	hr = D3D12GetDebugInterface(IID_PPV_ARGS(&spDebugController0));
	if (!CheckError(hr, nullptr)) return false;

	hr = spDebugController0->QueryInterface(IID_PPV_ARGS(&spDebugController1));
	if (!CheckError(hr, nullptr)) return false;

	spDebugController1->SetEnableGPUBasedValidation(true);
	return true;
}
#endif

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
	memset(keyboardCurrState, 0, sizeof(keyboardCurrState));//initialization is important, sometimes the value of unpressed key will not be changed

	DIKeyboard->Acquire();

	DIKeyboard->GetDeviceState(sizeof(keyboardCurrState), (LPVOID)&keyboardCurrState);

	POINT currentCursorPos = {};
	GetCursorPos(&currentCursorPos);
	ScreenToClient(hwnd, &currentCursorPos);

	int mouseX = currentCursorPos.x;
	int mouseY = currentCursorPos.y;

	//keyboard control
	//this is handled in mainloop, need to do this here again
	//if (KEYDOWN(keyboardCurrState, DIK_ESCAPE))
	//{
	//	PostMessage(hwnd, WM_DESTROY, 0, 0);
	//}

	//mouse control
	if (KEYDOWN(keyboardCurrState, DIK_C))//control camera
	{
		if (!mouseAcquired)
		{
			DIMouse->Acquire();
			mouseAcquired = true;
		}
	}
	else if (KEYDOWN(keyboardCurrState, DIK_B))//control brush
	{
		XMFLOAT2 screen = { static_cast<float>(mouseX), static_cast<float>(mouseY) };
		XMFLOAT3 world = mCamera.ScreenToWorld(screen);
		XMFLOAT3 ori = mCamera.GetPosition();
		XMFLOAT3 dir = {};
		XMVECTOR oriV = XMLoadFloat3(&ori);
		XMVECTOR worldV = XMLoadFloat3(&world);
		XMVECTOR dirV = XMVector3Normalize(worldV - oriV);
		XMStoreFloat3(&dir, dirV);
		XMFLOAT3 nor = { 0, 1, 0 };
		XMFLOAT3 p = { 0, 0, 0 };
		float t = Mesh::RayPlaneIntersection(ori, dir, nor, p);
		XMVECTOR resultV = oriV + dirV * t;
		XMFLOAT3 result = {};
		XMStoreFloat3(&result, resultV);
		//printf("plane:%f,%f,%f\n", result.x, result.y, result.z);
		mScene.SetUniformBrushOffsetU((result.x - WaterSurfacePosX) / WaterSurfaceScaleX * 2.f - 1.f);
		mScene.SetUniformBrushOffsetV(-((result.z - WaterSurfacePosZ) / WaterSurfaceScaleZ * 2.f - 1.f));
		mScene.UpdateUniformBuffer();
		CreateObstacle = true;
	}
	else
	{
		DIMouse->Unacquire();
		mouseAcquired = false;
		CreateObstacle = false;
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
	
	memcpy(keyboardLastState, keyboardCurrState, sizeof(keyboardLastState));

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

	commandQueue->SetName(L"WPWIV Command Queue");

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
			mRenderer.GetFluidRootSignaturePtr(i, static_cast<int>(Renderer::FluidStage::AdvectVelocity)),
			3))
		{
			printf("Create Fluid RS advection failed\n");
			return false;
		}

		if (!mRenderer.CreateHeap(
			device,
			mRenderer.GetFluidDescriptorHeapPtr(i, static_cast<int>(Renderer::FluidStage::AdvectVelocity)),
			mRenderer.GetFluidRtvDescriptorHeapPtr(i, static_cast<int>(Renderer::FluidStage::AdvectVelocity)),
			mRenderer.GetFluidDsvDescriptorHeapPtr(i, static_cast<int>(Renderer::FluidStage::AdvectVelocity)),
			3,
			1))
		{
			printf("Create Fluid Heap advection failed\n");
			return false;
		}

		if (!mRenderer.CreatePSO(
			device,
			mRenderer.GetFluidPsoPtr(i, static_cast<int>(Renderer::FluidStage::AdvectVelocity)),
			mRenderer.GetFluidRootSignature(i, static_cast<int>(Renderer::FluidStage::AdvectVelocity)),
			D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
			CD3DX12_BLEND_DESC(D3D12_DEFAULT),
			Renderer::NoDepthTest(),
			DXGI_FORMAT_R16G16B16A16_FLOAT,
			DXGI_FORMAT_D32_FLOAT,
			1,
			&mPostProcessVS,
			nullptr,
			nullptr,
			nullptr,
			&mFluidAdvectPS,
			L"fluid-advect-velocity"))
		{
			printf("Create Fluid Pipeline PSO advection failed\n");
			return false;
		}
	}

	// density advection
	for (int i = 0; i < FrameBufferCount; i++)
	{
		if (!mRenderer.CreateFluidRootSignature(
			device,
			mRenderer.GetFluidRootSignaturePtr(i, static_cast<int>(Renderer::FluidStage::AdvectDensity)),
			3))
		{
			printf("Create Fluid RS advection failed\n");
			return false;
		}

		if (!mRenderer.CreateHeap(
			device,
			mRenderer.GetFluidDescriptorHeapPtr(i, static_cast<int>(Renderer::FluidStage::AdvectDensity)),
			mRenderer.GetFluidRtvDescriptorHeapPtr(i, static_cast<int>(Renderer::FluidStage::AdvectDensity)),
			mRenderer.GetFluidDsvDescriptorHeapPtr(i, static_cast<int>(Renderer::FluidStage::AdvectDensity)),
			3,
			1))
		{
			printf("Create Fluid Heap advection failed\n");
			return false;
		}

		if (!mRenderer.CreatePSO(
			device,
			mRenderer.GetFluidPsoPtr(i, static_cast<int>(Renderer::FluidStage::AdvectDensity)),
			mRenderer.GetFluidRootSignature(i, static_cast<int>(Renderer::FluidStage::AdvectDensity)),
			D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
			CD3DX12_BLEND_DESC(D3D12_DEFAULT),
			Renderer::NoDepthTest(),
			DXGI_FORMAT_R16G16B16A16_FLOAT,
			DXGI_FORMAT_D32_FLOAT,
			1,
			&mPostProcessVS,
			nullptr,
			nullptr,
			nullptr,
			&mFluidAdvectPS,
			L"fluid-advect-density"))
		{
			printf("Create Fluid Pipeline PSO advection failed\n");
			return false;
		}
	}

	// fluid splat velocity
	for (int i = 0; i < FrameBufferCount; i++)
	{
		if (!mRenderer.CreateFluidRootSignature(
			device,
			mRenderer.GetFluidRootSignaturePtr(i, static_cast<int>(Renderer::FluidStage::SplatVelocity)),
			2))
		{
			printf("Create Fluid RS splat failed\n");
			return false;
		}

		if (!mRenderer.CreateHeap(
			device,
			mRenderer.GetFluidDescriptorHeapPtr(i, static_cast<int>(Renderer::FluidStage::SplatVelocity)),
			mRenderer.GetFluidRtvDescriptorHeapPtr(i, static_cast<int>(Renderer::FluidStage::SplatVelocity)),
			mRenderer.GetFluidDsvDescriptorHeapPtr(i, static_cast<int>(Renderer::FluidStage::SplatVelocity)),
			2,
			1))
		{
			printf("Create Fluid Heap splat failed\n");
			return false;
		}

		if (!mRenderer.CreatePSO(
			device,
			mRenderer.GetFluidPsoPtr(i, static_cast<int>(Renderer::FluidStage::SplatVelocity)),
			mRenderer.GetFluidRootSignature(i, static_cast<int>(Renderer::FluidStage::SplatVelocity)),
			D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
			CD3DX12_BLEND_DESC(D3D12_DEFAULT), //mRenderer.AdditiveBlend(),
			Renderer::NoDepthTest(),
			DXGI_FORMAT_R16G16B16A16_FLOAT,
			DXGI_FORMAT_D32_FLOAT,
			1,
			&mPostProcessVS,
			nullptr,
			nullptr,
			nullptr,
			&mFluidSplatVelocityPS,
			L"fluid-splat-velocity"))
		{
			printf("Create Fluid Pipeline PSO splat failed\n");
			return false;
		}
	}

	// splat density 
	for (int i = 0; i < FrameBufferCount; i++)
	{
		if (!mRenderer.CreateFluidRootSignature(
			device,
			mRenderer.GetFluidRootSignaturePtr(i, static_cast<int>(Renderer::FluidStage::SplatDensity)),
			2))
		{
			printf("Create Fluid RS splat failed\n");
			return false;
		}

		if (!mRenderer.CreateHeap(
			device,
			mRenderer.GetFluidDescriptorHeapPtr(i, static_cast<int>(Renderer::FluidStage::SplatDensity)),
			mRenderer.GetFluidRtvDescriptorHeapPtr(i, static_cast<int>(Renderer::FluidStage::SplatDensity)),
			mRenderer.GetFluidDsvDescriptorHeapPtr(i, static_cast<int>(Renderer::FluidStage::SplatDensity)),
			2,
			1))
		{
			printf("Create Fluid Heap splat failed\n");
			return false;
		}

		if (!mRenderer.CreatePSO(
			device,
			mRenderer.GetFluidPsoPtr(i, static_cast<int>(Renderer::FluidStage::SplatDensity)),
			mRenderer.GetFluidRootSignature(i, static_cast<int>(Renderer::FluidStage::SplatDensity)),
			D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
			CD3DX12_BLEND_DESC(D3D12_DEFAULT), //mRenderer.AdditiveBlend(),
			Renderer::NoDepthTest(),
			DXGI_FORMAT_R16G16B16A16_FLOAT,
			DXGI_FORMAT_D32_FLOAT,
			1,
			&mPostProcessVS,
			nullptr,
			nullptr,
			nullptr,
			&mFluidSplatDensityPS,
			L"fluid-splat-density"))
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
			mRenderer.GetFluidDsvDescriptorHeapPtr(i, static_cast<int>(Renderer::FluidStage::ComputeDivergence)),
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
			DXGI_FORMAT_D32_FLOAT,
			1,
			&mPostProcessVS,
			nullptr,
			nullptr,
			nullptr,
			&mFluidComputeDivergencePS,
			L"fluid-compute-divergency"))
		{
			printf("Create Fluid Pipeline PSO divergence failed\n");
			return false;
		}
	}

	// fluid jacobi
	for (int i = 0; i < FrameBufferCount; i++)
	{
		for (int j = 0; j < JacobiIteration; j++)
		{
			if (!mRenderer.CreateFluidRootSignature(
				device,
				&mRenderer.fluidJacobiRootSignature[i][j],
				3))
			{
				printf("Create Fluid RS jacobi failed\n");
				return false;
			}

			if (!mRenderer.CreateHeap(
				device,
				&mRenderer.fluidJacobiDescriptorHeap[i][j],
				&mRenderer.fluidJacobiRtvDescriptorHeap[i][j],
				&mRenderer.fluidJacobiDsvDescriptorHeap[i][j],
				3,
				1))
			{
				printf("Create Fluid Heap jacobi failed\n");
				return false;
			}

			if (!mRenderer.CreatePSO(
				device,
				&mRenderer.fluidJacobiPSO[i][j],
				mRenderer.fluidJacobiRootSignature[i][j],
				D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
				CD3DX12_BLEND_DESC(D3D12_DEFAULT),
				Renderer::NoDepthTest(),
				DXGI_FORMAT_R16G16B16A16_FLOAT,
				DXGI_FORMAT_D32_FLOAT,
				1,
				&mPostProcessVS,
				nullptr,
				nullptr,
				nullptr,
				&mFluidJacobiPS,
				L"fluid-jacobi"))
			{
				printf("Create Fluid Pipeline PSO jacobi failed\n");
				return false;
			}
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
			mRenderer.GetFluidDsvDescriptorHeapPtr(i, static_cast<int>(Renderer::FluidStage::SubtractGradient)),
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
			DXGI_FORMAT_D32_FLOAT,
			1,
			&mPostProcessVS,
			nullptr,
			nullptr,
			nullptr,
			&mFluidSubtractGradientPS,
			L"fluid-subtract-gradient"))
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
		mRenderer.GetWaveParticleDsvDescriptorHeapPtr(static_cast<int>(Renderer::WaveParticleStage::Default)),
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
		DXGI_FORMAT_D32_FLOAT,
		1,
		&mWaveParticleVS,
		nullptr,
		nullptr,
		nullptr,
		&mWaveParticlePS,
		L"wave-particle-default"))
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
		mRenderer.GetPostProcessDsvDescriptorHeapPtr(static_cast<int>(Renderer::PostProcessStage::Horizontal)),
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
		Renderer::NoDepthTest(),//CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT),
		DXGI_FORMAT_R16G16B16A16_FLOAT,
		DXGI_FORMAT_D32_FLOAT,
		2,
		&mPostProcessVS,
		nullptr,
		nullptr,
		nullptr,
		&mPostProcessPS_H,
		L"post-process-h"))
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
		mRenderer.GetPostProcessDsvDescriptorHeapPtr(static_cast<int>(Renderer::PostProcessStage::Vertical)),
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
		Renderer::NoDepthTest(),//CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT),
		DXGI_FORMAT_R16G16B16A16_FLOAT,
		DXGI_FORMAT_D32_FLOAT,
		2,
		&mPostProcessVS,
		nullptr,
		nullptr,
		nullptr,
		&mPostProcessPS_V,
		L"post-process-v"))
	{
		printf("CreatePostProcessPipeline PSO V failed\n");
		return false;
	}

	// graphics wave surface
	if (!mRenderer.CreateGraphicsRootSignature(
		device,
		mRenderer.GetGraphicsRootSignaturePtr(static_cast<int>(Renderer::GraphicsStage::WaterSurface)),
		mFrameGraphics.GetTextureVec().size()))
	{
		printf("CreateGraphicsPipeline RS failed\n");
		return false;
	}

	if (!mRenderer.CreateHeapBindTexture(
		device,
		mRenderer.GetGraphicsDescriptorHeapPtr(static_cast<int>(Renderer::GraphicsStage::WaterSurface)),
		mRenderer.GetGraphicsRtvDescriptorHeapPtr(static_cast<int>(Renderer::GraphicsStage::WaterSurface)),
		mRenderer.GetGraphicsDsvDescriptorHeapPtr(static_cast<int>(Renderer::GraphicsStage::WaterSurface)),
		mFrameGraphics.GetTextureVec(),
		mFrameGraphics.GetRenderTextureVec()))
	{
		printf("CreateGraphicsPipeline Heap failed\n");
		return false;
	}

	if (!mRenderer.CreatePSO(
		device,
		mRenderer.GetGraphicsPsoPtr(static_cast<int>(Renderer::GraphicsStage::WaterSurface)),
		mRenderer.GetGraphicsRootSignature(static_cast<int>(Renderer::GraphicsStage::WaterSurface)),
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH,//D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
		CD3DX12_BLEND_DESC(D3D12_DEFAULT),// Renderer::NoBlend(),
		CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT),
		DXGI_FORMAT_R8G8B8A8_UNORM,
		DXGI_FORMAT_D32_FLOAT,
		1,
		&mVertexShader,
		&mHullShader,
		&mDomainShader,
		nullptr,
		&mPixelShader,
		L"graphic-water-surface"))
	{
		printf("CreateGraphicsPipeline PSO failed\n");
		return false;
	}

	// graphics obstacle surface
	if (!mRenderer.CreateGraphicsRootSignature(
		device,
		mRenderer.GetGraphicsRootSignaturePtr(static_cast<int>(Renderer::GraphicsStage::Obstacle)),
		mFrameObstacle.GetTextureVec().size()))
	{
		printf("CreateGraphicsPipeline RS obstacle failed\n");
		return false;
	}

	if (!mRenderer.CreateHeapBindTexture(
		device,
		mRenderer.GetGraphicsDescriptorHeapPtr(static_cast<int>(Renderer::GraphicsStage::Obstacle)),
		mRenderer.GetGraphicsRtvDescriptorHeapPtr(static_cast<int>(Renderer::GraphicsStage::Obstacle)),
		mRenderer.GetGraphicsDsvDescriptorHeapPtr(static_cast<int>(Renderer::GraphicsStage::Obstacle)),
		mFrameObstacle.GetTextureVec(),
		mFrameObstacle.GetRenderTextureVec()))
	{
		printf("CreateGraphicsPipeline Heap obstacle failed\n");
		return false;
	}

	if (!mRenderer.CreatePSO(
		device,
		mRenderer.GetGraphicsPsoPtr(static_cast<int>(Renderer::GraphicsStage::Obstacle)),
		mRenderer.GetGraphicsRootSignature(static_cast<int>(Renderer::GraphicsStage::Obstacle)),
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH,//D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
		CD3DX12_BLEND_DESC(D3D12_DEFAULT),// Renderer::NoBlend(),
		CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT),
		DXGI_FORMAT_R8G8B8A8_UNORM,
		DXGI_FORMAT_D32_FLOAT,
		1,
		&mObstacleVS,
		&mObstacleHS,
		&mObstacleDS,
		nullptr,
		&mObstaclePS,
		L"graphic-obstacle"))
	{
		printf("CreateGraphicsPipeline PSO obstacle failed\n");
		return false;
	}

	// graphics create obstacle
	if (!mRenderer.CreateGraphicsRootSignature(
		device,
		mRenderer.GetGraphicsRootSignaturePtr(static_cast<int>(Renderer::GraphicsStage::CreateObstacle)),
		mFrameCreateObstacle.GetTextureVec().size()))
	{
		printf("CreateGraphicsPipeline obstacle RS failed\n");
		return false;
	}

	if (!mRenderer.CreateHeapBindTexture(
		device,
		mRenderer.GetGraphicsDescriptorHeapPtr(static_cast<int>(Renderer::GraphicsStage::CreateObstacle)),
		mRenderer.GetGraphicsRtvDescriptorHeapPtr(static_cast<int>(Renderer::GraphicsStage::CreateObstacle)),
		mRenderer.GetGraphicsDsvDescriptorHeapPtr(static_cast<int>(Renderer::GraphicsStage::CreateObstacle)),
		mFrameCreateObstacle.GetTextureVec(),
		mFrameCreateObstacle.GetRenderTextureVec()))
	{
		printf("CreateGraphicsPipeline obstacle Heap failed\n");
		return false;
	}

	if (!mRenderer.CreatePSO(
		device,
		mRenderer.GetGraphicsPsoPtr(static_cast<int>(Renderer::GraphicsStage::CreateObstacle)),
		mRenderer.GetGraphicsRootSignature(static_cast<int>(Renderer::GraphicsStage::CreateObstacle)),
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
		Renderer::AdditiveBlend(),
		Renderer::NoDepthTest(),
		DXGI_FORMAT_R8G8B8A8_UNORM,//DXGI_FORMAT_R16G16B16A16_FLOAT,
		DXGI_FORMAT_D32_FLOAT,
		1,
		&mCreateObstacleVS,
		nullptr,
		nullptr,
		nullptr,
		&mCreateObstaclePS,
		L"graphic-create-obstacle"))
	{
		printf("CreateGraphicsPipeline obstacle PSO failed\n");
		return false;
	}

	// postprocess blur H obstacle
	if (!mRenderer.CreatePostProcessRootSignature(
		device,
		mRenderer.GetPostProcessRootSignaturePtr(static_cast<int>(Renderer::PostProcessStage::ObstacleHorizontal)),
		mFrameObstacleHorizontal.GetTextureVec().size()))
	{
		printf("CreatePostProcessPipeline obstacle H RS failed\n");
		return false;
	}

	if (!mRenderer.CreateHeapBindTexture(
		device,
		mRenderer.GetPostProcessDescriptorHeapPtr(static_cast<int>(Renderer::PostProcessStage::ObstacleHorizontal)),
		mRenderer.GetPostProcessRtvDescriptorHeapPtr(static_cast<int>(Renderer::PostProcessStage::ObstacleHorizontal)),
		mRenderer.GetPostProcessDsvDescriptorHeapPtr(static_cast<int>(Renderer::PostProcessStage::ObstacleHorizontal)),
		mFrameObstacleHorizontal.GetTextureVec(),
		mFrameObstacleHorizontal.GetRenderTextureVec()))
	{
		printf("CreatePostProcessPipeline obstacle H Heap failed\n");
		return false;
	}

	if (!mRenderer.CreatePSO(
		device,
		mRenderer.GetPostProcessPsoPtr(static_cast<int>(Renderer::PostProcessStage::ObstacleHorizontal)),
		mRenderer.GetPostProcessRootSignature(static_cast<int>(Renderer::PostProcessStage::ObstacleHorizontal)),
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
		Renderer::NoBlend(),
		Renderer::NoDepthTest(),
		DXGI_FORMAT_R8G8B8A8_UNORM,//DXGI_FORMAT_R16G16B16A16_FLOAT,
		DXGI_FORMAT_D32_FLOAT,
		1,
		&mPostProcessVS,
		nullptr,
		nullptr,
		nullptr,
		&mObstacleHorizontalPS,
		L"post-process-obstacle-h"))
	{
		printf("CreatePostProcessPipeline obstacle H PSO failed\n");
		return false;
	}

	// postprocess blur V obstacle
	if (!mRenderer.CreatePostProcessRootSignature(
		device,
		mRenderer.GetPostProcessRootSignaturePtr(static_cast<int>(Renderer::PostProcessStage::ObstacleVertical)),
		mFrameObstacleVertical.GetTextureVec().size()))
	{
		printf("CreatePostProcessPipeline obstacle V RS failed\n");
		return false;
	}

	if (!mRenderer.CreateHeapBindTexture(
		device,
		mRenderer.GetPostProcessDescriptorHeapPtr(static_cast<int>(Renderer::PostProcessStage::ObstacleVertical)),
		mRenderer.GetPostProcessRtvDescriptorHeapPtr(static_cast<int>(Renderer::PostProcessStage::ObstacleVertical)),
		mRenderer.GetPostProcessDsvDescriptorHeapPtr(static_cast<int>(Renderer::PostProcessStage::ObstacleVertical)),
		mFrameObstacleVertical.GetTextureVec(),
		mFrameObstacleVertical.GetRenderTextureVec()))
	{
		printf("CreatePostProcessPipeline obstacle V Heap failed\n");
		return false;
	}

	if (!mRenderer.CreatePSO(
		device,
		mRenderer.GetPostProcessPsoPtr(static_cast<int>(Renderer::PostProcessStage::ObstacleVertical)),
		mRenderer.GetPostProcessRootSignature(static_cast<int>(Renderer::PostProcessStage::ObstacleVertical)),
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
		Renderer::NoBlend(),
		Renderer::NoDepthTest(),
		DXGI_FORMAT_R8G8B8A8_UNORM,//DXGI_FORMAT_R16G16B16A16_FLOAT,
		DXGI_FORMAT_D32_FLOAT,
		1,
		&mPostProcessVS,
		nullptr,
		nullptr,
		nullptr,
		&mObstacleVerticalPS,
		L"post-process-obstacle-v"))
	{
		printf("CreatePostProcessPipeline obstacle V PSO failed\n");
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

void WaitForPreviousFrame(int frameIndexOverride = -1)
{
	HRESULT hr;

	// swap the current rtv buffer index so we draw on the correct buffer
	frameIndex = frameIndexOverride < 0 ? swapChain->GetCurrentBackBufferIndex() : frameIndexOverride;

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

	if (CreateObstacle || ClearObstacle)
	{
		vector<D3D12_RESOURCE_BARRIER> barrierCreateObstacle;
		if (mRenderTextureObstacleCreate.GetResourceState() != D3D12_RESOURCE_STATE_RENDER_TARGET)
			barrierCreateObstacle.push_back(mRenderTextureObstacleCreate.TransitionToResourceState(D3D12_RESOURCE_STATE_RENDER_TARGET));
			
		commandList->ResourceBarrier(barrierCreateObstacle.size(), barrierCreateObstacle.data());

		///////// MY GRAPHICS CREATE OBSTACLE PIPELINE /////////
		//vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv//
		mRenderer.RecordPipeline(
			commandList,
			mRenderer.GetGraphicsPSO(static_cast<int>(Renderer::GraphicsStage::CreateObstacle)),
			mRenderer.GetGraphicsRootSignature(static_cast<int>(Renderer::GraphicsStage::CreateObstacle)),
			mRenderer.GetGraphicsDescriptorHeap(static_cast<int>(Renderer::GraphicsStage::CreateObstacle)),
			&mFrameCreateObstacle,
			&mScene,
			false,
			false,//because no valid dsv, invoke clear would trigger a debug layer error
			XMFLOAT4(0,0,0,0),
			1.0f,
			D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^//
		///////// MY GRAPHICS CREATE OBSTACLE PIPELINE /////////

		vector<D3D12_RESOURCE_BARRIER> barrierObstacleH;
		if (mRenderTextureObstacleCreate.GetResourceState() != D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE)
			barrierObstacleH.push_back(mRenderTextureObstacleCreate.TransitionToResourceState(D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
		if (mRenderTextureObstacleBlur.GetResourceState() != D3D12_RESOURCE_STATE_RENDER_TARGET)
			barrierObstacleH.push_back(mRenderTextureObstacleBlur.TransitionToResourceState(D3D12_RESOURCE_STATE_RENDER_TARGET));
			
		commandList->ResourceBarrier(barrierObstacleH.size(), barrierObstacleH.data());

		///////// MY POSTPROCESS OBSTACLE H PIPELINE /////////
		//vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv//
		mRenderer.RecordPipeline(
			commandList,
			mRenderer.GetPostProcessPSO(static_cast<int>(Renderer::PostProcessStage::ObstacleHorizontal)),
			mRenderer.GetPostProcessRootSignature(static_cast<int>(Renderer::PostProcessStage::ObstacleHorizontal)),
			mRenderer.GetPostProcessDescriptorHeap(static_cast<int>(Renderer::PostProcessStage::ObstacleHorizontal)),
			&mFrameObstacleHorizontal,
			&mScene,
			true,
			false,//because no valid dsv, invoke clear would trigger a debug layer error
			XMFLOAT4(0,0,0,0),
			1.0f,
			D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^//
		///////// MY POSTPROCESS OBSTACLE H PIPELINE /////////

		vector<D3D12_RESOURCE_BARRIER> barrierObstacleV;
		if (mRenderTextureObstacle.GetResourceState() != D3D12_RESOURCE_STATE_RENDER_TARGET)
			barrierObstacleV.push_back(mRenderTextureObstacle.TransitionToResourceState(D3D12_RESOURCE_STATE_RENDER_TARGET));
		if (mRenderTextureObstacleBlur.GetResourceState() != D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE)
			barrierObstacleV.push_back(mRenderTextureObstacleBlur.TransitionToResourceState(D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
			
		commandList->ResourceBarrier(barrierObstacleV.size(), barrierObstacleV.data());

		///////// MY POSTPROCESS OBSTACLE V PIPELINE /////////
		//vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv//
		mRenderer.RecordPipeline(
			commandList,
			mRenderer.GetPostProcessPSO(static_cast<int>(Renderer::PostProcessStage::ObstacleVertical)),
			mRenderer.GetPostProcessRootSignature(static_cast<int>(Renderer::PostProcessStage::ObstacleVertical)),
			mRenderer.GetPostProcessDescriptorHeap(static_cast<int>(Renderer::PostProcessStage::ObstacleVertical)),
			&mFrameObstacleVertical,
			&mScene,
			true,
			false,//because no valid dsv, invoke clear would trigger a debug layer error
			XMFLOAT4(0, 0, 0, 0),
			1.0f,
			D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^//
		///////// MY POSTPROCESS OBSTACLE V PIPELINE /////////

		if (ClearObstacle)
		{
			///////// MY GRAPHICS Clear OBSTACLE /////////
			//vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv//
			vector<D3D12_RESOURCE_BARRIER> barrierClearObstacle1;
			if (mRenderTextureObstacleCreate.GetResourceState() != D3D12_RESOURCE_STATE_RENDER_TARGET)
				barrierClearObstacle1.push_back(mRenderTextureObstacleCreate.TransitionToResourceState(D3D12_RESOURCE_STATE_RENDER_TARGET));
			commandList->ResourceBarrier(barrierClearObstacle1.size(), barrierClearObstacle1.data());

			mRenderer.Clear(
				commandList,
				&mFrameCreateObstacle,
				false,
				XMFLOAT4(0,0,0,0),
				1.0f
			);

			vector<D3D12_RESOURCE_BARRIER> barrierClearObstacle2;
			if (mRenderTextureObstacle.GetResourceState() != D3D12_RESOURCE_STATE_RENDER_TARGET)
				barrierClearObstacle2.push_back(mRenderTextureObstacle.TransitionToResourceState(D3D12_RESOURCE_STATE_RENDER_TARGET));
			commandList->ResourceBarrier(barrierClearObstacle2.size(), barrierClearObstacle2.data());

			mRenderer.Clear(
				commandList,
				&mFrameObstacleVertical,
				false,
				XMFLOAT4(0,0,0,0),
				1.0f
			);
			//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^//
			///////// MY GRAPHICS Clear OBSTACLE /////////
			ClearObstacle = false;
		}

	}

	fluidSimulationStep++;
	if (FluidSimulation && fluidSimulationStep >= FluidSimulationStep)
	{
		fluidSimulationStep = 0;

		vector<D3D12_RESOURCE_BARRIER> barrierAdvectVelocity;
		if (mRenderTextureObstacle.GetResourceState() != D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE)
			barrierAdvectVelocity.push_back(mRenderTextureObstacle.TransitionToResourceState(D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
		if (pRtVelocityPing->GetResourceState() != D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE)
			barrierAdvectVelocity.push_back(pRtVelocityPing->TransitionToResourceState(D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
		if (pRtVelocityPong->GetResourceState() != D3D12_RESOURCE_STATE_RENDER_TARGET)
			barrierAdvectVelocity.push_back(pRtVelocityPong->TransitionToResourceState(D3D12_RESOURCE_STATE_RENDER_TARGET));

		commandList->ResourceBarrier(barrierAdvectVelocity.size(), barrierAdvectVelocity.data());

		///////// MY FLUID PIPELINE advect velocity /////////
		//vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv//
		mRenderer.BindRenderTextureToRtvDsvDescriptorHeap(
			device,
			mRenderer.GetFluidRtvDescriptorHeap(frameIndex, static_cast<int>(Renderer::FluidStage::AdvectVelocity)),
			mRenderer.GetFluidDsvDescriptorHeap(frameIndex, static_cast<int>(Renderer::FluidStage::AdvectVelocity)),
			pRtVelocityPong,
			0);//rtv
		mRenderer.BindTextureToDescriptorHeap(
			device,
			mRenderer.GetFluidDescriptorHeap(frameIndex, static_cast<int>(Renderer::FluidStage::AdvectVelocity)),
			//&mTextureObstacle,
			&mRenderTextureObstacle,
			0);//t0
		mRenderer.BindTextureToDescriptorHeap(
			device,
			mRenderer.GetFluidDescriptorHeap(frameIndex, static_cast<int>(Renderer::FluidStage::AdvectVelocity)),
			pRtVelocityPing,
			1);//t1
		mRenderer.BindTextureToDescriptorHeap(
			device,
			mRenderer.GetFluidDescriptorHeap(frameIndex, static_cast<int>(Renderer::FluidStage::AdvectVelocity)),
			pRtVelocityPing,
			2);//t2
		mRenderer.RecordPipelineOverride(
			commandList,
			mRenderer.GetFluidPSO(frameIndex, static_cast<int>(Renderer::FluidStage::AdvectVelocity)),
			mRenderer.GetFluidRootSignature(frameIndex, static_cast<int>(Renderer::FluidStage::AdvectVelocity)),
			mRenderer.GetFluidDescriptorHeap(frameIndex, static_cast<int>(Renderer::FluidStage::AdvectVelocity)),
			pRtVelocityPong->GetRtvHandle(),//rtv
			pRtVelocityPong->GetDsvHandle(),//dsv
			&mFrameFluidAdvect,
			&mScene,
			true,
			false,//because no valid dsv, invoke clear would trigger a debug layer error
			XMFLOAT4(0,0,0,0),
			1.0f,
			D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		SwitchPingPong(&pRtVelocityPing, &pRtVelocityPong);//ping pong
		//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^//
		///////// MY FLUID PIPELINE advect velocity /////////

		vector<D3D12_RESOURCE_BARRIER> barrierAdvectDensity;
		if (pRtDensityPing->GetResourceState() != D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE)
			barrierAdvectDensity.push_back(pRtDensityPing->TransitionToResourceState(D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
		if (pRtVelocityPing->GetResourceState() != D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE)
			barrierAdvectDensity.push_back(pRtVelocityPing->TransitionToResourceState(D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
		if (pRtDensityPong->GetResourceState() != D3D12_RESOURCE_STATE_RENDER_TARGET)
			barrierAdvectDensity.push_back(pRtDensityPong->TransitionToResourceState(D3D12_RESOURCE_STATE_RENDER_TARGET));
			
		commandList->ResourceBarrier(barrierAdvectDensity.size(), barrierAdvectDensity.data());

		///////// MY FLUID PIPELINE advect density /////////
		//vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv//
		mRenderer.BindRenderTextureToRtvDsvDescriptorHeap(
			device,
			mRenderer.GetFluidRtvDescriptorHeap(frameIndex, static_cast<int>(Renderer::FluidStage::AdvectDensity)),
			mRenderer.GetFluidDsvDescriptorHeap(frameIndex, static_cast<int>(Renderer::FluidStage::AdvectDensity)),
			pRtDensityPong,
			0);//rtv
		mRenderer.BindTextureToDescriptorHeap(
			device,
			mRenderer.GetFluidDescriptorHeap(frameIndex, static_cast<int>(Renderer::FluidStage::AdvectDensity)),
			//&mTextureObstacle,
			&mRenderTextureObstacle,
			0);//t0
		mRenderer.BindTextureToDescriptorHeap(
			device,
			mRenderer.GetFluidDescriptorHeap(frameIndex, static_cast<int>(Renderer::FluidStage::AdvectDensity)),
			pRtVelocityPing,
			1);//t1
		mRenderer.BindTextureToDescriptorHeap(
			device,
			mRenderer.GetFluidDescriptorHeap(frameIndex, static_cast<int>(Renderer::FluidStage::AdvectDensity)),
			pRtDensityPing,
			2);//t2
		mRenderer.RecordPipelineOverride(
			commandList,
			mRenderer.GetFluidPSO(frameIndex, static_cast<int>(Renderer::FluidStage::AdvectDensity)),
			mRenderer.GetFluidRootSignature(frameIndex, static_cast<int>(Renderer::FluidStage::AdvectDensity)),
			mRenderer.GetFluidDescriptorHeap(frameIndex, static_cast<int>(Renderer::FluidStage::AdvectDensity)),
			pRtDensityPong->GetRtvHandle(),//rtv
			pRtDensityPong->GetDsvHandle(),//dsv
			&mFrameFluidAdvect,
			&mScene,
			true,
			false,//because no valid dsv, invoke clear would trigger a debug layer error
			XMFLOAT4(0,0,0,0),
			1.0f,
			D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		SwitchPingPong(&pRtDensityPing, &pRtDensityPong);//ping pong
		//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^//
		///////// MY FLUID PIPELINE advect density /////////

		vector<D3D12_RESOURCE_BARRIER> barrierSplatVelocity;
		if (pRtVelocityPong->GetResourceState() != D3D12_RESOURCE_STATE_RENDER_TARGET)
			barrierSplatVelocity.push_back(pRtVelocityPong->TransitionToResourceState(D3D12_RESOURCE_STATE_RENDER_TARGET));
		if (pRtVelocityPing->GetResourceState() != D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE)
			barrierSplatVelocity.push_back(pRtVelocityPing->TransitionToResourceState(D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
			
		commandList->ResourceBarrier(barrierSplatVelocity.size(), barrierSplatVelocity.data());

		///////// MY FLUID PIPELINE splat velocity /////////
		//vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv//
		mRenderer.BindRenderTextureToRtvDsvDescriptorHeap(
			device,
			mRenderer.GetFluidRtvDescriptorHeap(frameIndex, static_cast<int>(Renderer::FluidStage::SplatVelocity)),
			mRenderer.GetFluidDsvDescriptorHeap(frameIndex, static_cast<int>(Renderer::FluidStage::SplatVelocity)),
			pRtVelocityPong,
			0);//rtv
		mRenderer.BindTextureToDescriptorHeap(
			device,
			mRenderer.GetFluidDescriptorHeap(frameIndex, static_cast<int>(Renderer::FluidStage::SplatVelocity)),
			&mRenderTextureObstacle,
			0);//t0
		mRenderer.BindTextureToDescriptorHeap(
			device,
			mRenderer.GetFluidDescriptorHeap(frameIndex, static_cast<int>(Renderer::FluidStage::SplatVelocity)),
			pRtVelocityPing,
			1);//t1
		mRenderer.RecordPipelineOverride(
			commandList,
			mRenderer.GetFluidPSO(frameIndex, static_cast<int>(Renderer::FluidStage::SplatVelocity)),
			mRenderer.GetFluidRootSignature(frameIndex, static_cast<int>(Renderer::FluidStage::SplatVelocity)),
			mRenderer.GetFluidDescriptorHeap(frameIndex, static_cast<int>(Renderer::FluidStage::SplatVelocity)),
			pRtVelocityPong->GetRtvHandle(),//rtv
			pRtVelocityPong->GetDsvHandle(),//dsv
			&mFrameFluidSplat,
			&mScene,
			false,
			false,//because no valid dsv, invoke clear would trigger a debug layer error
			XMFLOAT4(0,0,0,0),
			1.0f,
			D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		SwitchPingPong(&pRtVelocityPing, &pRtVelocityPong);//ping pong
		//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^//
		///////// MY FLUID PIPELINE splat velocity /////////

		vector<D3D12_RESOURCE_BARRIER> barrierSplatDensity;
		if (pRtDensityPong->GetResourceState() != D3D12_RESOURCE_STATE_RENDER_TARGET)
			barrierSplatDensity.push_back(pRtDensityPong->TransitionToResourceState(D3D12_RESOURCE_STATE_RENDER_TARGET));
		if (pRtDensityPing->GetResourceState() != D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE)
			barrierSplatDensity.push_back(pRtDensityPing->TransitionToResourceState(D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
			
		commandList->ResourceBarrier(barrierSplatDensity.size(), barrierSplatDensity.data());

		///////// MY FLUID PIPELINE splat density /////////
		//vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv//
		mRenderer.BindRenderTextureToRtvDsvDescriptorHeap(
			device,
			mRenderer.GetFluidRtvDescriptorHeap(frameIndex, static_cast<int>(Renderer::FluidStage::SplatDensity)),
			mRenderer.GetFluidDsvDescriptorHeap(frameIndex, static_cast<int>(Renderer::FluidStage::SplatDensity)),
			pRtDensityPong,
			0);//rtv
		mRenderer.BindTextureToDescriptorHeap(
			device,
			mRenderer.GetFluidDescriptorHeap(frameIndex, static_cast<int>(Renderer::FluidStage::SplatDensity)),
			//&mTextureObstacle,
			&mRenderTextureObstacle,
			0);//t0
		mRenderer.BindTextureToDescriptorHeap(
			device,
			mRenderer.GetFluidDescriptorHeap(frameIndex, static_cast<int>(Renderer::FluidStage::SplatDensity)),
			pRtDensityPing,
			1);//t1
		mRenderer.RecordPipelineOverride(
			commandList,
			mRenderer.GetFluidPSO(frameIndex, static_cast<int>(Renderer::FluidStage::SplatDensity)),
			mRenderer.GetFluidRootSignature(frameIndex, static_cast<int>(Renderer::FluidStage::SplatDensity)),
			mRenderer.GetFluidDescriptorHeap(frameIndex, static_cast<int>(Renderer::FluidStage::SplatDensity)),
			pRtDensityPong->GetRtvHandle(),//rtv
			pRtDensityPong->GetDsvHandle(),//dsv
			&mFrameFluidSplat,
			&mScene,
			false,
			false,//because no valid dsv, invoke clear would trigger a debug layer error
			XMFLOAT4(0,0,0,0),
			1.0f,
			D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		SwitchPingPong(&pRtDensityPing, &pRtDensityPong);//ping pong
		//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^//
		///////// MY FLUID PIPELINE splat density /////////

		vector<D3D12_RESOURCE_BARRIER> barrierDivergence;
		if (mRenderTextureFluidDivergence.GetResourceState() != D3D12_RESOURCE_STATE_RENDER_TARGET)
			barrierDivergence.push_back(mRenderTextureFluidDivergence.TransitionToResourceState(D3D12_RESOURCE_STATE_RENDER_TARGET));
		if (pRtVelocityPing->GetResourceState() != D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE)
			barrierDivergence.push_back(pRtVelocityPing->TransitionToResourceState(D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

		commandList->ResourceBarrier(barrierDivergence.size(), barrierDivergence.data());

		///////// MY FLUID PIPELINE divergence /////////
		//vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv//
		mRenderer.BindRenderTextureToRtvDsvDescriptorHeap(
			device,
			mRenderer.GetFluidRtvDescriptorHeap(frameIndex, static_cast<int>(Renderer::FluidStage::ComputeDivergence)),
			mRenderer.GetFluidDsvDescriptorHeap(frameIndex, static_cast<int>(Renderer::FluidStage::ComputeDivergence)),
			&mRenderTextureFluidDivergence,
			0);//rtv
		mRenderer.BindTextureToDescriptorHeap(
			device,
			mRenderer.GetFluidDescriptorHeap(frameIndex, static_cast<int>(Renderer::FluidStage::ComputeDivergence)),
			&mRenderTextureObstacle,
			0);//t0
		mRenderer.BindTextureToDescriptorHeap(
			device,
			mRenderer.GetFluidDescriptorHeap(frameIndex, static_cast<int>(Renderer::FluidStage::ComputeDivergence)),
			pRtVelocityPing,
			1);//t1
		mRenderer.RecordPipelineOverride(
			commandList,
			mRenderer.GetFluidPSO(frameIndex, static_cast<int>(Renderer::FluidStage::ComputeDivergence)),
			mRenderer.GetFluidRootSignature(frameIndex, static_cast<int>(Renderer::FluidStage::ComputeDivergence)),
			mRenderer.GetFluidDescriptorHeap(frameIndex, static_cast<int>(Renderer::FluidStage::ComputeDivergence)),
			mRenderTextureFluidDivergence.GetRtvHandle(),//rtv
			mRenderTextureFluidDivergence.GetDsvHandle(),//dsv
			&mFrameFluidDivergence,
			&mScene,
			true,
			false,//because no valid dsv, invoke clear would trigger a debug layer error
			XMFLOAT4(0,0,0,0),
			1.0f,
			D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^//
		///////// MY FLUID PIPELINE divergence /////////

		///////// MY FLUID PIPELINE jacobi /////////
		//vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv//
		vector<D3D12_RESOURCE_BARRIER> barrierPreJacobi;
		if (pRtPressurePing->GetResourceState() != D3D12_RESOURCE_STATE_RENDER_TARGET)
			barrierPreJacobi.push_back(pRtPressurePing->TransitionToResourceState(D3D12_RESOURCE_STATE_RENDER_TARGET));

		commandList->ResourceBarrier(barrierPreJacobi.size(), barrierPreJacobi.data());

		mRenderer.BindRenderTextureToRtvDsvDescriptorHeap(
			device,
			mRenderer.fluidJacobiRtvDescriptorHeap[frameIndex][0],
			mRenderer.fluidJacobiDsvDescriptorHeap[frameIndex][0],
			pRtPressurePing,
			0);//rtv
		mRenderer.ClearOverride(
			commandList,
			pRtPressurePing->GetRtvHandle(),
			pRtPressurePing->GetDsvHandle(),
			false,
			XMFLOAT4(0,0,0,0),
			1.0f);

		for (int i = 0; i < JacobiIteration; i++)
		{
			vector<D3D12_RESOURCE_BARRIER> barrierJacobi;
			if (pRtPressurePong->GetResourceState() != D3D12_RESOURCE_STATE_RENDER_TARGET)
				barrierJacobi.push_back(pRtPressurePong->TransitionToResourceState(D3D12_RESOURCE_STATE_RENDER_TARGET));
			if (pRtPressurePing->GetResourceState() != D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE)
				barrierJacobi.push_back(pRtPressurePing->TransitionToResourceState(D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
			if (mRenderTextureFluidDivergence.GetResourceState() != D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE)
				barrierJacobi.push_back(mRenderTextureFluidDivergence.TransitionToResourceState(D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

			commandList->ResourceBarrier(barrierJacobi.size(), barrierJacobi.data());

			mRenderer.BindRenderTextureToRtvDsvDescriptorHeap(
				device,
				mRenderer.fluidJacobiRtvDescriptorHeap[frameIndex][i],
				mRenderer.fluidJacobiDsvDescriptorHeap[frameIndex][i],
				pRtPressurePong,
				0);//rtv
			mRenderer.BindTextureToDescriptorHeap(
				device,
				mRenderer.fluidJacobiDescriptorHeap[frameIndex][i],
				//mRenderer.GetFluidDescriptorHeap(frameIndex, static_cast<int>(Renderer::FluidStage::Jacobi)),
				//&mTextureObstacle,
				&mRenderTextureObstacle,
				0);//t0
			mRenderer.BindTextureToDescriptorHeap(
				device,
				mRenderer.fluidJacobiDescriptorHeap[frameIndex][i],
				//mRenderer.GetFluidDescriptorHeap(frameIndex, static_cast<int>(Renderer::FluidStage::Jacobi)),
				pRtPressurePing,
				1);//t1
			mRenderer.BindTextureToDescriptorHeap(
				device,
				mRenderer.fluidJacobiDescriptorHeap[frameIndex][i],
				//mRenderer.GetFluidDescriptorHeap(frameIndex, static_cast<int>(Renderer::FluidStage::Jacobi)),
				&mRenderTextureFluidDivergence,
				2);//t2
			mRenderer.RecordPipelineOverride(
				commandList,
				mRenderer.fluidJacobiPSO[frameIndex][i],
				mRenderer.fluidJacobiRootSignature[frameIndex][i],
				mRenderer.fluidJacobiDescriptorHeap[frameIndex][i],
				pRtPressurePong->GetRtvHandle(),//rtv
				pRtPressurePong->GetDsvHandle(),//dsv
				&mFrameFluidJacobi,
				&mScene,
				true,
				false,//because no valid dsv, invoke clear would trigger a debug layer error
				XMFLOAT4(0,0,0,0),
				1.0f,
				D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			SwitchPingPong(&pRtPressurePing, &pRtPressurePong);
		}
		//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^//
		///////// MY FLUID PIPELINE jacobi /////////

		vector<D3D12_RESOURCE_BARRIER> barrierGradient;
		if (pRtVelocityPong->GetResourceState() != D3D12_RESOURCE_STATE_RENDER_TARGET)
			barrierGradient.push_back(pRtVelocityPong->TransitionToResourceState(D3D12_RESOURCE_STATE_RENDER_TARGET));
		if (pRtPressurePing->GetResourceState() != D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE)
			barrierGradient.push_back(pRtPressurePing->TransitionToResourceState(D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
		if (pRtVelocityPing->GetResourceState() != D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE)
			barrierGradient.push_back(pRtVelocityPing->TransitionToResourceState(D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

		commandList->ResourceBarrier(barrierGradient.size(), barrierGradient.data());

		///////// MY FLUID PIPELINE gradient /////////
		//vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv//
		mRenderer.BindRenderTextureToRtvDsvDescriptorHeap(
			device,
			mRenderer.GetFluidRtvDescriptorHeap(frameIndex, static_cast<int>(Renderer::FluidStage::SubtractGradient)),
			mRenderer.GetFluidDsvDescriptorHeap(frameIndex, static_cast<int>(Renderer::FluidStage::SubtractGradient)),
			pRtVelocityPong,
			0);//rtv
		mRenderer.BindTextureToDescriptorHeap(
			device,
			mRenderer.GetFluidDescriptorHeap(frameIndex, static_cast<int>(Renderer::FluidStage::SubtractGradient)),
			//&mTextureObstacle,
			&mRenderTextureObstacle,
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
		mRenderer.RecordPipelineOverride(
			commandList,
			mRenderer.GetFluidPSO(frameIndex, static_cast<int>(Renderer::FluidStage::SubtractGradient)),
			mRenderer.GetFluidRootSignature(frameIndex, static_cast<int>(Renderer::FluidStage::SubtractGradient)),
			mRenderer.GetFluidDescriptorHeap(frameIndex, static_cast<int>(Renderer::FluidStage::SubtractGradient)),
			pRtVelocityPong->GetRtvHandle(),//rtv
			pRtVelocityPong->GetDsvHandle(),//dsv
			&mFrameFluidGradient,
			&mScene,
			true,
			false,//because no valid dsv, invoke clear would trigger a debug layer error
			XMFLOAT4(0,0,0,0),
			1.0f,
			D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		SwitchPingPong(&pRtVelocityPing, &pRtVelocityPong);
		//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^//
		///////// MY FLUID PIPELINE gradient /////////
	}

	vector<D3D12_RESOURCE_BARRIER> barrierGraphicsToWaveParticle;
	if (mRenderTextureWaveParticle.GetResourceState() != D3D12_RESOURCE_STATE_RENDER_TARGET)
		barrierGraphicsToWaveParticle.push_back(mRenderTextureWaveParticle.TransitionToResourceState(D3D12_RESOURCE_STATE_RENDER_TARGET));

	commandList->ResourceBarrier(barrierGraphicsToWaveParticle.size(), barrierGraphicsToWaveParticle.data());

	///////// MY WAVE PARTICLE PIPELINE /////////
	//vvvvvvvvvvvvvvvvvvvvvvvvvvv//
	mRenderer.RecordPipeline(
		commandList,
		mRenderer.GetWaveParticlePSO(static_cast<int>(Renderer::WaveParticleStage::Default)),
		mRenderer.GetWaveParticleRootSignature(static_cast<int>(Renderer::WaveParticleStage::Default)),
		mRenderer.GetWaveParticleDescriptorHeap(static_cast<int>(Renderer::WaveParticleStage::Default)),
		&mFrameWaveParticle,
		&mScene,
		true,
		false,//because no valid dsv, invoke clear would trigger a debug layer error
		XMFLOAT4(0,0,0,0),
		1.0f,
		D3D_PRIMITIVE_TOPOLOGY_POINTLIST);
	//^^^^^^^^^^^^^^^^^^^^^^^^^^^//
	///////// MY WAVE PARTICLE PIPELINE /////////

	vector<D3D12_RESOURCE_BARRIER> barrierWaveParticleToPostprocessH;
	if (mRenderTextureWaveParticle.GetResourceState() != D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE)
		barrierWaveParticleToPostprocessH.push_back(mRenderTextureWaveParticle.TransitionToResourceState(D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
	if (mRenderTexturePostProcessH1.GetResourceState() != D3D12_RESOURCE_STATE_RENDER_TARGET)
		barrierWaveParticleToPostprocessH.push_back(mRenderTexturePostProcessH1.TransitionToResourceState(D3D12_RESOURCE_STATE_RENDER_TARGET));
	if (mRenderTexturePostProcessH2.GetResourceState() != D3D12_RESOURCE_STATE_RENDER_TARGET)
		barrierWaveParticleToPostprocessH.push_back(mRenderTexturePostProcessH2.TransitionToResourceState(D3D12_RESOURCE_STATE_RENDER_TARGET));

	commandList->ResourceBarrier(barrierWaveParticleToPostprocessH.size(), barrierWaveParticleToPostprocessH.data());
	
	///////// MY POSTPROCESS H PIPELINE /////////
	//vvvvvvvvvvvvvvvvvvvvvvvvvvv//
	mRenderer.RecordPipeline(
		commandList,
		mRenderer.GetPostProcessPSO(static_cast<int>(Renderer::PostProcessStage::Horizontal)),
		mRenderer.GetPostProcessRootSignature(static_cast<int>(Renderer::PostProcessStage::Horizontal)),
		mRenderer.GetPostProcessDescriptorHeap(static_cast<int>(Renderer::PostProcessStage::Horizontal)),
		&mFramePostProcessH,
		&mScene,
		true,
		false,//because no valid dsv, invoke clear would trigger a debug layer error
		XMFLOAT4(0,0,0,0),
		1.0f,
		D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//^^^^^^^^^^^^^^^^^^^^^^^^^^^//
	///////// MY POSTPROCESS H PIPELINE /////////

	vector<D3D12_RESOURCE_BARRIER> barrierPostprocessHToPostprocessV;
	if (mRenderTexturePostProcessH1.GetResourceState() != D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE)
		barrierPostprocessHToPostprocessV.push_back(mRenderTexturePostProcessH1.TransitionToResourceState(D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
	if (mRenderTexturePostProcessH2.GetResourceState() != D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE)
		barrierPostprocessHToPostprocessV.push_back(mRenderTexturePostProcessH2.TransitionToResourceState(D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
	if (mRenderTexturePostProcessV1.GetResourceState() != D3D12_RESOURCE_STATE_RENDER_TARGET)
		barrierPostprocessHToPostprocessV.push_back(mRenderTexturePostProcessV1.TransitionToResourceState(D3D12_RESOURCE_STATE_RENDER_TARGET));
	if (mRenderTexturePostProcessV2.GetResourceState() != D3D12_RESOURCE_STATE_RENDER_TARGET)
		barrierPostprocessHToPostprocessV.push_back(mRenderTexturePostProcessV2.TransitionToResourceState(D3D12_RESOURCE_STATE_RENDER_TARGET));

	commandList->ResourceBarrier(barrierPostprocessHToPostprocessV.size(), barrierPostprocessHToPostprocessV.data());

	///////// MY POSTPROCESS V PIPELINE /////////
	//vvvvvvvvvvvvvvvvvvvvvvvvvvv//
	mRenderer.RecordPipeline(
		commandList,
		mRenderer.GetPostProcessPSO(static_cast<int>(Renderer::PostProcessStage::Vertical)),
		mRenderer.GetPostProcessRootSignature(static_cast<int>(Renderer::PostProcessStage::Vertical)),
		mRenderer.GetPostProcessDescriptorHeap(static_cast<int>(Renderer::PostProcessStage::Vertical)),
		&mFramePostProcessV,
		&mScene,
		true,
		false,//because no valid dsv, invoke clear would trigger a debug layer error
		XMFLOAT4(0,0,0,0),
		1.0f,
		D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//^^^^^^^^^^^^^^^^^^^^^^^^^^^//
	///////// MY POSTPROCESS V PIPELINE /////////

	//D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE	
	//The resource is used with a shader other than the pixel shader.
	//A subresource must be in this state before being read by any stage(except for the pixel shader stage) via a shader resource view.
	//You can still use the resource in a pixel shader with this flag as long as it also has the flag D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE set.
	//This is a read - only state.
	//https://docs.microsoft.com/en-us/windows/win32/api/d3d12/ne-d3d12-d3d12_resource_states

	vector<D3D12_RESOURCE_BARRIER> barrierPostprocessVToGraphics;
	if (mRenderTextureObstacle.GetResourceState() != D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE)
		barrierPostprocessVToGraphics.push_back(mRenderTextureObstacle.TransitionToResourceState(D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));
	if (mRenderTexturePostProcessV1.GetResourceState() != (D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE))
		barrierPostprocessVToGraphics.push_back(mRenderTexturePostProcessV1.TransitionToResourceState(D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
	if (mRenderTexturePostProcessV2.GetResourceState() != (D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE))
		barrierPostprocessVToGraphics.push_back(mRenderTexturePostProcessV2.TransitionToResourceState(D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
	if (pRtVelocityPing->GetResourceState() != (D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE))
		barrierPostprocessVToGraphics.push_back(pRtVelocityPing->TransitionToResourceState(D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
	if (pRtPressurePing->GetResourceState() != D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE)
		barrierPostprocessVToGraphics.push_back(pRtPressurePing->TransitionToResourceState(D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
	if (pRtDensityPing->GetResourceState() != D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE)
		barrierPostprocessVToGraphics.push_back(pRtDensityPing->TransitionToResourceState(D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
	if (mRenderTextureFluidDivergence.GetResourceState() != D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE)
		barrierPostprocessVToGraphics.push_back(mRenderTextureFluidDivergence.TransitionToResourceState(D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

	commandList->ResourceBarrier(barrierPostprocessVToGraphics.size(), barrierPostprocessVToGraphics.data());
	
	///////// MY GRAPHICS WATER SURFACE PIPELINE /////////
	//vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv//
	mRenderer.BindTextureToDescriptorHeap(
		device,
		mRenderer.GetGraphicsDescriptorHeap(static_cast<int>(Renderer::GraphicsStage::WaterSurface)),
		pRtVelocityPing,
		//pRtPressurePing,
		1);//t1
	mRenderer.BindTextureToDescriptorHeap(
		device,
		mRenderer.GetGraphicsDescriptorHeap(static_cast<int>(Renderer::GraphicsStage::WaterSurface)),
		pRtDensityPing,
		//pRtPressurePing,
		4);//t4
	mRenderer.BindTextureToDescriptorHeap(
		device,
		mRenderer.GetGraphicsDescriptorHeap(static_cast<int>(Renderer::GraphicsStage::WaterSurface)),
		pRtPressurePing,
		//pRtPressurePing,
		5);//t5
	mRenderer.RecordPipelineOverride(
		commandList,
		mRenderer.GetGraphicsPSO(static_cast<int>(Renderer::GraphicsStage::WaterSurface)),
		mRenderer.GetGraphicsRootSignature(static_cast<int>(Renderer::GraphicsStage::WaterSurface)),
		mRenderer.GetGraphicsDescriptorHeap(static_cast<int>(Renderer::GraphicsStage::WaterSurface)),
		mRenderer.GetRtvHandle(frameIndex),
		mRenderer.GetDsvHandle(frameIndex),
		&mFrameGraphics,
		&mScene,
		true,
		true,
		XMFLOAT4(0.0, 0.2, 0.4, 1.0),
		1.0f,
		D3D_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);
	//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^//
	///////// MY GRAPHICS WATER SURFACE PIPELINE /////////

	if (mScene.GetUniformMode() == 0 || mScene.GetUniformMode() == 11)
	{
		///////// MY GRAPHICS OBSTACLE SURFACE PIPELINE /////////
		//vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv//
		mRenderer.RecordPipelineOverride(
			commandList,
			mRenderer.GetGraphicsPSO(static_cast<int>(Renderer::GraphicsStage::Obstacle)),
			mRenderer.GetGraphicsRootSignature(static_cast<int>(Renderer::GraphicsStage::Obstacle)),
			mRenderer.GetGraphicsDescriptorHeap(static_cast<int>(Renderer::GraphicsStage::Obstacle)),
			mRenderer.GetRtvHandle(frameIndex),
			mRenderer.GetDsvHandle(frameIndex),
			&mFrameObstacle,
			&mScene,
			false,//we are blending obstacle to the frame buffer, no clear
			false,//we are blending obstacle to the frame buffer, no clear
			XMFLOAT4(0,0,0,0),
			1.0f,
			D3D_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);
		//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^//
		///////// MY GRAPHICS OBSTACLE SURFACE PIPELINE /////////
	}

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
		CheckError(hr);
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
	static float foamScale = mScene.GetUniformFoamScale();

	static float timeStepFluid = mScene.GetUniformTimeStepFluid();
	static float fluidCellSize = mScene.GetUniformFluidCellSize();
	static float fluidDissipation = mScene.GetUniformFluidDissipation();
	static float vorticityScale = mScene.GetUniformVorticityScale();
	static float splatDirU = mScene.GetUniformSplatDirU();
	static float splatDirV = mScene.GetUniformSplatDirV();
	static float splatScale = mScene.GetUniformSplatScale();
	static float splatDensityU = mScene.GetUniformSplatDensityU();
	static float splatDensityV = mScene.GetUniformSplatDensityV();
	static float splatDensityRadius = mScene.GetUniformSplatDensityRadius();
	static float splatDensityScale = mScene.GetUniformSplatDensityScale();

	static float brushScale = mScene.GetUniformBrushScale();
	static float brushStrength = mScene.GetUniformBrushStrength();

	static float obstacleScale = mScene.GetUniformObstacleScale();
	static float obstacleThresholdFluid = mScene.GetUniformObstacleThresholdFluid();
	static float obstacleThresholdWave = mScene.GetUniformObstacleThresholdWave();

	static int edgeTess = mScene.GetUniformEdgeTessFactor();
	static int insideTess = mScene.GetUniformInsideTessFactor();
	static int fluidWidth = mScene.GetUniformTextureWidthFluid();
	static int fluidHeight = mScene.GetUniformTextureHeightFluid();

	static float lighthight = mScene.GetUniformLightHight();
	static float extinctcoeff = mScene.GetUniformExtinctcoeff();
	static float shiness = mScene.GetShiness();
	static float fscale = mScene.GetfScale();
	static float fbias = mScene.GetBias();
	static float fpow = mScene.GetFpow();
	static float foampower = mScene.GetFoamScale();

	bool needToUpdateSceneUniform = false;

	ImGui::SetNextWindowPos(ImVec2(0, 0));

	ImGui::Begin("Control Panel ");
	ImGui::Text("Wave Particles ");

	if (ImGui::Combo("mode", &mode, "default\0flow map\0density\0divergence\0pressure\0flow map driven texture\0wave particle\0horizontal blur\0vertical blur\0horizontal and vertical blur\0normal\0final\0\0"))
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

	if (ImGui::SliderFloat("flowSpeed ", &flowSpeed, 0.f, 0.002f, "%.6f"))
	{
		mScene.SetUniformFlowSpeed(flowSpeed);
		needToUpdateSceneUniform = true;
	}

	if (ImGui::SliderFloat("timeScale ", &timeScale, 0.0f, 100.0f, "%.6f"))
	{
		mScene.SetUniformTimeScale(timeScale);
		needToUpdateSceneUniform = true;
	}

	if (ImGui::SliderFloat("foamScale ", &foamScale, 0.0f, 10.0f, "%.6f"))
	{
		mScene.SetUniformFoamScale(foamScale);
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

	ImGui::Text("Fluid ");
	ImGui::Checkbox("fluidSim ", &FluidSimulation);
	ImGui::SliderInt("fluidSimStep ", &FluidSimulationStep, 0, 60);

	if (ImGui::SliderFloat("timeStepFluid ", &timeStepFluid, 0.0f, 0.1, "%.6f"))
	{
		mScene.SetUniformTimeStepFluid(timeStepFluid);
		needToUpdateSceneUniform = true;
	}

	if (ImGui::SliderFloat("fluidDissipation ", &fluidDissipation, 0.99f, 1.0f, "%.6f"))
	{
		mScene.SetUniformFluidDissipation(fluidDissipation);
		needToUpdateSceneUniform = true;
	}

	if (ImGui::SliderFloat("vorticityScale", &vorticityScale, 0.f, 10.f, "%.6f"))
	{
		mScene.SetUniformVorticityScale(vorticityScale);
		needToUpdateSceneUniform = true;
	}

	if (ImGui::SliderFloat("fluidCellSize ", &fluidCellSize, 0.1f, 5.0f, "%.6f"))
	{
		mScene.SetUniformFluidCellSize(fluidCellSize);
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

	if (ImGui::SliderFloat("splatScale ", &splatScale, 0.0f, 0.01f, "%.6f"))
	{
		mScene.SetUniformSplatScale(splatScale);
		needToUpdateSceneUniform = true;
	}

	if (ImGui::SliderFloat("splatDensityU ", &splatDensityU, 0.0f, 1.0f, "%.6f"))
	{
		mScene.SetUniformSplatDensityU(splatDensityU);
		needToUpdateSceneUniform = true;
	}

	if (ImGui::SliderFloat("splatDensityV ", &splatDensityV, 0.0f, 1.0f, "%.6f"))
	{
		mScene.SetUniformSplatDensityV(splatDensityV);
		needToUpdateSceneUniform = true;
	}

	if (ImGui::SliderFloat("splatDensityRadius ", &splatDensityRadius, 0.0f, 1.0f, "%.6f"))
	{
		mScene.SetUniformSplatDensityRadius(splatDensityRadius);
		needToUpdateSceneUniform = true;
	}

	if (ImGui::SliderFloat("splatDensityScale ", &splatDensityScale, 0.0f, 0.01f, "%.6f"))
	{
		mScene.SetUniformSplatDensityScale(splatDensityScale);
		needToUpdateSceneUniform = true;
	}

	if (ImGui::SliderInt("fluid width ", &fluidWidth, 0, WidthRtFluid * 2))
	{
		mScene.SetUniformTextureWidthFluid(fluidWidth);
		needToUpdateSceneUniform = true;
	}

	if (ImGui::SliderInt("fluid height ", &fluidHeight, 0, HeightRtFluid * 2))
	{
		mScene.SetUniformTextureHeightFluid(fluidHeight);
		needToUpdateSceneUniform = true;
	}

	ImGui::Text("Brush ");

	if (ImGui::Button("clearObstacle"))
	{
		ClearObstacle = true;
	}

	if (ImGui::SliderFloat("brushScale ", &brushScale, 0.0f, 1.0f, "%.6f"))
	{
		mScene.SetUniformBrushScale(brushScale);
		needToUpdateSceneUniform = true;
	}

	if (ImGui::SliderFloat("brushStrength ", &brushStrength, -1.0f, 1.0f, "%.6f"))
	{
		mScene.SetUniformBrushStrength(brushStrength);
		needToUpdateSceneUniform = true;
	}

	float brushOffsetU = mScene.GetUniformBrushOffsetU();
	float brushOffsetV = mScene.GetUniformBrushOffsetV();

	ImGui::Text("brushOffsetU: %f ", brushOffsetU);
	ImGui::Text("brushOffsetV: %f ", brushOffsetV);

	ImGui::Text("Obstacle ");

	if (ImGui::SliderFloat("obstacleScale ", &obstacleScale, 0.0f, 3.0f, "%.6f"))
	{
		mScene.SetUniformObstacleScale(obstacleScale);
		needToUpdateSceneUniform = true;
	}

	if (ImGui::SliderFloat("obstacleThresholdFluid ", &obstacleThresholdFluid, 0.0f, 1.0f, "%.6f"))
	{
		mScene.SetUniformObstacleThresholdFluid(obstacleThresholdFluid);
		needToUpdateSceneUniform = true;
	}

	if (ImGui::SliderFloat("obstacleThresholdWave ", &obstacleThresholdWave, 0.0f, 1.0f, "%.6f"))
	{
		mScene.SetUniformObstacleThresholdWave(obstacleThresholdWave);
		needToUpdateSceneUniform = true;
	}

	ImGui::Text("Rendering Properties ");

	if (ImGui::SliderFloat("light hight", &lighthight, 1, 20, "%.6f"))
	{
		mScene.SetUniformLightHight(lighthight);
		needToUpdateSceneUniform = true;
	}

	if (ImGui::SliderFloat("extinct coeff", &extinctcoeff, -0.7, 0, "%.6f"))
	{
		mScene.SetUniformExtinctcoeff(extinctcoeff);
		needToUpdateSceneUniform = true;
	}

	if (ImGui::SliderFloat("Shiness", &shiness, 10, 600, "%.6f"))
	{
		mScene.SetShiness(shiness);
		needToUpdateSceneUniform = true;
	}

	if (ImGui::SliderFloat("fresnel scale", &fscale, 0, 1, "%.6f"))
	{
		mScene.SetFScale(fscale);
		needToUpdateSceneUniform = true;
	}

	if (ImGui::SliderFloat("fresnel bias", &fbias, 0, 4, "%.6f"))
	{
		mScene.SetBias(fbias);
		needToUpdateSceneUniform = true;
	}

	if (ImGui::SliderFloat("fresnel power", &fpow, 0, 5, "%.6f"))
	{
		mScene.SetFPow(fpow);
		needToUpdateSceneUniform = true;
	}

	if (ImGui::SliderFloat("Foam power", &foampower, 3, 15, "%.6f"))
	{
		mScene.SetFoamScale(foampower);
		needToUpdateSceneUniform = true;
	}

	ImGui::Text("%.3f ms/frame (%.1f FPS) ", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	ImGui::Text("Hold C and use mouse to rotate camera.");
	ImGui::Text("Hold B and use mouse to control brush.");
	ImGui::End();

	if (needToUpdateSceneUniform)
	{
		mScene.UpdateUniformBuffer();
	}

	//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^//
	////////// IMGUI EXAMPLES///////////
	////////////////////////////////////
}

void Cleanup()
{
	// wait for the gpu to finish all frames
	for (int i = 0; i < FrameBufferCount; ++i)
	{
		frameIndex = i;
		//fenceValue[i]++;
		commandQueue->Signal(fence[i], fenceValue[i]);
		WaitForPreviousFrame(i);
	}

	// close the fence event
	CloseHandle(fenceEvent);
	mRenderer.Release();
	mScene.Release();

	//imgui stuff
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
	SAFE_RELEASE(g_pd3dSrvDescHeap);

	//direct input stuff
	DIKeyboard->Unacquire();
	DIMouse->Unacquire();
	DirectInput->Release();


	// get swapchain out of full screen before exiting
	BOOL fs = false;
	swapChain->GetFullscreenState(&fs, NULL);
	if (fs == TRUE)
		swapChain->SetFullscreenState(false, NULL);

	for (int i = 0; i < FrameBufferCount; ++i)
	{
		SAFE_RELEASE(commandAllocator[i]);
		SAFE_RELEASE(fence[i]);
	};

	SAFE_RELEASE(commandList);
	SAFE_RELEASE(commandQueue);
	SAFE_RELEASE(swapChain);

#ifdef MY_DEBUG
	//report live objects
	ID3D12DebugDevice* debugDev;
	bool hasDebugDev = false;
	if (SUCCEEDED(device->QueryInterface(IID_PPV_ARGS(&debugDev))))
	{
		hasDebugDev = true;
	}
#endif 

	SAFE_RELEASE(device);

#ifdef MY_DEBUG
	if (hasDebugDev)
	{
		OutputDebugStringW(L"Debug Report Live Device Objects >>>>>>>>>>>>>>>>>>>>>>>>>>\n");
		debugDev->ReportLiveDeviceObjects(D3D12_RLDO_DETAIL);
		OutputDebugStringW(L"Debug Report Live Device Objects >>>>>>>>>>>>>>>>>>>>>>>>>>\n");
	}
	SAFE_RELEASE(debugDev);
	SAFE_RELEASE(debugController);
#endif
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
bool InitWindow(HINSTANCE hInstance, int ShowWnd,	bool fullscreen)
{
	if (fullscreen)
	{
		HMONITOR hmon = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
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
		
		{
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
	if (!InitWindow(hInstance, nShowCmd, FullScreen))
	{
		MessageBox(0, L"Window Initialization - Failed", L"Error", MB_OK);
		return 1;
	}

	// initialize input device
	if (!InitDirectInput(hInstance))
	{
		MessageBox(0, L"Failed to initialize input", L"Error", MB_OK);
		return 1;
	}

	// OK, it seems I messed up a lot of resource state stuff.
	// Be advised if you choose to enable debug layer.
	// --- ALL FIXED

#ifdef MY_DEBUG
	// debug layer
	if (!EnableDebugLayer())
	{
		MessageBox(0, L"Failed to enable debug layer", L"Error", MB_OK);
		return 1;
	}

	// gpu-based validation
	// this is problematic for the moment, don't know why 'FluidVelocity2' always report incompatible resource state even though I have proper barriers
	// it is used by both domain shader and pixel shader, but it seems the transition to (D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE) is not enough for some reason
	//if (!EnableShaderBasedValidation())
	//{
	//	MessageBox(0, L"Failed to enable shader based validation", L"Error", MB_OK);
	//	return 1;
	//}
#endif

	// initialize direct3d
	if (!InitD3D())
	{
		MessageBox(0, L"Failed to initialize direct3d 12", L"Error", MB_OK);
		Cleanup();
		return 1;
	}

	// start the main loop
	mainloop();

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