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
	//void ReleaseBufferCPU();//in our demo there is no need to free CPU memory before delete this object
	void ReleaseBuffer();

	bool InitTexture(ID3D12Device* device);

protected:
	wstring fileName;

	ID3D12Resource* textureBuffer;
	D3D12_RESOURCE_DESC textureDesc;
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
	RenderTexture(int _width, int _height);
	virtual bool CreateTextureBuffer(ID3D12Device* device);
	virtual bool UpdateTextureBuffer(ID3D12Device* device);

	CD3DX12_CPU_DESCRIPTOR_HANDLE GetRtvHandle();

private:
	int width;
	int height;
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc;
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle;//not used currently
};