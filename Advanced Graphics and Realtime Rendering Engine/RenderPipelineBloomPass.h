#pragma once
#include "RenderPipelineStage.h"
#include "RenderPipelineGeometryPass.h"
#include "ComputeShader.h"

class RenderPipelineBloomPass : public RenderPipelineStage {
private:
	RenderPipelineGeometryPass*				m_geometryPass;

	std::shared_ptr<ComputeShader>			m_bloomComputeShader;
	Microsoft::WRL::ComPtr<ID3D11Buffer>	m_bloomConstantBuffer;

public:
	RenderPipelineBloomPass(Microsoft::WRL::ComPtr<ID3D11Device> _device,
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> _context,
		DirectX::XMINT2 renderResolution,
		RenderPipelineGeometryPass* geometryPass);
	~RenderPipelineBloomPass() = default;

	void Initialise() override;
	void Render() override;
};