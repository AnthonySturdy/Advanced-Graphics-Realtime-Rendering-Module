#include "pch.h"
#include "RenderPipelineImageFilterPass.h"

RenderPipelineImageFilterPass::RenderPipelineImageFilterPass(Microsoft::WRL::ComPtr<ID3D11Device> _device,
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> _context, DirectX::XMINT2 renderResolution,
	RenderPipelineGeometryPass* geometryPass)
		: RenderPipelineStage(_device, _context, renderResolution), m_geometryPass(geometryPass) { }

void RenderPipelineImageFilterPass::Initialise() {
    m_imageFilterComputeShader = std::make_unique<ComputeShader>(device.Get(), L"ImageFilterShader");

    // Create the constant buffer
    D3D11_BUFFER_DESC bd = {};
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(ImageFilterConstantBuffer);
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bd.CPUAccessFlags = 0;
    HRESULT hr = device->CreateBuffer(&bd, nullptr, m_imageFilterConstantBuffer.GetAddressOf());
    if (FAILED(hr))
        return;
}

void RenderPipelineImageFilterPass::Render() {
    ID3D11UnorderedAccessView* nullUav = nullptr;
    ID3D11ShaderResourceView* nullSrv = nullptr;

    static ImageFilterConstantBuffer ifcb = { 0, 0.0f, 0.0f, 0.0f, static_cast<float>(m_resolution.x), static_cast<float>(m_resolution.y), 0.0f, 0.0f };
    if (ifcb.filterIntensity > 0) {
        ifcb.time = static_cast<float>(rand());
        ifcb.resolution[0] = static_cast<float>(m_resolution.x);
        ifcb.resolution[1] = static_cast<float>(m_resolution.y);

        context->UpdateSubresource(m_imageFilterConstantBuffer.Get(), 0, nullptr, &ifcb, 0, 0);
        context->CSSetConstantBuffers(0, 1, m_imageFilterConstantBuffer.GetAddressOf());

        // Pass textures to compute shader
        context->CSSetUnorderedAccessViews(0, 1, GetUnorderedAccessView().GetAddressOf(), nullptr);

        // Dispatch horizontal blur pass
        context->CSSetShader(m_imageFilterComputeShader->GetComputeShader(), nullptr, 0);
        context->Dispatch(m_resolution.x / 8, m_resolution.y / 8, 1);

        // Unbind textures after Dispatch
        context->CSSetUnorderedAccessViews(0, 1, &nullUav, nullptr);
        context->CSSetShaderResources(0, 1, &nullSrv);
        context->CSSetShaderResources(1, 1, &nullSrv);
    }

    // Render GUI
    ImGui::Begin("Image Filters");
        const char* items[] = { "Invert", "Greyscale", "Film Grain", "Vignette" };
        static int selection = ifcb.filterType;
        ImGui::Combo("Filter Type", &selection, items, 4);
        ifcb.filterType = selection;

        ImGui::DragFloat("Intensity", &ifcb.filterIntensity, 0.01f, 0.0f, 1.0f);
	ImGui::End();
}