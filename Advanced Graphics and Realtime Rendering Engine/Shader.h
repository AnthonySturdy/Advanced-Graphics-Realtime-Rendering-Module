#pragma once

class Shader {
public:
	Shader(ID3D11Device* device, const WCHAR* vertexShaderPathWithoutExt, const WCHAR* pixelShaderPathWithoutExt, D3D11_INPUT_ELEMENT_DESC* vertexLayout, UINT numElements);
	~Shader();

	Microsoft::WRL::ComPtr<ID3D11VertexShader> GetVertexShader() { return m_VertexShader; }
	Microsoft::WRL::ComPtr<ID3D11PixelShader> GetPixelShader() { return m_PixelShader; }
	Microsoft::WRL::ComPtr<ID3D11InputLayout> GetVertexLayout() { return m_VertexLayout; }

private:
	Microsoft::WRL::ComPtr<ID3D11VertexShader> m_VertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> m_PixelShader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> m_VertexLayout;

	HRESULT CompileShaderFromFile(const WCHAR* fileName, LPCSTR entryPoint, LPCSTR shaderModel, ID3DBlob** blobOut);
};