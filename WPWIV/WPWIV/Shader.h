#pragma once

#include <D3Dcompiler.h>
#include "GlobalInclude.h"

class Shader
{
public:
	Shader();
	~Shader();

	bool CreateVertexShaderFromFile(const wstring& fileName);
	bool CreateHullShaderFromFile(const wstring& fileName);
	bool CreateDomainShaderFromFile(const wstring& fileName);
	bool CreatePixelShaderFromFile(const wstring& fileName);
	D3D12_SHADER_BYTECODE GetShaderByteCode();

private:

	D3D12_SHADER_BYTECODE shaderBytecode;
};

