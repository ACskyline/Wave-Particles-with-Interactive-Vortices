#pragma once

#include <wincodec.h>
#include <d3d12.h>

class Image
{
public:
	Image();
	~Image();

	//int LoadImageDataFromFile(BYTE** imageData, D3D12_RESOURCE_DESC& resourceDescription, LPCWSTR filename, int &bytesPerRow);

	//DXGI_FORMAT GetDXGIFormatFromWICFormat(WICPixelFormatGUID& wicFormatGUID);
	//WICPixelFormatGUID GetConvertToWICFormat(WICPixelFormatGUID& wicFormatGUID);
	//int GetDXGIFormatBitsPerPixel(DXGI_FORMAT& dxgiFormat);
};

