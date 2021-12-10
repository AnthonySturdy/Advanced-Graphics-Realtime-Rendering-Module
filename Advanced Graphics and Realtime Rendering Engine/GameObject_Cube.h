#pragma once
#include "GameObject.h"

class GameObject_Cube : public GameObject {
public:
	GameObject_Cube(DirectX::XMFLOAT3 initPos, DirectX::XMFLOAT3 initRot, DirectX::XMFLOAT3 initScale);
	~GameObject_Cube();

	HRESULT InitMesh(ID3D11Device* device, ID3D11DeviceContext* context) override;
	void Render(ID3D11DeviceContext* context) override;
	void RenderGUIControls(ID3D11Device* device, Camera* camera) override;

private:
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_textureResourceView;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_normalResourceView;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_parallaxResourceView;
};