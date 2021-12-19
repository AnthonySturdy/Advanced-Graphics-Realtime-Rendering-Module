#pragma once
#include "GameObject.h"
#include "RenderPipelineStage.h"

class RenderPipelineGeometryPass : public RenderPipelineStage {
private:
	const std::vector<std::shared_ptr<GameObject>>&		m_gameObjects;
	const std::shared_ptr<Camera>&						m_camera;
																					
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView>		m_renderTargetView;		// Geometry render
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView>		m_renderTargetViewHDR;	// HDR extracted during geometry render
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView>      m_depthStencilView;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>    m_renderTargetSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>    m_hdrRenderTargetSRV;

	Microsoft::WRL::ComPtr<ID3D11Buffer>				m_constantBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer>				m_lightConstantBuffer;

	void SetupLightsForRender();

public:
	RenderPipelineGeometryPass(Microsoft::WRL::ComPtr<ID3D11Device> _device,
								Microsoft::WRL::ComPtr<ID3D11DeviceContext> _context,
								const std::vector<std::shared_ptr<GameObject>>& gameObjects,
								const std::shared_ptr<Camera>& camera,
								DirectX::XMINT2 renderResolution);
	~RenderPipelineGeometryPass() = default;

	void Initialise() override;
	void Render() override;

	const Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>& GetSrv() const { return m_renderTargetSRV; }
	const Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>& GetHDRSrv() const { return m_hdrRenderTargetSRV; }
};