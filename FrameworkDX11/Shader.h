#pragma once
#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <memory>

class Shader {
public:
	Shader(ID3D11Device* device, const WCHAR* vertexShaderPath, const WCHAR* pixelShaderPath, D3D11_INPUT_ELEMENT_DESC* vertexLayout, UINT numElements);
	~Shader();

	ID3D11VertexShader* GetVertexShader() { return m_VertexShader; }
	ID3D11PixelShader* GetPixelShader() { return m_PixelShader; }
	ID3D11InputLayout* GetVertexLayout() { return m_VertexLayout; }

private:
	ID3D11VertexShader* m_VertexShader;
	ID3D11PixelShader* m_PixelShader;
	ID3D11InputLayout* m_VertexLayout;

	HRESULT CompileShaderFromFile(const WCHAR* fileName, LPCSTR entryPoint, LPCSTR shaderModel, ID3DBlob** blobOut);

};
