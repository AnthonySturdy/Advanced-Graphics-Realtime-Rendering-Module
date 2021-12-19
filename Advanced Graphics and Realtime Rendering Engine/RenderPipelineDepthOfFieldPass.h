#pragma once
#include "RenderPipelineStage.h"
#include "Camera.h"
#include "ComputeShader.h"
#include "Structures.h"

class RenderPipelineDepthOfFieldPass : public RenderPipelineStage {
private:
	const std::shared_ptr<Camera>&			m_camera;

	std::shared_ptr<ComputeShader>			m_depthOfFieldComputeShader;
	Microsoft::WRL::ComPtr<ID3D11Buffer>	m_depthOfFieldConstantBuffer;

public:
	RenderPipelineDepthOfFieldPass(Microsoft::WRL::ComPtr<ID3D11Device> _device,
									Microsoft::WRL::ComPtr<ID3D11DeviceContext> _context,
									const std::shared_ptr<Camera>& camera,
									DirectX::XMINT2 renderResolution);
	~RenderPipelineDepthOfFieldPass() = default;

	void Initialise() override;
	void Render() override;


};
