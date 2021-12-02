#pragma once
class ComputeShader {
public:
	ComputeShader(ID3D11Device* device, const WCHAR* shaderPathWithoutExt);
	~ComputeShader();

	ID3D11ComputeShader* GetComputeShader() { return m_ComputeShader.Get(); }

private:
	Microsoft::WRL::ComPtr<ID3D11ComputeShader> m_ComputeShader;

	HRESULT CompileShaderFromFile(const WCHAR* fileName, LPCSTR entryPoint, LPCSTR shaderModel, ID3DBlob** blobOut);
};