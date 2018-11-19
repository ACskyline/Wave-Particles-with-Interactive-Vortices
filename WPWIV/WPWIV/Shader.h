#pragma once

#include <D3Dcompiler.h>
#include "GlobalInclude.h"

class Shader
{
public:
	enum ShaderType { VertexShader, HullShader, DomainShader, PixelShader };

	Shader(const ShaderType& _type, const wstring& _fileName);
	~Shader();

	bool CreateShader();
	bool CreateVertexShaderFromFile(const wstring& _fileName);
	bool CreateHullShaderFromFile(const wstring& _fileName);
	bool CreateDomainShaderFromFile(const wstring& _fileName);
	bool CreatePixelShaderFromFile(const wstring& _fileName);
	D3D12_SHADER_BYTECODE GetShaderByteCode();

private:
	ShaderType type;
	wstring fileName;
	D3D12_SHADER_BYTECODE shaderBytecode;
};

