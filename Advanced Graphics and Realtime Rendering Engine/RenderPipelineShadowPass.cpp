#include "pch.h"
#include "RenderPipelineShadowPass.h"

RenderPipelineShadowPass::RenderPipelineShadowPass(Microsoft::WRL::ComPtr<ID3D11Device> _device, 
												    Microsoft::WRL::ComPtr<ID3D11DeviceContext> _context, 
												    DirectX::XMINT2 renderResolution, 
												    const std::vector<std::shared_ptr<GameObject>>& gameObjects)
		: RenderPipelineStage(_device, _context, renderResolution), m_gameObjects(gameObjects) {}

void RenderPipelineShadowPass::Initialise() {
    // Check for feature support
    D3D11_FEATURE_DATA_D3D9_SHADOW_SUPPORT isD3D9ShadowSupported;
    ZeroMemory(&isD3D9ShadowSupported, sizeof(isD3D9ShadowSupported));
    device->CheckFeatureSupport(
        D3D11_FEATURE_D3D9_SHADOW_SUPPORT,
        &isD3D9ShadowSupported,
        sizeof(D3D11_FEATURE_D3D9_SHADOW_SUPPORT)
    );

    if (!isD3D9ShadowSupported.SupportsDepthAsTextureWithLessEqualComparisonFilter)
        return;

    // Create shadow map texture
    Microsoft::WRL::ComPtr<ID3D11Texture2D> shadowMapTex;
    D3D11_TEXTURE2D_DESC shadowMapTexDesc;
    ZeroMemory(&shadowMapTexDesc, sizeof(D3D11_TEXTURE2D_DESC));
    shadowMapTexDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
    shadowMapTexDesc.MipLevels = 1;
    shadowMapTexDesc.ArraySize = 1;
    shadowMapTexDesc.SampleDesc.Count = 1;
    shadowMapTexDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_DEPTH_STENCIL;
    shadowMapTexDesc.Height = static_cast<UINT>(m_resolution.x);
    shadowMapTexDesc.Width = static_cast<UINT>(m_resolution.y);

    DX::ThrowIfFailed(
        device->CreateTexture2D(
        &shadowMapTexDesc,
        nullptr,
        shadowMapTex.ReleaseAndGetAddressOf()
    ));

    // Create resource views
    D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
    ZeroMemory(&depthStencilViewDesc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
    depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    depthStencilViewDesc.Texture2D.MipSlice = 0;

    D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
    ZeroMemory(&shaderResourceViewDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
    shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    shaderResourceViewDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
    shaderResourceViewDesc.Texture2D.MipLevels = 1;

    DX::ThrowIfFailed(
        device->CreateDepthStencilView(
        shadowMapTex.Get(),
        &depthStencilViewDesc,
        m_depthStencilView.ReleaseAndGetAddressOf()
    ));

    DX::ThrowIfFailed(
        device->CreateShaderResourceView(
        shadowMapTex.Get(),
        &shaderResourceViewDesc,
        m_depthShaderResourceView.ReleaseAndGetAddressOf()
    ));

    // Create sampler state
    D3D11_SAMPLER_DESC comparisonSamplerDesc;
    ZeroMemory(&comparisonSamplerDesc, sizeof(D3D11_SAMPLER_DESC));
    comparisonSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
    comparisonSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
    comparisonSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
    comparisonSamplerDesc.BorderColor[0] = 1.0f;
    comparisonSamplerDesc.BorderColor[1] = 1.0f;
    comparisonSamplerDesc.BorderColor[2] = 1.0f;
    comparisonSamplerDesc.BorderColor[3] = 1.0f;
    comparisonSamplerDesc.MinLOD = 0.f;
    comparisonSamplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
    comparisonSamplerDesc.MipLODBias = 0.f;
    comparisonSamplerDesc.MaxAnisotropy = 0;
    comparisonSamplerDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
    comparisonSamplerDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_POINT;

    DX::ThrowIfFailed(
        device->CreateSamplerState(
            &comparisonSamplerDesc,
            m_samplerState.ReleaseAndGetAddressOf()
        )
    );

    // Create render state
    D3D11_RASTERIZER_DESC shadowRenderStateDesc;
    ZeroMemory(&shadowRenderStateDesc, sizeof(D3D11_RASTERIZER_DESC));
    shadowRenderStateDesc.CullMode = D3D11_CULL_FRONT;
    shadowRenderStateDesc.FillMode = D3D11_FILL_SOLID;
    shadowRenderStateDesc.DepthClipEnable = true;

	DX::ThrowIfFailed(
        device->CreateRasterizerState(
            &shadowRenderStateDesc,
            m_renderState.ReleaseAndGetAddressOf()
        )
    );

    // Create constant buffer
    D3D11_BUFFER_DESC bd = {};
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(ShadowMappingConstantBuffer);
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bd.CPUAccessFlags = 0;

    DX::ThrowIfFailed(
        device->CreateBuffer(
            &bd, 
            nullptr, 
            m_constantBuffer.ReleaseAndGetAddressOf()
        )
    );
}

void RenderPipelineShadowPass::Render() {
    // Create viewport
    D3D11_VIEWPORT viewport;
    ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));
    viewport.Height = static_cast<FLOAT>(m_resolution.x);
    viewport.Width = static_cast<FLOAT>(m_resolution.y);
    viewport.MinDepth = 0.f;
    viewport.MaxDepth = 1.f;

    // Render scene
    context->ClearDepthStencilView(m_depthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    context->OMSetRenderTargets(                        // Only bind the ID3D11DepthStencilView for output
        0,
        nullptr,
        m_depthStencilView.Get()
    );
    context->RSSetState(m_renderState.Get());           // Set rendering state.
    context->RSSetViewports(1, &viewport);

    for(const auto& gameObject : m_gameObjects) {
        // Update constant buffer
        static ShadowMappingConstantBuffer smcb = {};

        static const DirectX::XMVECTORF32 eye = { 10.0f, 10.0f, 10.0f, 0.0f };
        static const DirectX::XMVECTORF32 at = { 0.0f, 0.0f, 0.0f, 0.0f };
        static const DirectX::XMVECTORF32 up = { 0.0f, 1.0f, 0.0f, 0.0f };
        smcb.mView = XMMatrixTranspose(XMMatrixLookAtRH(eye, at, up)); // Point light at (20, 15, 20), pointed at the origin
        smcb.mWorld = DirectX::XMMatrixTranspose(DirectX::XMLoadFloat4x4(gameObject->GetTransform()));
        smcb.mProjection = DirectX::XMMatrixOrthographicRH(
            DirectX::XM_PI * 1.8f,
            DirectX::XM_PI * 1.8f,
            1.f,
            12.f
        );

        context->UpdateSubresource(
            m_constantBuffer.Get(),
            0,
            NULL,
            &smcb,
            0,
            0
        );

        // Set Index and Vertex Buffers
        UINT stride = sizeof(SimpleVertex);
        UINT offset = 0;
        auto vertBuffer = gameObject->GetMesh()->GetVertexBuffer();
        context->IASetVertexBuffers(
            0,
            1,
            &vertBuffer,
            &stride,
            &offset
        );

        context->IASetIndexBuffer(
            gameObject->GetMesh()->GetIndexBuffer(),
            DXGI_FORMAT_R16_UINT, // Each index is one 16-bit unsigned integer (short).
            0
        );

        // Set Input Layout
        context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        context->IASetInputLayout(gameObject->GetShader()->GetVertexLayout().Get());

        // Set Vertex Shader
        context->VSSetShader(
            gameObject->GetShader()->GetVertexShader().Get(),
            nullptr,
            0
        );

        // Set Constant Buffer
        context->VSSetConstantBuffers(
            0,
            1,
            m_constantBuffer.GetAddressOf()
        );

        // Set null Pixel Shader
        context->PSSetShader(
            nullptr,
            nullptr,
            0
        );

        // Draw the GameObject
        context->DrawIndexed(
            gameObject->GetMesh()->GetNumIndices(),
            0,
            0
        );
    }

    // Unbind Render Targets
    ID3D11RenderTargetView* nullRTV = nullptr;  // Unbind render targets for use in next pass
    ID3D11DepthStencilView* nullDSV = nullptr;
    context->OMSetRenderTargets(1, &nullRTV, nullDSV);
}
