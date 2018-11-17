#pragma once

#include "GlobalInclude.h"
#include "Shader.h"
#include "Texture.h"

class Renderer
{
public:
	Renderer();
	~Renderer();
	
	bool CreateRootSignature(ID3D12Device* device);
	bool CreatePSO(ID3D12Device* device, Shader* vertexShader, Shader* pixelShader);
	bool CreateDepthStencilBuffer(ID3D12Device* device, float Width, float Height);
	bool CreateMainDescriptorHeap(ID3D12Device* device);
	bool BindTextureToMainDescriptor(ID3D12Device* device, Texture* texture);

	ID3D12PipelineState* GetPSO();
	ID3D12DescriptorHeap* GetDsHeap();
	ID3D12RootSignature* GetRootSignature();
	ID3D12DescriptorHeap* GetMainHeap();

	void Release();

private:
	DXGI_SAMPLE_DESC sampleDesc;
	ID3D12PipelineState* pipelineStateObject; // pso containing a pipeline state
	ID3D12RootSignature* rootSignature; // root signature defines data shaders will access
	ID3D12DescriptorHeap* mainDescriptorHeap;
	ID3D12Resource* depthStencilBuffer; // This is the memory for our depth buffer. it will also be used for a stencil buffer in a later tutorial
	ID3D12DescriptorHeap* dsDescriptorHeap; // This is a heap for our depth/stencil buffer descriptor
};

