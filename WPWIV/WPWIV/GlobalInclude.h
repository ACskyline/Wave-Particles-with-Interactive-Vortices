#pragma once

#include <dxgi1_4.h>
#include <d3d12.h>
#include <DirectXMath.h>
#include <string>
#include "Dependencies/d3dx12.h"

#define MY_DEBUG

#define SAFE_RELEASE(p) { if ( (p) ) { (p)->Release(); (p) = 0; } }
#define SAFE_RELEASE_ARRAY(p) { int n = _countof(p); for(int i = 0;i<n;i++){ SAFE_RELEASE(p[i]); } }
#define KEYDOWN(name, key) ((name)[(key)] & 0x80)
#define EPSILON 0.00000001
#define SIZEOF_ARRAY(arr) sizeof()

using namespace DirectX;
using namespace std;

enum UNIFORM_SLOT { OBJECT, CAMERA, FRAME, SCENE, TABLE, COUNT };

const int FrameBufferCount = 3;
const int MultiSampleCount = 1;
const int JacobiIteration = 40;

const D3D12_INPUT_ELEMENT_DESC VertexInputLayout[] =
{
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
};

struct Vertex {
	Vertex() : Vertex(0, 0, 0, 0, 0, 0, 0, 0) {}
	Vertex(float x, float y, float z, float u, float v) : Vertex(x, y, z, u, v, 0, 0, 0) {}
	Vertex(float x, float y, float z, float u, float v, float nx, float ny, float nz) : pos(x, y, z), texCoord(u, v), nor(nx, ny, nz) {}
	XMFLOAT3 pos;
	XMFLOAT2 texCoord;
	XMFLOAT3 nor;
};

bool CheckError(HRESULT hr, ID3D10Blob* error_message = nullptr);