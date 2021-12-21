#include "pch.h"
#include "RenderPipelineGeometryPass.h"

RenderPipelineGeometryPass::RenderPipelineGeometryPass(Microsoft::WRL::ComPtr<ID3D11Device> _device,
                                                       Microsoft::WRL::ComPtr<ID3D11DeviceContext> _context, 
                                                       const std::vector<std::shared_ptr<GameObject>>& gameObjects,
                                                       const std::shared_ptr<Camera>& camera,
                                                       DirectX::XMINT2 renderResolution,
														RenderPipelineShadowPass* shadowPass)
    : RenderPipelineStage(_device, _context, renderResolution), m_gameObjects(gameObjects), m_camera(camera), m_shadowPass(shadowPass) {}

void RenderPipelineGeometryPass::Initialise() {
    // Create render texture and depth stencil
    Microsoft::WRL::ComPtr<ID3D11Texture2D> rttTex;
    D3D11_TEXTURE2D_DESC rttDesc = {};
    rttDesc.Width = m_resolution.x;
    rttDesc.Height = m_resolution.y;
    rttDesc.MipLevels = 1;
    rttDesc.ArraySize = 1;
    rttDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    rttDesc.SampleDesc.Count = 1;
    rttDesc.Usage = D3D11_USAGE_DEFAULT;
    rttDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    rttDesc.CPUAccessFlags = 0;
    rttDesc.MiscFlags = 0;
    DX::ThrowIfFailed(device->CreateTexture2D(&rttDesc, nullptr, rttTex.GetAddressOf()));
    DX::ThrowIfFailed(device->CreateRenderTargetView(rttTex.Get(), nullptr, m_renderTargetView.ReleaseAndGetAddressOf()));

    Microsoft::WRL::ComPtr<ID3D11Texture2D> rttTexHDR;
    DX::ThrowIfFailed(device->CreateTexture2D(&rttDesc, nullptr, rttTexHDR.GetAddressOf()));
    DX::ThrowIfFailed(device->CreateRenderTargetView(rttTexHDR.Get(), nullptr, m_renderTargetViewHDR.ReleaseAndGetAddressOf()));

	Microsoft::WRL::ComPtr<ID3D11Texture2D> rttDepthStencil;
    D3D11_TEXTURE2D_DESC depthStencilDesc = {};
    depthStencilDesc.Width = m_resolution.x;
    depthStencilDesc.Height = m_resolution.y;
    depthStencilDesc.MipLevels = 1;
    depthStencilDesc.ArraySize = 1;
    depthStencilDesc.Format = DXGI_FORMAT_R32_TYPELESS;
    depthStencilDesc.SampleDesc.Count = 1;
    depthStencilDesc.SampleDesc.Quality = 0;
    depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
    depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
    depthStencilDesc.CPUAccessFlags = 0;
    depthStencilDesc.MiscFlags = 0;
    DX::ThrowIfFailed(device->CreateTexture2D(&depthStencilDesc, nullptr, rttDepthStencil.GetAddressOf()));

    D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc = {};
    depthStencilViewDesc.Flags = 0;
    depthStencilViewDesc.Format = DXGI_FORMAT_D32_FLOAT;
    depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    depthStencilViewDesc.Texture2D.MipSlice = 0;
    DX::ThrowIfFailed(device->CreateDepthStencilView(rttDepthStencil.Get(), &depthStencilViewDesc, m_depthStencilView.ReleaseAndGetAddressOf()));

    // Create SRVs of render textures
    Microsoft::WRL::ComPtr<ID3D11Resource> geometryPassResource;
    m_renderTargetView->GetResource(geometryPassResource.ReleaseAndGetAddressOf());
    DX::ThrowIfFailed(device->CreateShaderResourceView(geometryPassResource.Get(), nullptr, m_renderTargetSRV.ReleaseAndGetAddressOf()));

    Microsoft::WRL::ComPtr<ID3D11Resource> geometryPassHDRResource;
    m_renderTargetViewHDR->GetResource(geometryPassHDRResource.ReleaseAndGetAddressOf());
    DX::ThrowIfFailed(device->CreateShaderResourceView(geometryPassHDRResource.Get(), nullptr, m_hdrRenderTargetSRV.ReleaseAndGetAddressOf()));

    // Create rasteriser state
    D3D11_RASTERIZER_DESC drawingRenderStateDesc;
    ZeroMemory(&drawingRenderStateDesc, sizeof(D3D11_RASTERIZER_DESC));
    drawingRenderStateDesc.CullMode = D3D11_CULL_BACK;
    drawingRenderStateDesc.FillMode = D3D11_FILL_SOLID;
    drawingRenderStateDesc.DepthClipEnable = true; 
    DX::ThrowIfFailed(
        device->CreateRasterizerState(
            &drawingRenderStateDesc,
            m_renderState.ReleaseAndGetAddressOf()
        )
    );

    // Create constant buffers
    D3D11_BUFFER_DESC bd = {};
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(ConstantBuffer);
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bd.CPUAccessFlags = 0;
    HRESULT hr = device->CreateBuffer(&bd, nullptr, m_constantBuffer.GetAddressOf());
    if (FAILED(hr))
        return;

    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(LightPropertiesConstantBuffer);
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bd.CPUAccessFlags = 0;
    hr = device->CreateBuffer(&bd, nullptr, m_lightConstantBuffer.GetAddressOf());
    if (FAILED(hr))
        return;
}

void RenderPipelineGeometryPass::Render() {
    // Clear views
    const DirectX::XMFLOAT4 tpClear = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
    const DirectX::XMFLOAT4 clearColour = m_camera->GetBackgroundColour();

	context->ClearRenderTargetView(m_renderTargetView.Get(), &clearColour.x);
    context->ClearRenderTargetView(m_renderTargetViewHDR.Get(), &tpClear.x);
    context->ClearDepthStencilView(m_depthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    // Bind RTVs and depth stencil
    ID3D11RenderTargetView* rtvs[2];
    rtvs[0] = m_renderTargetView.Get();
    rtvs[1] = m_renderTargetViewHDR.Get();
    context->OMSetRenderTargets(2, rtvs, m_depthStencilView.Get());

    // Set the viewport.
    CD3D11_VIEWPORT viewport(0.0f, 0.0f, static_cast<float>(m_resolution.x), static_cast<float>(m_resolution.y), 0.0f, 1.0f);
    context->RSSetState(m_renderState.Get());
	context->RSSetViewports(1, &viewport);

    SetupLightsForRender();

    // Render Geometry
    for (const auto& gameObject : m_gameObjects) {
        // get the game object world transform
        DirectX::XMMATRIX mGO = XMLoadFloat4x4(gameObject->GetTransform());

        // Get the game object's shader
        Shader* shader = gameObject->GetShader().get();

        // Set active vertex layout
        context->IASetInputLayout(shader->GetVertexLayout().Get());

        // store this and the view / projection in a constant buffer for the vertex shader to use
        ConstantBuffer cb1;
        cb1.mWorld = XMMatrixTranspose(mGO);
        cb1.mView = XMMatrixTranspose(m_camera->CalculateViewMatrix());
        cb1.mProjection = XMMatrixTranspose(m_camera->CalculateProjectionMatrix());
        cb1.vOutputColor = DirectX::XMFLOAT4(0, 0, 0, 0);
        cb1.camEyePos = m_camera->GetCameraPosition();
        context->UpdateSubresource(m_constantBuffer.Get(), 0, nullptr, &cb1, 0, 0);

        // Render the cube
        context->VSSetShader(shader->GetVertexShader().Get(), nullptr, 0);
        context->VSSetConstantBuffers(0, 1, m_constantBuffer.GetAddressOf());
    	context->VSSetConstantBuffers(1, 1, m_shadowPass->GetConstantBuffer().GetAddressOf());

        context->PSSetShader(shader->GetPixelShader().Get(), nullptr, 0);
        context->PSSetConstantBuffers(2, 1, m_lightConstantBuffer.GetAddressOf());
        ID3D11Buffer* materialCB = gameObject->GetMaterialConstantBuffer();
        context->PSSetConstantBuffers(1, 1, &materialCB);

        // Bind shadow depth view
        context->PSSetShaderResources(3, 1, m_shadowPass->GetSrv().GetAddressOf());

        context->PSSetSamplers(1, 1, m_shadowPass->GetSamplerState().GetAddressOf());

        gameObject->Render(context.Get());
    }

    // Unbind RTVs and DSV
    ID3D11RenderTargetView* nullRTV[2] = { nullptr, nullptr };  // Unbind render targets for use in next pass
    ID3D11DepthStencilView* nullDSV = nullptr;
    context->OMSetRenderTargets(2, &nullRTV[0], nullDSV);
}

void RenderPipelineGeometryPass::SetupLightsForRender() {
    Light light;
    light.Enabled = static_cast<int>(true);
    light.LightType = PointLight;
    light.Color = DirectX::XMFLOAT4(DirectX::Colors::Green);
    light.SpotAngle = DirectX::XMConvertToRadians(45.0f);
    light.ConstantAttenuation = 1.0f;
    light.LinearAttenuation = .1f;
    light.QuadraticAttenuation = 0.01f;

    DirectX::XMFLOAT4 LightPosition(0.0f, 2.0f, -4.0f, 1.0f);
    light.Position = LightPosition;
    DirectX::XMVECTOR LightDirection = DirectX::XMVectorSet(-LightPosition.x, -LightPosition.y, -LightPosition.z, 0.0f);
    LightDirection = DirectX::XMVector3Normalize(LightDirection);
    XMStoreFloat4(&light.Direction, LightDirection);

    LightPropertiesConstantBuffer lightProperties;
    lightProperties.EyePosition = m_camera->GetCameraPosition();
    lightProperties.Lights[0] = light;

    light.Position = DirectX::XMFLOAT4(0.0f, 0.5f, 2.0f, 1.0f);
    light.Color = DirectX::XMFLOAT4(DirectX::Colors::Purple);
    lightProperties.Lights[1] = light;

    context->UpdateSubresource(m_lightConstantBuffer.Get(), 0, nullptr, &lightProperties, 0, 0);
}