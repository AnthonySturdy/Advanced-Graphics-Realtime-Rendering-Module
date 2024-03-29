#pragma once
#include "GameObject.h"

class GameObject_Quad : public GameObject {
public:
	GameObject_Quad(DirectX::XMFLOAT3 initPos, DirectX::XMFLOAT3 initRot, DirectX::XMFLOAT3 initScale);
	~GameObject_Quad();

	HRESULT InitMesh(ID3D11Device* device, ID3D11DeviceContext* context) override;
	void Render(ID3D11DeviceContext* context) override;
	void RenderGUIControls(ID3D11Device* device, Camera* camera) override;

private:
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_textureResourceView;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_normalResourceView;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_parallaxResourceView;
};