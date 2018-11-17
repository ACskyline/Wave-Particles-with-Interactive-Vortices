#pragma once

#include "GlobalInclude.h"
#include <wincodec.h>

class Texture
{
public:
	Texture();
	~Texture();

	bool CreateTextureBufferFromFile(ID3D12Device* device, const wstring& fileName);
	bool UpdateTextureBuffer(ID3D12Device* device);
	ID3D12Resource* GetTextureBuffer();
	D3D12_SHADER_RESOURCE_VIEW_DESC GetSrvDesc();
	//void ReleaseBufferCPU();//in our demo there is no need to free CPU memory before delete this object
	void ReleaseBuffer();

private:
	ID3D12Resource* textureBuffer;
	D3D12_RESOURCE_DESC textureDesc;
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
	int imageBytesPerRow;
	BYTE* imageData;

	int LoadImageDataFromFile(BYTE** imageData, 
		D3D12_RESOURCE_DESC& resourceDescription, 
		LPCWSTR filename, 
		int &bytesPerRow);

	int GetDXGIFormatBitsPerPixel(DXGI_FORMAT& dxgiFormat);

	DXGI_FORMAT GetDXGIFormatFromWICFormat(WICPixelFormatGUID& wicFormatGUID);

	WICPixelFormatGUID GetConvertToWICFormat(WICPixelFormatGUID& wicFormatGUID);
};

