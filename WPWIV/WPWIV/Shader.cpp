#include "Shader.h"

Shader::Shader()
{
}


Shader::~Shader()
{
}

bool Shader::CreateVertexShaderFromFile(const wstring& fileName)
{
	HRESULT hr;
	// create vertex and pixel shaders

	// when debugging, we can compile the shader files at runtime.
	// but for release versions, we can compile the hlsl shaders
	// with fxc.exe to create .cso files, which contain the shader
	// bytecode. We can load the .cso files at runtime to get the
	// shader bytecode, which of course is faster than compiling
	// them at runtime

	// compile vertex shader
	ID3DBlob* errorBuff; // a buffer holding the error data if any
	ID3DBlob* shader; // d3d blob for holding vertex shader bytecode
	hr = D3DCompileFromFile(fileName.c_str(),
		nullptr,
		nullptr,
		"main",
		"vs_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0,
		&shader,
		&errorBuff);
	if (FAILED(hr))
	{
		OutputDebugStringA((char*)errorBuff->GetBufferPointer());
		return false;
	}

	// fill out a shader bytecode structure, which is basically just a pointer
	// to the shader bytecode and the size of the shader bytecode
	shaderBytecode.BytecodeLength = shader->GetBufferSize();
	shaderBytecode.pShaderBytecode = shader->GetBufferPointer();

	return true;
}


bool Shader::CreatePixelShaderFromFile(const wstring& fileName)
{
	HRESULT hr;
	// create vertex and pixel shaders

	// when debugging, we can compile the shader files at runtime.
	// but for release versions, we can compile the hlsl shaders
	// with fxc.exe to create .cso files, which contain the shader
	// bytecode. We can load the .cso files at runtime to get the
	// shader bytecode, which of course is faster than compiling
	// them at runtime

	// compile pixel shader
	ID3DBlob* errorBuff; // a buffer holding the error data if any
	ID3DBlob* shader; // d3d blob for holding pixel shader bytecode
	
	hr = D3DCompileFromFile(L"PixelShader.hlsl",
		nullptr,
		nullptr,
		"main",
		"ps_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0,
		&shader,
		&errorBuff);
	if (FAILED(hr))
	{
		OutputDebugStringA((char*)errorBuff->GetBufferPointer());
		return false;
	}

	// fill out shader bytecode structure for pixel shader
	shaderBytecode.BytecodeLength = shader->GetBufferSize();
	shaderBytecode.pShaderBytecode = shader->GetBufferPointer();

	return true;
}


D3D12_SHADER_BYTECODE Shader::GetShaderByteCode()
{
	return shaderBytecode;
}