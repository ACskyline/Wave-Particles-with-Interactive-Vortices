#pragma once

#include <dxgi1_4.h>
#include <d3d12.h>
#include <DirectXMath.h>
#include <string>
#include "Dependencies/d3dx12.h"

#define SAFE_RELEASE(p) { if ( (p) ) { (p)->Release(); (p) = 0; } }
#define KEYDOWN(name, key) ((name)[(key)] & 0x80)
#define EPSILON 0.00000001

using namespace DirectX;
using namespace std;

const int FrameBufferCount = 3;
const int MultiSampleCount = 1;

const D3D12_INPUT_ELEMENT_DESC VertexInputLayout[] =
{
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
};

struct Vertex {
	Vertex() { pos = { 0, 0, 0 }; texCoord = { 0, 0 }; }
	Vertex(float x, float y, float z, float u, float v) : pos(x, y, z), texCoord(u, v) {}
	XMFLOAT3 pos;
	XMFLOAT2 texCoord;
};

bool CheckError(HRESULT hr, ID3D10Blob* error_message);

#define CellSize (1.25f)
#define ViewportWidth (1024)
#define ViewportHeight (768)
#define GridWidth (ViewportWidth / 2)
#define GridHeight (ViewportHeight / 2)
#define SplatRadius ((float) GridWidth / 8.0f)