#pragma once

#include <d3d12.h>
#include <dxgi1_4.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include "Dependencies/d3dx12.h"

using namespace DirectX; // we will be using the directxmath library

class Renderer
{
public:
	Renderer();
	~Renderer();
	
	//ID3D12Device* device; // direct3d device

	//IDXGISwapChain3* swapChain; // swapchain used to switch between render targets

	//ID3D12CommandQueue* commandQueue; // container for command lists

	//ID3D12DescriptorHeap* rtvDescriptorHeap; // a descriptor heap to hold resources like the render targets

	//ID3D12Resource* renderTargets[frameBufferCount]; // number of render targets equal to buffer count

	//ID3D12CommandAllocator* commandAllocator[frameBufferCount]; // we want enough allocators for each buffer * number of threads (we only have one thread)

	//ID3D12GraphicsCommandList* commandList; // a command list we can record commands into, then execute them to render the frame

	//ID3D12Fence* fence[frameBufferCount];    // an object that is locked while our command list is being executed by the gpu. We need as many 
	//										 //as we have allocators (more if we want to know when the gpu is finished with an asset)

	//HANDLE fenceEvent; // a handle to an event when our fence is unlocked by the gpu

	//UINT64 fenceValue[frameBufferCount]; // this value is incremented each frame. each fence will have its own value

	//int frameIndex; // current rtv we are on

	//int rtvDescriptorSize; // size of the rtv descriptor on the device (all front and back buffers will be the same size)
	//					   // function declarations

	//bool InitD3D(); // initializes direct3d 12

	//void Update(); // update the game logic

	//void UpdatePipeline(); // update the direct3d pipeline (update command lists)

	//void Render(); // execute the command list

	//void Cleanup(); // release com ojects and clean up memory

	//void WaitForPreviousFrame(); // wait until gpu is finished with command list

	//ID3D12PipelineState* pipelineStateObject; // pso containing a pipeline state

	//ID3D12RootSignature* rootSignature; // root signature defines data shaders will access

	//D3D12_VIEWPORT viewport; // area that output from rasterizer will be stretched to.

	//D3D12_RECT scissorRect; // the area to draw in. pixels outside that area will not be drawn onto

	//ID3D12Resource* vertexBuffer; // a default buffer in GPU memory that we will load vertex data for our triangle into
	//ID3D12Resource* indexBuffer; // a default buffer in GPU memory that we will load index data for our triangle into

	//D3D12_VERTEX_BUFFER_VIEW vertexBufferView; // a structure containing a pointer to the vertex data in gpu memory
	//										   // the total size of the buffer, and the size of each element (vertex)

	//D3D12_INDEX_BUFFER_VIEW indexBufferView; // a structure holding information about the index buffer

	//ID3D12Resource* depthStencilBuffer; // This is the memory for our depth buffer. it will also be used for a stencil buffer in a later tutorial
	//ID3D12DescriptorHeap* dsDescriptorHeap; // This is a heap for our depth/stencil buffer descriptor

	//// this is the structure of our constant buffer.
	//struct ConstantBufferPerObject {
	//	XMFLOAT4X4 wvpMat;
	//};

	//// Constant buffers must be 256-byte aligned which has to do with constant reads on the GPU.
	//// We are only able to read at 256 byte intervals from the start of a resource heap, so we will
	//// make sure that we add padding between the two constant buffers in the heap (one for cube1 and one for cube2)
	//// Another way to do this would be to add a float array in the constant buffer structure for padding. In this case
	//// we would need to add a float padding[50]; after the wvpMat variable. This would align our structure to 256 bytes (4 bytes per float)
	//// The reason i didn't go with this way, was because there would actually be wasted cpu cycles when memcpy our constant
	//// buffer data to the gpu virtual address. currently we memcpy the size of our structure, which is 16 bytes here, but if we
	//// were to add the padding array, we would memcpy 64 bytes if we memcpy the size of our structure, which is 50 wasted bytes
	//// being copied.
	//int ConstantBufferPerObjectAlignedSize = (sizeof(ConstantBufferPerObject) + 255) & ~255;

	//ConstantBufferPerObject cbPerObject; // this is the constant buffer data we will send to the gpu 
	//										// (which will be placed in the resource we created above)

	//ID3D12Resource* constantBufferUploadHeaps[frameBufferCount]; // this is the memory on the gpu where constant buffers for each frame will be placed

	//UINT8* cbvGPUAddress[frameBufferCount]; // this is a pointer to each of the constant buffer resource heaps

	//XMFLOAT4X4 cameraProjMat; // this will store our projection matrix
	//XMFLOAT4X4 cameraViewMat; // this will store our view matrix

	//XMFLOAT4 cameraPosition; // this is our cameras position vector
	//XMFLOAT4 cameraTarget; // a vector describing the point in space our camera is looking at
	//XMFLOAT4 cameraUp; // the worlds up vector

	//XMFLOAT4X4 cube1WorldMat; // our first cubes world matrix (transformation matrix)
	//XMFLOAT4X4 cube1RotMat; // this will keep track of our rotation for the first cube
	//XMFLOAT4 cube1Position; // our first cubes position in space

	//XMFLOAT4X4 cube2WorldMat; // our first cubes world matrix (transformation matrix)
	//XMFLOAT4X4 cube2RotMat; // this will keep track of our rotation for the second cube
	//XMFLOAT4 cube2PositionOffset; // our second cube will rotate around the first cube, so this is the position offset from the first cube

	//int numCubeIndices; // the number of indices to draw the cube

	//ID3D12Resource* textureBuffer; // the resource heap containing our texture


	//ID3D12DescriptorHeap* mainDescriptorHeap;
	//ID3D12Resource* textureBufferUploadHeap;
	//struct Vertex {
	//	Vertex(float x, float y, float z, float u, float v) : pos(x, y, z), texCoord(u, v) {}
	//	XMFLOAT3 pos;
	//	XMFLOAT2 texCoord;
	//};
};

