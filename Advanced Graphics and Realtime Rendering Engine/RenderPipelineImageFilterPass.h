#pragma once
#include "RenderPipelineStage.h"
#include "RenderPipelineGeometryPass.h"
#include "ComputeShader.h"

class RenderPipelineImageFilterPass : public RenderPipelineStage {
private:
	RenderPipelineGeometryPass*				m_geometryPass;

	std::shared_ptr<ComputeShader>			m_imageFilterComputeShader;
	Microsoft::WRL::ComPtr<ID3D11Buffer>	m_imageFilterConstantBuffer;

public:
	RenderPipelineImageFilterPass(Microsoft::WRL::ComPtr<ID3D11Device> _device,
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> _context,
		DirectX::XMINT2 renderResolution,
		RenderPipelineGeometryPass* geometryPass);
	~RenderPipelineImageFilterPass() = default;

	void Initialise() override;
	void Render() override;
};