#pragma once
#include "GameObject.h"
#include "RenderPipelineStage.h"
#include "Structures.h"

class RenderPipelineShadowPass : public RenderPipelineStage {
private:
	const std::vector<std::shared_ptr<GameObject>>&		m_gameObjects;

	Microsoft::WRL::ComPtr<ID3D11DepthStencilView>      m_depthStencilView;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>	m_depthShaderResourceView;
	Microsoft::WRL::ComPtr<ID3D11SamplerState>			m_samplerState;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState>		m_renderState;
	Microsoft::WRL::ComPtr<ID3D11Buffer>				m_constantBuffer;

public:
	RenderPipelineShadowPass(Microsoft::WRL::ComPtr<ID3D11Device> _device,
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> _context,
		DirectX::XMINT2 renderResolution,
		const std::vector<std::shared_ptr<GameObject>>& gameObjects);
	~RenderPipelineShadowPass() = default;

	void Initialise() override;
	void Render() override;

	const Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>& GetSrv() const { return m_depthShaderResourceView; }
};