#include "Texture.h"

Texture::Texture(const wstring& _fileName) :
	fileName(_fileName),
	imageData(nullptr),
	textureBuffer(nullptr)
{
}

Texture::Texture() : 
	Texture(L"no file")
{
}

Texture::~Texture()
{
	ReleaseBuffer();
	ReleaseBufferCPU();
}

void Texture::ReleaseBuffer()
{
	SAFE_RELEASE(textureBuffer);
}

void Texture::ReleaseBufferCPU()
{
	if (imageData != nullptr)
	{
		delete imageData;
		imageData = nullptr;
	}
}

bool Texture::LoadTextureBuffer()
{
	// load the image, create a texture resource and descriptor heap

	// Load the image from file
	int imageSize = LoadImageDataFromFile(&imageData, textureDesc, fileName.c_str(), imageBytesPerRow);

	// make sure we have data
	if (imageSize <= 0)
	{
		return false;
	}

	return true;
}

bool Texture::LoadTextureBufferFromFile(const wstring& _fileName)
{
	// load the image, create a texture resource and descriptor heap

	// Load the image from file
	int imageSize = LoadImageDataFromFile(&imageData, textureDesc, _fileName.c_str(), imageBytesPerRow);

	// make sure we have data
	if (imageSize <= 0)
	{
		return false;
	}

	return true;
}

bool Texture::CreateTextureBuffer(ID3D12Device* device)
{
	HRESULT hr;
	// create a default heap where the upload heap will copy its contents into (contents being the texture)
	hr = device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), // a default heap
		D3D12_HEAP_FLAG_NONE, // no flags
		&textureDesc, // the description of our texture
		D3D12_RESOURCE_STATE_COPY_DEST, // We will copy the texture from the upload heap to here, so we start it out in a copy dest state
		nullptr, // used for render targets and depth/stencil buffers
		IID_PPV_ARGS(&textureBuffer));

	if (FAILED(hr))
	{
		return false;
	}
	//textureBuffer->SetName(L"Texture Buffer Resource Heap");
	textureBuffer->SetName(fileName.c_str());

	return true;
}

bool Texture::UpdateTextureBuffer(ID3D12Device* device)
{
	HRESULT hr;

	ID3D12CommandQueue* immediateCopyCommandQueue;
	ID3D12CommandAllocator* immediateCopyCommandAllocator;
	ID3D12GraphicsCommandList* immediateCopyCommandList;
	ID3D12Resource* immediateCopyBuffer;
	ID3D12Fence* fence;
	HANDLE fenceEvent; // a handle to an event when our fence is unlocked by the gpu
	UINT64 fenceValue; // this value is incremented each frame. each fence will have its own value

	// -- Create fence related resources -- //
	fenceValue = 0; // set the initial fence value to 0
	hr = device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
	if (FAILED(hr))
	{
		return false;
	}
	fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	if (fenceEvent == nullptr)
	{
		return false;
	}

	// -- Create a direct command queue -- //
	D3D12_COMMAND_QUEUE_DESC cqDesc = {};
	cqDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	cqDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT; // direct means the gpu can directly execute this command queue
	hr = device->CreateCommandQueue(&cqDesc, IID_PPV_ARGS(&immediateCopyCommandQueue)); // create the command queue
	if (FAILED(hr))
	{
		return false;
	}

	immediateCopyCommandQueue->SetName(L"texture immediate copy");
	// -- Create a command allocator -- //
	hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&immediateCopyCommandAllocator));
	if (FAILED(hr))
	{
		return false;
	}

	// -- Create a command list -- //
	hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, immediateCopyCommandAllocator, NULL, IID_PPV_ARGS(&immediateCopyCommandList));
	if (FAILED(hr))
	{
		return false;
	}

	UINT64 textureUploadBufferSize;
	// this function gets the size an upload buffer needs to be to upload a texture to the gpu.
	// each row must be 256 byte aligned except for the last row, which can just be the size in bytes of the row
	// eg. textureUploadBufferSize = ((((width * numBytesPerPixel) + 255) & ~255) * (height - 1)) + (width * numBytesPerPixel);
	//textureUploadBufferSize = (((imageBytesPerRow + 255) & ~255) * (textureDesc.Height - 1)) + imageBytesPerRow;
	device->GetCopyableFootprints(&textureDesc, 0, 1, 0, nullptr, nullptr, nullptr, &textureUploadBufferSize);

	// -- Creaete an upload buffer -- //
	hr = device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), // upload heap
		D3D12_HEAP_FLAG_NONE, // no flags
		&CD3DX12_RESOURCE_DESC::Buffer(textureUploadBufferSize), // resource description for a buffer (storing the image data in this heap just to copy to the default heap)
		D3D12_RESOURCE_STATE_GENERIC_READ, // We will copy the contents from this heap to the default heap above
		nullptr,
		IID_PPV_ARGS(&immediateCopyBuffer));
	if (FAILED(hr))
	{
		return false;
	}
	immediateCopyBuffer->SetName(L"texture upload buffer");

	// store vertex buffer in upload heap
	D3D12_SUBRESOURCE_DATA textureData = {};
	textureData.pData = imageData; // pointer to our image data
	textureData.RowPitch = imageBytesPerRow; // size of all our triangle vertex data
	textureData.SlicePitch = static_cast<LONG_PTR>(imageBytesPerRow) * static_cast<LONG_PTR>(textureDesc.Height); // also the size of our triangle vertex data

	// Now we copy the upload buffer contents to the default heap
	UpdateSubresources(immediateCopyCommandList, textureBuffer, immediateCopyBuffer, 0, 0, 1, &textureData);

	// transition the texture default heap to a pixel shader resource (we will be sampling from this heap in the pixel shader to get the color of pixels)
	immediateCopyCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(textureBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = textureDesc.Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;

	immediateCopyCommandList->Close();
	ID3D12CommandList* commandLists[] = { immediateCopyCommandList };
	immediateCopyCommandQueue->ExecuteCommandLists(_countof(commandLists), commandLists);
	
	// -- Use fence to wait until finish -- //
	fenceValue++;
	hr = immediateCopyCommandQueue->Signal(fence, fenceValue);
	if (FAILED(hr))
	{
		return false;
	}

	if (fence->GetCompletedValue() < fenceValue)
	{
		// we have the fence create an event which is signaled once the fence's current value is "fenceValue"
		hr = fence->SetEventOnCompletion(fenceValue, fenceEvent);
		if (FAILED(hr))
		{
			return false;
		}

		// We will wait until the fence has triggered the event that it's current value has reached "fenceValue". once it's value
		// has reached "fenceValue", we know the command queue has finished executing
		WaitForSingleObject(fenceEvent, INFINITE);
	}

	// -- Release -- //
	SAFE_RELEASE(fence);
	SAFE_RELEASE(immediateCopyCommandQueue);
	SAFE_RELEASE(immediateCopyCommandAllocator);
	SAFE_RELEASE(immediateCopyCommandList);
	SAFE_RELEASE(immediateCopyBuffer);

	return true;
}

ID3D12Resource* Texture::GetTextureBuffer()
{
	return textureBuffer;
}

D3D12_SHADER_RESOURCE_VIEW_DESC Texture::GetSrvDesc()
{
	return srvDesc;
}

//void Texture::ReleaseBufferCPU()
//{
//	free(imageData);
//}

wstring Texture::GetName()
{
	return fileName;
}

bool Texture::InitTexture(ID3D12Device* device)
{
	if (!CreateTextureBuffer(device))
	{
		printf("CreateTextureBuffer failed\n");
		return false;
	}

	if (!UpdateTextureBuffer(device))
	{
		printf("UpdateTextureBuffer failed\n");
		return false;
	}

	return true;
}

int Texture::LoadImageDataFromFile(BYTE** imageData, D3D12_RESOURCE_DESC& resourceDescription, LPCWSTR filename, int &bytesPerRow)
{
	HRESULT hr;

	// we only need one instance of the imaging factory to create decoders and frames
	IWICImagingFactory *wicFactory = NULL;

	// reset decoder, frame and converter since these will be different for each image we load
	IWICBitmapDecoder *wicDecoder = NULL;
	IWICBitmapFrameDecode *wicFrame = NULL;
	IWICFormatConverter *wicConverter = NULL;

	bool imageConverted = false;

	if (wicFactory == NULL)
	{
		// Initialize the COM library
		hr = CoInitialize(NULL);
		if (FAILED(hr)) return 0;

		// create the WIC factory
		hr = CoCreateInstance(
			CLSID_WICImagingFactory,
			NULL,
			CLSCTX_INPROC_SERVER,
			IID_PPV_ARGS(&wicFactory)
		);
		if (FAILED(hr)) return 0;
	}

	// load a decoder for the image
	hr = wicFactory->CreateDecoderFromFilename(
		filename,                        // Image we want to load in
		NULL,                            // This is a vendor ID, we do not prefer a specific one so set to null
		GENERIC_READ,                    // We want to read from this file
		WICDecodeMetadataCacheOnLoad,    // We will cache the metadata right away, rather than when needed, which might be unknown
		&wicDecoder                      // the wic decoder to be created
	);
	if (FAILED(hr)) return 0;

	// get image from decoder (this will decode the "frame")
	hr = wicDecoder->GetFrame(0, &wicFrame);
	if (FAILED(hr)) return 0;

	// get wic pixel format of image
	WICPixelFormatGUID pixelFormat;
	hr = wicFrame->GetPixelFormat(&pixelFormat);
	if (FAILED(hr)) return 0;

	// get size of image
	UINT textureWidth, textureHeight;
	hr = wicFrame->GetSize(&textureWidth, &textureHeight);
	if (FAILED(hr)) return 0;

	// we are not handling sRGB types in this tutorial, so if you need that support, you'll have to figure
	// out how to implement the support yourself

	// convert wic pixel format to dxgi pixel format
	DXGI_FORMAT dxgiFormat = GetDXGIFormatFromWICFormat(pixelFormat);

	// if the format of the image is not a supported dxgi format, try to convert it
	if (dxgiFormat == DXGI_FORMAT_UNKNOWN)
	{
		// get a dxgi compatible wic format from the current image format
		WICPixelFormatGUID convertToPixelFormat = GetConvertToWICFormat(pixelFormat);

		// return if no dxgi compatible format was found
		if (convertToPixelFormat == GUID_WICPixelFormatDontCare) return 0;

		// set the dxgi format
		dxgiFormat = GetDXGIFormatFromWICFormat(convertToPixelFormat);

		// create the format converter
		hr = wicFactory->CreateFormatConverter(&wicConverter);
		if (FAILED(hr)) return 0;

		// make sure we can convert to the dxgi compatible format
		BOOL canConvert = FALSE;
		hr = wicConverter->CanConvert(pixelFormat, convertToPixelFormat, &canConvert);
		if (FAILED(hr) || !canConvert) return 0;

		// do the conversion (wicConverter will contain the converted image)
		hr = wicConverter->Initialize(wicFrame, convertToPixelFormat, WICBitmapDitherTypeErrorDiffusion, 0, 0, WICBitmapPaletteTypeCustom);
		if (FAILED(hr)) return 0;

		// this is so we know to get the image data from the wicConverter (otherwise we will get from wicFrame)
		imageConverted = true;
	}

	int bitsPerPixel = GetDXGIFormatBitsPerPixel(dxgiFormat); // number of bits per pixel
	bytesPerRow = (textureWidth * bitsPerPixel) / 8; // number of bytes in each row of the image data
	int imageSize = bytesPerRow * textureHeight; // total image size in bytes

	// allocate enough memory for the raw image data, and set imageData to point to that memory
	*imageData = (BYTE*)malloc(imageSize);

	// copy (decoded) raw image data into the newly allocated memory (imageData)
	if (imageConverted)
	{
		// if image format needed to be converted, the wic converter will contain the converted image
		hr = wicConverter->CopyPixels(0, bytesPerRow, imageSize, *imageData);
		if (FAILED(hr)) return 0;
	}
	else
	{
		// no need to convert, just copy data from the wic frame
		hr = wicFrame->CopyPixels(0, bytesPerRow, imageSize, *imageData);
		if (FAILED(hr)) return 0;
	}

	// now describe the texture with the information we have obtained from the image
	resourceDescription = {};
	resourceDescription.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resourceDescription.Alignment = 0; // may be 0, 4KB, 64KB, or 4MB. 0 will let runtime decide between 64KB and 4MB (4MB for multi-sampled textures)
	resourceDescription.Width = textureWidth; // width of the texture
	resourceDescription.Height = textureHeight; // height of the texture
	resourceDescription.DepthOrArraySize = 1; // if 3d image, depth of 3d image. Otherwise an array of 1D or 2D textures (we only have one image, so we set 1)
	resourceDescription.MipLevels = 1; // Number of mipmaps. We are not generating mipmaps for this texture, so we have only one level
	resourceDescription.Format = dxgiFormat; // This is the dxgi format of the image (format of the pixels)
	resourceDescription.SampleDesc.Count = 1; // This is the number of samples per pixel, we just want 1 sample
	resourceDescription.SampleDesc.Quality = 0; // The quality level of the samples. Higher is better quality, but worse performance
	resourceDescription.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN; // The arrangement of the pixels. Setting to unknown lets the driver choose the most efficient one
	resourceDescription.Flags = D3D12_RESOURCE_FLAG_NONE; // no flags

	// return the size of the image. remember to delete the image once your done with it (in this tutorial once its uploaded to the gpu)
	return imageSize;
}

// get the dxgi format equivilent of a wic format
DXGI_FORMAT Texture::GetDXGIFormatFromWICFormat(WICPixelFormatGUID& wicFormatGUID)
{
	if (wicFormatGUID == GUID_WICPixelFormat128bppRGBAFloat) return DXGI_FORMAT_R32G32B32A32_FLOAT;
	else if (wicFormatGUID == GUID_WICPixelFormat64bppRGBAHalf) return DXGI_FORMAT_R16G16B16A16_FLOAT;
	else if (wicFormatGUID == GUID_WICPixelFormat64bppRGBA) return DXGI_FORMAT_R16G16B16A16_UNORM;
	else if (wicFormatGUID == GUID_WICPixelFormat32bppRGBA) return DXGI_FORMAT_R8G8B8A8_UNORM;
	else if (wicFormatGUID == GUID_WICPixelFormat32bppBGRA) return DXGI_FORMAT_B8G8R8A8_UNORM;
	else if (wicFormatGUID == GUID_WICPixelFormat32bppBGR) return DXGI_FORMAT_B8G8R8X8_UNORM;
	else if (wicFormatGUID == GUID_WICPixelFormat32bppRGBA1010102XR) return DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM;

	else if (wicFormatGUID == GUID_WICPixelFormat32bppRGBA1010102) return DXGI_FORMAT_R10G10B10A2_UNORM;
	else if (wicFormatGUID == GUID_WICPixelFormat16bppBGRA5551) return DXGI_FORMAT_B5G5R5A1_UNORM;
	else if (wicFormatGUID == GUID_WICPixelFormat16bppBGR565) return DXGI_FORMAT_B5G6R5_UNORM;
	else if (wicFormatGUID == GUID_WICPixelFormat32bppGrayFloat) return DXGI_FORMAT_R32_FLOAT;
	else if (wicFormatGUID == GUID_WICPixelFormat16bppGrayHalf) return DXGI_FORMAT_R16_FLOAT;
	else if (wicFormatGUID == GUID_WICPixelFormat16bppGray) return DXGI_FORMAT_R16_UNORM;
	else if (wicFormatGUID == GUID_WICPixelFormat8bppGray) return DXGI_FORMAT_R8_UNORM;
	else if (wicFormatGUID == GUID_WICPixelFormat8bppAlpha) return DXGI_FORMAT_A8_UNORM;

	else return DXGI_FORMAT_UNKNOWN;
}

// get a dxgi compatible wic format from another wic format
WICPixelFormatGUID Texture::GetConvertToWICFormat(WICPixelFormatGUID& wicFormatGUID)
{
	if (wicFormatGUID == GUID_WICPixelFormatBlackWhite) return GUID_WICPixelFormat8bppGray;
	else if (wicFormatGUID == GUID_WICPixelFormat1bppIndexed) return GUID_WICPixelFormat32bppRGBA;
	else if (wicFormatGUID == GUID_WICPixelFormat2bppIndexed) return GUID_WICPixelFormat32bppRGBA;
	else if (wicFormatGUID == GUID_WICPixelFormat4bppIndexed) return GUID_WICPixelFormat32bppRGBA;
	else if (wicFormatGUID == GUID_WICPixelFormat8bppIndexed) return GUID_WICPixelFormat32bppRGBA;
	else if (wicFormatGUID == GUID_WICPixelFormat2bppGray) return GUID_WICPixelFormat8bppGray;
	else if (wicFormatGUID == GUID_WICPixelFormat4bppGray) return GUID_WICPixelFormat8bppGray;
	else if (wicFormatGUID == GUID_WICPixelFormat16bppGrayFixedPoint) return GUID_WICPixelFormat16bppGrayHalf;
	else if (wicFormatGUID == GUID_WICPixelFormat32bppGrayFixedPoint) return GUID_WICPixelFormat32bppGrayFloat;
	else if (wicFormatGUID == GUID_WICPixelFormat16bppBGR555) return GUID_WICPixelFormat16bppBGRA5551;
	else if (wicFormatGUID == GUID_WICPixelFormat32bppBGR101010) return GUID_WICPixelFormat32bppRGBA1010102;
	else if (wicFormatGUID == GUID_WICPixelFormat24bppBGR) return GUID_WICPixelFormat32bppRGBA;
	else if (wicFormatGUID == GUID_WICPixelFormat24bppRGB) return GUID_WICPixelFormat32bppRGBA;
	else if (wicFormatGUID == GUID_WICPixelFormat32bppPBGRA) return GUID_WICPixelFormat32bppRGBA;
	else if (wicFormatGUID == GUID_WICPixelFormat32bppPRGBA) return GUID_WICPixelFormat32bppRGBA;
	else if (wicFormatGUID == GUID_WICPixelFormat48bppRGB) return GUID_WICPixelFormat64bppRGBA;
	else if (wicFormatGUID == GUID_WICPixelFormat48bppBGR) return GUID_WICPixelFormat64bppRGBA;
	else if (wicFormatGUID == GUID_WICPixelFormat64bppBGRA) return GUID_WICPixelFormat64bppRGBA;
	else if (wicFormatGUID == GUID_WICPixelFormat64bppPRGBA) return GUID_WICPixelFormat64bppRGBA;
	else if (wicFormatGUID == GUID_WICPixelFormat64bppPBGRA) return GUID_WICPixelFormat64bppRGBA;
	else if (wicFormatGUID == GUID_WICPixelFormat48bppRGBFixedPoint) return GUID_WICPixelFormat64bppRGBAHalf;
	else if (wicFormatGUID == GUID_WICPixelFormat48bppBGRFixedPoint) return GUID_WICPixelFormat64bppRGBAHalf;
	else if (wicFormatGUID == GUID_WICPixelFormat64bppRGBAFixedPoint) return GUID_WICPixelFormat64bppRGBAHalf;
	else if (wicFormatGUID == GUID_WICPixelFormat64bppBGRAFixedPoint) return GUID_WICPixelFormat64bppRGBAHalf;
	else if (wicFormatGUID == GUID_WICPixelFormat64bppRGBFixedPoint) return GUID_WICPixelFormat64bppRGBAHalf;
	else if (wicFormatGUID == GUID_WICPixelFormat64bppRGBHalf) return GUID_WICPixelFormat64bppRGBAHalf;
	else if (wicFormatGUID == GUID_WICPixelFormat48bppRGBHalf) return GUID_WICPixelFormat64bppRGBAHalf;
	else if (wicFormatGUID == GUID_WICPixelFormat128bppPRGBAFloat) return GUID_WICPixelFormat128bppRGBAFloat;
	else if (wicFormatGUID == GUID_WICPixelFormat128bppRGBFloat) return GUID_WICPixelFormat128bppRGBAFloat;
	else if (wicFormatGUID == GUID_WICPixelFormat128bppRGBAFixedPoint) return GUID_WICPixelFormat128bppRGBAFloat;
	else if (wicFormatGUID == GUID_WICPixelFormat128bppRGBFixedPoint) return GUID_WICPixelFormat128bppRGBAFloat;
	else if (wicFormatGUID == GUID_WICPixelFormat32bppRGBE) return GUID_WICPixelFormat128bppRGBAFloat;
	else if (wicFormatGUID == GUID_WICPixelFormat32bppCMYK) return GUID_WICPixelFormat32bppRGBA;
	else if (wicFormatGUID == GUID_WICPixelFormat64bppCMYK) return GUID_WICPixelFormat64bppRGBA;
	else if (wicFormatGUID == GUID_WICPixelFormat40bppCMYKAlpha) return GUID_WICPixelFormat64bppRGBA;
	else if (wicFormatGUID == GUID_WICPixelFormat80bppCMYKAlpha) return GUID_WICPixelFormat64bppRGBA;

#if (_WIN32_WINNT >= _WIN32_WINNT_WIN8) || defined(_WIN7_PLATFORM_UPDATE)
	else if (wicFormatGUID == GUID_WICPixelFormat32bppRGB) return GUID_WICPixelFormat32bppRGBA;
	else if (wicFormatGUID == GUID_WICPixelFormat64bppRGB) return GUID_WICPixelFormat64bppRGBA;
	else if (wicFormatGUID == GUID_WICPixelFormat64bppPRGBAHalf) return GUID_WICPixelFormat64bppRGBAHalf;
#endif

	else return GUID_WICPixelFormatDontCare;
}

// get the number of bits per pixel for a dxgi format
int Texture::GetDXGIFormatBitsPerPixel(DXGI_FORMAT& dxgiFormat)
{
	if (dxgiFormat == DXGI_FORMAT_R32G32B32A32_FLOAT) return 128;
	else if (dxgiFormat == DXGI_FORMAT_R16G16B16A16_FLOAT) return 64;
	else if (dxgiFormat == DXGI_FORMAT_R16G16B16A16_UNORM) return 64;
	else if (dxgiFormat == DXGI_FORMAT_R8G8B8A8_UNORM) return 32;
	else if (dxgiFormat == DXGI_FORMAT_B8G8R8A8_UNORM) return 32;
	else if (dxgiFormat == DXGI_FORMAT_B8G8R8X8_UNORM) return 32;
	else if (dxgiFormat == DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM) return 32;

	else if (dxgiFormat == DXGI_FORMAT_R10G10B10A2_UNORM) return 32;
	else if (dxgiFormat == DXGI_FORMAT_B5G5R5A1_UNORM) return 16;
	else if (dxgiFormat == DXGI_FORMAT_B5G6R5_UNORM) return 16;
	else if (dxgiFormat == DXGI_FORMAT_R32_FLOAT) return 32;
	else if (dxgiFormat == DXGI_FORMAT_R16_FLOAT) return 16;
	else if (dxgiFormat == DXGI_FORMAT_R16_UNORM) return 16;
	else if (dxgiFormat == DXGI_FORMAT_R8_UNORM) return 8;
	else if (dxgiFormat == DXGI_FORMAT_A8_UNORM) return 8;

	return -1;
}

///////////////////////////////////////////////////////////////
/////////////// RENDER TEXTURE STARTS FROM HERE ///////////////
///////////////////////////////////////////////////////////////

RenderTexture::RenderTexture(int _width, int _height, DXGI_FORMAT format, bool _supportDepth)
	: RenderTexture(_width, _height, L"no name", format, _supportDepth)
{
}

RenderTexture::RenderTexture(int _width, int _height, const wstring& _fileName, DXGI_FORMAT format, bool _supportDepth)
	: width(_width),
	height(_height),
	supportDepth(_supportDepth),
	depthStencilBuffer(nullptr),
	resourceState(D3D12_RESOURCE_STATE_RENDER_TARGET),
	Texture(_fileName.c_str())
{
	textureDesc = {};
	textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	textureDesc.Alignment = 0; // may be 0, 4KB, 64KB, or 4MB. 0 will let runtime decide between 64KB and 4MB (4MB for multi-sampled textures)
	textureDesc.Width = width; // width of the texture
	textureDesc.Height = height; // height of the texture
	textureDesc.DepthOrArraySize = 1; // if 3d image, depth of 3d image. Otherwise an array of 1D or 2D textures (we only have one image, so we set 1)
	textureDesc.MipLevels = 1; // Number of mipmaps. We are not generating mipmaps for this texture, so we have only one level
	textureDesc.Format = format; // This is the dxgi format of the image (format of the pixels)
	textureDesc.SampleDesc.Count = 1; // This is the number of samples per pixel, we just want 1 sample
	textureDesc.SampleDesc.Quality = 0; // The quality level of the samples. Higher is better quality, but worse performance
	textureDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN; // The arrangement of the pixels. Setting to unknown lets the driver choose the most efficient one
	textureDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

	depthStencilBufferDesc = {};
	depthStencilBufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depthStencilBufferDesc.Alignment = 0; // may be 0, 4KB, 64KB, or 4MB. 0 will let runtime decide between 64KB and 4MB (4MB for multi-sampled textures)
	depthStencilBufferDesc.Width = width; // width of the texture
	depthStencilBufferDesc.Height = height; // height of the texture
	depthStencilBufferDesc.DepthOrArraySize = 1; // if 3d image, depth of 3d image. Otherwise an array of 1D or 2D textures (we only have one image, so we set 1)
	depthStencilBufferDesc.MipLevels = 0; // Number of mipmaps. We are not generating mipmaps for this texture, so we have only one level
	depthStencilBufferDesc.Format = DXGI_FORMAT_D32_FLOAT; // This is the dxgi format of the image (format of the pixels)
	depthStencilBufferDesc.SampleDesc.Count = 1; // This is the number of samples per pixel, we just want 1 sample
	depthStencilBufferDesc.SampleDesc.Quality = 0; // The quality level of the samples. Higher is better quality, but worse performance
	depthStencilBufferDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN; // The arrangement of the pixels. Setting to unknown lets the driver choose the most efficient one
	depthStencilBufferDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
}

RenderTexture::~RenderTexture()
{
	ReleaseBuffers();
}

void RenderTexture::ReleaseBuffers()
{
	SAFE_RELEASE(depthStencilBuffer);
	SAFE_RELEASE(textureBuffer);
}

bool RenderTexture::CreateTextureBuffer(ID3D12Device* device)
{
	HRESULT hr;

	D3D12_CLEAR_VALUE colorOptimizedClearValue = {};
	colorOptimizedClearValue.Format = textureDesc.Format;
	colorOptimizedClearValue.Color[0] = 0.0f;
	colorOptimizedClearValue.Color[1] = 0.0f;
	colorOptimizedClearValue.Color[2] = 0.0f;
	colorOptimizedClearValue.Color[3] = 0.0f;

	hr = device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&textureDesc,
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		&colorOptimizedClearValue,
		IID_PPV_ARGS(&textureBuffer));

	if (FAILED(hr))
	{
		return false;
	}

	textureBuffer->SetName(fileName.c_str());

	////////////////////////////////////////////////////////////////////
	if (supportDepth)
	{
		D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
		depthOptimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
		depthOptimizedClearValue.DepthStencil.Depth = 1.0f;
		depthOptimizedClearValue.DepthStencil.Stencil = 0;

		hr = device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, width, height, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL),//depthStencilBufferDesc,//
			D3D12_RESOURCE_STATE_DEPTH_WRITE,
			&depthOptimizedClearValue,
			IID_PPV_ARGS(&depthStencilBuffer)
		);

		if (FAILED(hr))
		{
			return false;
		}

		depthStencilBuffer->SetName(fileName.c_str());
	}

	return true;
}

bool RenderTexture::UpdateTextureBuffer(ID3D12Device* device)
{
	HRESULT hr;

	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = textureDesc.Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;

	//Render Target View Desc
	rtvDesc.Format = textureDesc.Format;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Texture2D.MipSlice = 0;
	
	//Stencil Depth View Desc
	//if you want to bind null dsv desciptor, you need to create the dsv
	//if you want to create the dsv, you need a initialized dsvDesc to avoid the debug layer throwing out error
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
	dsvDesc.Texture2D.MipSlice = 0;

	return true;
}

void RenderTexture::UpdateViewport()
{
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = width;
	viewport.Height = height;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
}

void RenderTexture::UpdateScissorRect()
{
	scissorRect.left = 0;
	scissorRect.top = 0;
	scissorRect.right = width;
	scissorRect.bottom = height;
}

void RenderTexture::SetRtvHandle(CD3DX12_CPU_DESCRIPTOR_HANDLE _rtvHandle)
{
	rtvHandle = _rtvHandle;
}

void RenderTexture::SetDsvHandle(CD3DX12_CPU_DESCRIPTOR_HANDLE _dsvHandle)
{
	dsvHandle = _dsvHandle;
}

void RenderTexture::SetResourceState(D3D12_RESOURCE_STATES _resourceState)
{
	resourceState = _resourceState;
}

bool RenderTexture::SupportDepth()
{
	return supportDepth;
}

ID3D12Resource* RenderTexture::GetDepthStencilBuffer()
{
	// make sure to create null descriptor for render textures which does not support depth
	return supportDepth ? depthStencilBuffer : nullptr;
}

D3D12_VIEWPORT RenderTexture::GetViewport()
{
	return viewport;
}

D3D12_RECT RenderTexture::GetScissorRect()
{
	return scissorRect;
}

CD3DX12_CPU_DESCRIPTOR_HANDLE RenderTexture::GetRtvHandle()
{
	return rtvHandle;
}

CD3DX12_CPU_DESCRIPTOR_HANDLE RenderTexture::GetDsvHandle()
{
	return dsvHandle;
}

D3D12_RENDER_TARGET_VIEW_DESC RenderTexture::GetRtvDesc()
{
	return rtvDesc;
}

D3D12_DEPTH_STENCIL_VIEW_DESC RenderTexture::GetDsvDesc()
{
	return dsvDesc;
}

D3D12_RESOURCE_STATES RenderTexture::GetResourceState()
{
	return resourceState;
}

CD3DX12_RESOURCE_BARRIER RenderTexture::TransitionToResourceState(D3D12_RESOURCE_STATES _resourceState)
{
	CD3DX12_RESOURCE_BARRIER result;
	result = CD3DX12_RESOURCE_BARRIER::Transition(textureBuffer, resourceState, _resourceState, 0);
	resourceState = _resourceState;
	return result;
}

bool RenderTexture::InitTexture(ID3D12Device* device)
{
	UpdateViewport();
	UpdateScissorRect();

	if (!CreateTextureBuffer(device))
	{
		printf("CreateTextureBuffer failed\n");
		return false;
	}

	if (!UpdateTextureBuffer(device))
	{
		printf("UpdateTextureBuffer failed\n");
		return false;
	}

	return true;
}
