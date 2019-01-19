#pragma once

#include "GlobalInclude.h"
#include <wincodec.h>

class Texture
{
public:
	Texture(const wstring& _fileName);
	virtual ~Texture();

	bool LoadTextureBuffer();
	bool LoadTextureBufferFromFile(const wstring& _fileName);
	virtual bool CreateTextureBuffer(ID3D12Device* device);
	virtual bool UpdateTextureBuffer(ID3D12Device* device);
	ID3D12Resource* GetTextureBuffer();
	D3D12_SHADER_RESOURCE_VIEW_DESC GetSrvDesc();

	virtual bool InitTexture(ID3D12Device* device);
	void ReleaseBuffer();
	void ReleaseBufferCPU();
	wstring GetName();

protected:
	wstring fileName;

	ID3D12Resource* textureBuffer;
	D3D12_RESOURCE_DESC textureDesc;//contains format
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
	int imageBytesPerRow;
	BYTE* imageData;

	Texture();

	int LoadImageDataFromFile(BYTE** imageData, 
		D3D12_RESOURCE_DESC& resourceDescription, 
		LPCWSTR filename, 
		int &bytesPerRow);

	int GetDXGIFormatBitsPerPixel(DXGI_FORMAT& dxgiFormat);
	DXGI_FORMAT GetDXGIFormatFromWICFormat(WICPixelFormatGUID& wicFormatGUID);
	WICPixelFormatGUID GetConvertToWICFormat(WICPixelFormatGUID& wicFormatGUID);
};

class RenderTexture : public Texture
{
public:
	RenderTexture(int _width, int _height, DXGI_FORMAT format, bool _supportDepth = false);
	RenderTexture(int _width, int _height, const wstring& _fileName, DXGI_FORMAT format, bool _supportDepth = false);
	~RenderTexture();

	bool CreateTextureBuffer(ID3D12Device* device);
	bool UpdateTextureBuffer(ID3D12Device* device);
	void UpdateViewport();
	void UpdateScissorRect();

	void SetRtvHandle(CD3DX12_CPU_DESCRIPTOR_HANDLE _rtvHandle);
	void SetDsvHandle(CD3DX12_CPU_DESCRIPTOR_HANDLE _dsvHandle);
	void SetResourceState(D3D12_RESOURCE_STATES _resourceState);

	bool SupportDepth();
	ID3D12Resource* GetDepthStencilBuffer();
	D3D12_VIEWPORT GetViewport();
	D3D12_RECT GetScissorRect();
	CD3DX12_CPU_DESCRIPTOR_HANDLE GetRtvHandle();
	CD3DX12_CPU_DESCRIPTOR_HANDLE GetDsvHandle();
	D3D12_RENDER_TARGET_VIEW_DESC GetRtvDesc();
	D3D12_DEPTH_STENCIL_VIEW_DESC GetDsvDesc();
	D3D12_RESOURCE_STATES GetResourceState();

	CD3DX12_RESOURCE_BARRIER TransitionToResourceState(D3D12_RESOURCE_STATES _resourceState);

	bool InitTexture(ID3D12Device* device);
	void ReleaseBuffers();
private:
	bool supportDepth;
	ID3D12Resource* depthStencilBuffer;
	D3D12_RESOURCE_DESC depthStencilBufferDesc;//contains format

	int width;
	int height;
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc;
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle;
	CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle;
	D3D12_RESOURCE_STATES resourceState;
	D3D12_VIEWPORT viewport;
	D3D12_RECT scissorRect;
};
