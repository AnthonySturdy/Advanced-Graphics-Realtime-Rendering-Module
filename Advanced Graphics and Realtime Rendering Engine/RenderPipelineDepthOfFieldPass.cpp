#include "pch.h"
#include "RenderPipelineDepthOfFieldPass.h"


RenderPipelineDepthOfFieldPass::RenderPipelineDepthOfFieldPass(Microsoft::WRL::ComPtr<ID3D11Device> _device,
                                                               Microsoft::WRL::ComPtr<ID3D11DeviceContext> _context, 
                                                               const std::shared_ptr<Camera>& camera,
                                                               DirectX::XMINT2 renderResolution)
	: RenderPipelineStage(_device, _context, renderResolution), m_camera(camera) { }

void RenderPipelineDepthOfFieldPass::Initialise() {
	m_depthOfFieldComputeShader = std::make_unique<ComputeShader>(device.Get(), L"DepthOfFieldShader");

	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(DepthOfFieldConstantBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	HRESULT hr = device->CreateBuffer(&bd, nullptr, m_depthOfFieldConstantBuffer.GetAddressOf());
	if (FAILED(hr))
		return;
}

void RenderPipelineDepthOfFieldPass::Render() {
	
}
