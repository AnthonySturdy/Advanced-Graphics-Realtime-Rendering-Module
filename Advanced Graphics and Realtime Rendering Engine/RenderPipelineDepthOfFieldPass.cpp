#include "pch.h"
#include "RenderPipelineDepthOfFieldPass.h"

RenderPipelineDepthOfFieldPass::RenderPipelineDepthOfFieldPass(Microsoft::WRL::ComPtr<ID3D11Device> _device,
                                                               Microsoft::WRL::ComPtr<ID3D11DeviceContext> _context, 
															   DirectX::XMINT2 renderResolution, 
															   const std::shared_ptr<Camera>& camera,
															   RenderPipelineGeometryPass* geometryPass)
	: RenderPipelineStage(_device, _context, renderResolution), m_camera(camera), m_geometryPass(geometryPass) { }

void RenderPipelineDepthOfFieldPass::Initialise() {
    ResetUnorderedAccessView();

	m_depthOfFieldComputeShader = std::make_unique<ComputeShader>(device.Get(), L"DepthOfFieldShader");

    // Create constant buffer
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
    ID3D11UnorderedAccessView* nullUav = nullptr;
    ID3D11ShaderResourceView* nullSrv = nullptr;

    // Update GPU with Gaussian blur cbuffer
    static DepthOfFieldConstantBuffer dofcb = { 25.0f, 8.0f, 25.0f, m_camera->GetFarPlane(), m_resolution.x, m_resolution.y, 3.5f };
    dofcb.farPlaneDepth = m_camera->GetFarPlane();
    dofcb.resolution[0] = m_resolution.x;
    dofcb.resolution[1] = m_resolution.y;
    context->UpdateSubresource(m_depthOfFieldConstantBuffer.Get(), 0, nullptr, &dofcb, 0, 0);
    context->CSSetConstantBuffers(0, 1, m_depthOfFieldConstantBuffer.GetAddressOf());

    // Pass textures to compute shader
    context->CSSetShaderResources(0, 1, m_geometryPass->GetSrv().GetAddressOf());
    context->CSSetUnorderedAccessViews(0, 1, GetUnorderedAccessView().GetAddressOf(), nullptr);

    // Dispatch gaussian blur pass
    context->CSSetShader(m_depthOfFieldComputeShader->GetComputeShader(), nullptr, 0);
    context->Dispatch(m_resolution.x / 8, m_resolution.y / 8, 1);

    // Unbind textures after Dispatch
    context->CSSetUnorderedAccessViews(0, 1, &nullUav, nullptr);
    context->CSSetShaderResources(0, 1, &nullSrv);

    // Render GUI
    ImGui::Begin("Depth of Field");
	    ImGui::DragFloat("Size", &dofcb.size, 0.1f, 0.0f, 50.0f);
	    ImGui::DragFloat("Quality", &dofcb.quality, 0.1f, 0.0f, 50.0f);
	    ImGui::DragFloat("Directions", &dofcb.directions, 0.1f, 0.0f, 50.0f);
	    ImGui::DragFloat("Depth", &dofcb.depth, 0.1f, 0.0f, 50.0f);
    ImGui::End();
}
