#include "Shader.h"

Shader::Shader(const ShaderType& _type, const wstring& _fileName) :
	type(_type),
	fileName(_fileName)
{
}

Shader::~Shader()
{
	ReleaseBuffer();
}

bool Shader::CreateShader()
{
	if (type == ShaderType::VertexShader)
	{
		return CreateVertexShaderFromFile(fileName);
	}
	else if (type == ShaderType::HullShader)
	{
		return CreateHullShaderFromFile(fileName);
	}
	else if (type == ShaderType::DomainShader)
	{
		return CreateDomainShaderFromFile(fileName);
	}
	else if (type == ShaderType::GeometryShader)
	{
		return CreateGeometryShaderFromFile(fileName);
	}
	else if (type == ShaderType::PixelShader)
	{
		return CreatePixelShaderFromFile(fileName);
	}
	return false;
}

bool Shader::CreateVertexShaderFromFile(const wstring& _fileName)
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
	
	hr = D3DCompileFromFile(_fileName.c_str(),
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"main",
		"vs_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0,
		&shader,
		&errorBuff);
	if (FAILED(hr))
	{
		CheckError(hr, errorBuff);
		return false;
	}

	SAFE_RELEASE(errorBuff);
	// fill out a shader bytecode structure, which is basically just a pointer
	// to the shader bytecode and the size of the shader bytecode
	shaderBytecode.BytecodeLength = shader->GetBufferSize();
	shaderBytecode.pShaderBytecode = shader->GetBufferPointer();
	
	return true;
}

bool Shader::CreateHullShaderFromFile(const wstring& _fileName)
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
	hr = D3DCompileFromFile(_fileName.c_str(),
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"main",
		"hs_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0,
		&shader,
		&errorBuff);
	if (FAILED(hr))
	{
		CheckError(hr, errorBuff);
		return false;
	}

	SAFE_RELEASE(errorBuff);
	// fill out a shader bytecode structure, which is basically just a pointer
	// to the shader bytecode and the size of the shader bytecode
	shaderBytecode.BytecodeLength = shader->GetBufferSize();
	shaderBytecode.pShaderBytecode = shader->GetBufferPointer();

	return true;
}

bool Shader::CreateDomainShaderFromFile(const wstring& _fileName)
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
	hr = D3DCompileFromFile(_fileName.c_str(),
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"main",
		"ds_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0,
		&shader,
		&errorBuff);
	if (FAILED(hr))
	{
		CheckError(hr, errorBuff);
		return false;
	}

	SAFE_RELEASE(errorBuff);
	// fill out a shader bytecode structure, which is basically just a pointer
	// to the shader bytecode and the size of the shader bytecode
	shaderBytecode.BytecodeLength = shader->GetBufferSize();
	shaderBytecode.pShaderBytecode = shader->GetBufferPointer();

	return true;
}

bool Shader::CreateGeometryShaderFromFile(const wstring& _fileName)
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
	hr = D3DCompileFromFile(_fileName.c_str(),
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"main",
		"gs_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0,
		&shader,
		&errorBuff);
	if (FAILED(hr))
	{
		CheckError(hr, errorBuff);
		return false;
	}

	SAFE_RELEASE(errorBuff);
	// fill out a shader bytecode structure, which is basically just a pointer
	// to the shader bytecode and the size of the shader bytecode
	shaderBytecode.BytecodeLength = shader->GetBufferSize();
	shaderBytecode.pShaderBytecode = shader->GetBufferPointer();

	return true;
}

bool Shader::CreatePixelShaderFromFile(const wstring& _fileName)
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
	
	hr = D3DCompileFromFile(_fileName.c_str(),
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"main",
		"ps_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0,
		&shader,
		&errorBuff);
	if (FAILED(hr))
	{
		CheckError(hr, errorBuff);
		return false;
	}

	SAFE_RELEASE(errorBuff);
	// fill out shader bytecode structure for pixel shader
	shaderBytecode.BytecodeLength = shader->GetBufferSize();
	shaderBytecode.pShaderBytecode = shader->GetBufferPointer();

	return true;
}


D3D12_SHADER_BYTECODE Shader::GetShaderByteCode()
{
	return shaderBytecode;
}

void Shader::ReleaseBuffer()
{
	SAFE_RELEASE(shader);
}
