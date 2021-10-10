#pragma once

#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <directxcolors.h>
#include <DirectXCollision.h>
#include "DDSTextureLoader.h"
#include "resource.h"
#include <iostream>
#include "structures.h"
#include "Shader.h"


using namespace DirectX;

struct SimpleVertex
{
	XMFLOAT3 Pos;
	XMFLOAT3 Normal;
	XMFLOAT2 TexCoord;
};

class DrawableGameObject
{
public:
	DrawableGameObject();
	~DrawableGameObject();

	void Cleanup();

	HRESULT								InitMesh(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pContext);
	void								InitShader(ID3D11Device* device, const WCHAR* vertexShaderPath, const WCHAR* pixelShaderPath, D3D11_INPUT_ELEMENT_DESC* vertexLayout, UINT numElements);
	void								Update(float t, ID3D11DeviceContext* pContext);
	void								Render(ID3D11DeviceContext* pContext);
	ID3D11Buffer*						GetVertexBuffer() { return m_pVertexBuffer; }
	ID3D11Buffer*						GetIndexBuffer() { return m_pIndexBuffer; }
	ID3D11ShaderResourceView**			GetTextureResourceView() { return &m_pTextureResourceView; 	}
	XMFLOAT4X4*							GetTransform() { return &m_World; }
	ID3D11SamplerState**				GetTextureSamplerState() { return &m_pSamplerLinear; }
	ID3D11Buffer*						GetMaterialConstantBuffer() { return m_pMaterialConstantBuffer;}
	std::shared_ptr<Shader>								GetShader() { return m_Shader; }
	
	void								SetPosition(XMFLOAT3 position);

private:
	
	XMFLOAT4X4							m_World;

	ID3D11Buffer*						m_pVertexBuffer;
	ID3D11Buffer*						m_pIndexBuffer;
	ID3D11ShaderResourceView*			m_pTextureResourceView;
	ID3D11SamplerState *				m_pSamplerLinear;
	MaterialPropertiesConstantBuffer	m_material;
	ID3D11Buffer*						m_pMaterialConstantBuffer = nullptr;
	XMFLOAT3							m_position;
	std::shared_ptr<Shader>				m_Shader;
};

