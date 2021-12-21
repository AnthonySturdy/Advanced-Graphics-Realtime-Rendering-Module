#include "pch.h"
#include "RenderPipelineBloomPass.h"

RenderPipelineBloomPass::RenderPipelineBloomPass(Microsoft::WRL::ComPtr<ID3D11Device> _device,
												Microsoft::WRL::ComPtr<ID3D11DeviceContext> _context, 
												DirectX::XMINT2 renderResolution,
												RenderPipelineGeometryPass* geometryPass)
		: RenderPipelineStage(_device, _context, renderResolution), m_geometryPass(geometryPass) {}

void RenderPipelineBloomPass::Initialise() {
    m_bloomComputeShader = std::make_unique<ComputeShader>(device.Get(), L"BloomShader");

    // Create constant buffer
	D3D11_BUFFER_DESC bd = {};
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(BloomConstantBuffer);
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bd.CPUAccessFlags = 0;
    HRESULT hr = device->CreateBuffer(&bd, nullptr, m_bloomConstantBuffer.GetAddressOf());
    if (FAILED(hr))
        return;
}

void RenderPipelineBloomPass::Render() {
    ID3D11UnorderedAccessView* nullUav = nullptr;
    ID3D11ShaderResourceView* nullSrv = nullptr;

    // Update GPU with Gaussian blur cbuffer
    static BloomConstantBuffer gbcb = { 25.0f, 8.0f, 45.0f, 0.0f };
    context->UpdateSubresource(m_bloomConstantBuffer.Get(), 0, nullptr, &gbcb, 0, 0);
    context->CSSetConstantBuffers(0, 1, m_bloomConstantBuffer.GetAddressOf());

    // Pass textures to compute shader
    context->CSSetShaderResources(0, 1, m_geometryPass->GetSrv().GetAddressOf());
    context->CSSetShaderResources(1, 1, m_geometryPass->GetHDRSrv().GetAddressOf());
    context->CSSetUnorderedAccessViews(0, 1, GetUnorderedAccessView().GetAddressOf(), nullptr);

    // Dispatch horizontal blur pass
    context->CSSetShader(m_bloomComputeShader->GetComputeShader(), nullptr, 0);
    context->Dispatch(m_resolution.x / 8, m_resolution.y / 8, 1);

    // Unbind textures after Dispatch
    context->CSSetUnorderedAccessViews(0, 1, &nullUav, nullptr);
    context->CSSetShaderResources(0, 1, &nullSrv);
    context->CSSetShaderResources(1, 1, &nullSrv);

    // Render GUI
    ImGui::Begin("Bloom");
        ImGui::DragFloat("Size", &gbcb.size, 0.1f, 0.0f, 50.0f);
		ImGui::DragFloat("Quality", &gbcb.quality, 0.1f, 0.0f, 50.0f);
	    ImGui::DragFloat("Directions", &gbcb.directions, 0.1f, 0.0f, 50.0f);
    ImGui::End();
}