#include "pch.h"
#include "RenderPipelineStage.h"

RenderPipelineStage::RenderPipelineStage(Microsoft::WRL::ComPtr<ID3D11Device> _device,
                                         Microsoft::WRL::ComPtr<ID3D11DeviceContext> _context,
                                         DirectX::XMINT2 resolution)
	: device(_device), context(_context), m_resolution(resolution) {}

void RenderPipelineStage::TryInitialiseUAV() {
	if (m_postProcUnorderedAccessView != nullptr)
		return;

    // Create post processing Unordered Access Resource (UAV)
    Microsoft::WRL::ComPtr<ID3D11Texture2D> postProcUAVTex;
    D3D11_TEXTURE2D_DESC postProcTexDesc = {};
    postProcTexDesc.Width = m_resolution.x;
    postProcTexDesc.Height = m_resolution.x;
    postProcTexDesc.MipLevels = 1;
    postProcTexDesc.ArraySize = 1;
    postProcTexDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    postProcTexDesc.SampleDesc.Count = 1;
    postProcTexDesc.SampleDesc.Quality = 0;
    postProcTexDesc.Usage = D3D11_USAGE_DEFAULT;
    postProcTexDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
    postProcTexDesc.CPUAccessFlags = 0;
    postProcTexDesc.MiscFlags = 0;
    DX::ThrowIfFailed(device->CreateTexture2D(&postProcTexDesc, nullptr, postProcUAVTex.GetAddressOf()));

    D3D11_UNORDERED_ACCESS_VIEW_DESC postProcUAVDesc;
    postProcUAVDesc.Format = DXGI_FORMAT_UNKNOWN;
    postProcUAVDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
    postProcUAVDesc.Texture2D.MipSlice = 0;

    DX::ThrowIfFailed(device->CreateUnorderedAccessView(postProcUAVTex.Get(), &postProcUAVDesc, m_postProcUnorderedAccessView.ReleaseAndGetAddressOf()));

    // Create SRV and render to ImGui window
	Microsoft::WRL::ComPtr<ID3D11Resource> renderResource;
    m_postProcUnorderedAccessView->GetResource(renderResource.ReleaseAndGetAddressOf());
    device->CreateShaderResourceView(renderResource.Get(), nullptr, m_unorderedAccessSRV.ReleaseAndGetAddressOf());
}