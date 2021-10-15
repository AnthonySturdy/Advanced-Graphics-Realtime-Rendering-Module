#pragma once
#include "Structures.h"
#include "Shader.h"

class GameObject {
public:
	GameObject();
	~GameObject();

	HRESULT InitMesh(ID3D11Device* device, ID3D11DeviceContext* context);
	void InitShader(ID3D11Device* device, const WCHAR* vertexShaderPath, const WCHAR* pixelShaderPath, D3D11_INPUT_ELEMENT_DESC* vertexLayout, UINT numElements);

	void Update(float t, ID3D11DeviceContext* context);
	void Render(ID3D11DeviceContext* context);

	DirectX::XMFLOAT4X4* GetTransform() { return &m_world; }
	ID3D11Buffer* GetMaterialConstantBuffer() { return m_materialConstantBuffer.Get(); }
	std::shared_ptr<Shader> GetShader() { return m_shader; }

	void SetPosition(DirectX::XMFLOAT3 pos) { m_position = pos; }

private:
	DirectX::XMFLOAT3 m_position;
	DirectX::XMFLOAT4X4 m_world;

	Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_indexBuffer;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_textureResourceView;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> m_samplerLinear;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_materialConstantBuffer;
	
	MaterialPropertiesConstantBuffer m_material;

	std::shared_ptr<Shader> m_shader;
};
