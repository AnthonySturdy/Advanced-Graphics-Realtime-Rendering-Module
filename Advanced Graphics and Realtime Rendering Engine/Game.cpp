#include "pch.h"
#include "Game.h"

extern void ExitGame() noexcept;

using namespace DirectX;

using Microsoft::WRL::ComPtr;

Game::Game() noexcept :
    m_window(nullptr),
    m_outputWidth(1280),
    m_outputHeight(720),
    m_featureLevel(D3D_FEATURE_LEVEL_9_1)
{
}

// Initialize the Direct3D resources required to run.
void Game::Initialise(HWND window, int width, int height)
{
    m_window = window;
    m_outputWidth = std::max(width, 1);
    m_outputHeight = std::max(height, 1);

    CreateDevice();

    CreateResources();

    InitialiseImGui(window);

    CreateConstantBuffers();

    CreateCameras(width, height);
    
    CreateGameObjects();

    m_timer.SetFixedTimeStep(true);
    m_timer.SetTargetElapsedSeconds(1.0 / 60);
}

// Executes the basic game loop.
void Game::Tick()
{
    m_timer.Tick([&]()
    {
        Update(m_timer);
    });

    Render();
}

// Updates the world.
void Game::Update(DX::StepTimer const& timer)
{
    float elapsedTime = float(timer.GetElapsedSeconds());

    // TODO: Add your game logic here.
    m_gameObject->Update(elapsedTime, m_d3dContext.Get());
}

// Draws the scene.
void Game::Render()
{
    // Don't try to render anything before the first Update.
    if (m_timer.GetFrameCount() == 0)
    {
        return;
    }

    ID3D11UnorderedAccessView* nullUav = nullptr;
    ID3D11ShaderResourceView* nullSrv = nullptr;

    // Bind render target to intermediate RenderTargetView
    ID3D11RenderTargetView** rtvs[2];
    rtvs[0] = m_rttRenderTargetViews.GetAddressOf();
    rtvs[1] = m_rttRenderTargetViewsHDR.GetAddressOf();

    XMFLOAT4 tpClear = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);                
    m_d3dContext->ClearRenderTargetView(*rtvs[1], &tpClear.x);  // Clear HDR RTV with transparent
    SetRenderTargetAndClear(rtvs[0], m_rttDepthStencilViews.Get(), 2);  // Clear main RTV and set render target

    // Start the Dear ImGui frame
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
    ImGuizmo::BeginFrame();

    //
    // Geometry render pass
    //
    // get the game object world transform
    XMMATRIX mGO = XMLoadFloat4x4(m_gameObject->GetTransform());

    // Get the game object's shader
    Shader* shader = m_gameObject->GetShader().get();

    // Set active vertex layout
    m_d3dContext->IASetInputLayout(shader->GetVertexLayout().Get());

    // store this and the view / projection in a constant buffer for the vertex shader to use
    ConstantBuffer cb1;
    cb1.mWorld = XMMatrixTranspose(mGO);
    cb1.mView = XMMatrixTranspose(m_camera->CalculateViewMatrix());
    cb1.mProjection = XMMatrixTranspose(m_camera->CalculateProjectionMatrix());
    cb1.vOutputColor = XMFLOAT4(0, 0, 0, 0);
    m_d3dContext->UpdateSubresource(m_constantBuffer.Get(), 0, nullptr, &cb1, 0, 0);

    SetupLightsForRender();

    // Render the cube
    m_d3dContext->VSSetShader(shader->GetVertexShader().Get(), nullptr, 0);
    m_d3dContext->VSSetConstantBuffers(0, 1, m_constantBuffer.GetAddressOf());

    m_d3dContext->PSSetShader(shader->GetPixelShader().Get(), nullptr, 0);
    m_d3dContext->PSSetConstantBuffers(2, 1, m_lightConstantBuffer.GetAddressOf());
    ID3D11Buffer* materialCB = m_gameObject->GetMaterialConstantBuffer();
    m_d3dContext->PSSetConstantBuffers(1, 1, &materialCB);

    m_gameObject->Render(m_d3dContext.Get());

    ID3D11RenderTargetView* nullRTV[2] = { nullptr, nullptr };  // Unbind render targets for use in next pass
	m_d3dContext->OMSetRenderTargets(1, &nullRTV[0], nullptr);

    //
    // Bloom pass
    //
    // Convert RTV to SRV for passing into shader
    ComPtr<ID3D11Resource> geometryPassResource;
    m_rttRenderTargetViews->GetResource(geometryPassResource.ReleaseAndGetAddressOf());
    ComPtr<ID3D11ShaderResourceView> geometryPassSrv;
    m_d3dDevice->CreateShaderResourceView(geometryPassResource.Get(), nullptr, geometryPassSrv.ReleaseAndGetAddressOf());

    ComPtr<ID3D11Resource> geometryPassHDRResource;
    m_rttRenderTargetViewsHDR->GetResource(geometryPassHDRResource.ReleaseAndGetAddressOf());
    ComPtr<ID3D11ShaderResourceView> geometryPassHDRSrv;
    m_d3dDevice->CreateShaderResourceView(geometryPassHDRResource.Get(), nullptr, geometryPassHDRSrv.ReleaseAndGetAddressOf());

    // Update GPU with Gaussian blur cbuffer
    static GaussianBlurConstantBuffer gbcb = { 25.0f, 8.0f, 25.0f, 0.0f };
    m_d3dContext->UpdateSubresource(m_gaussianBlurConstantBuffer.Get(), 0, nullptr, &gbcb, 0, 0);
    m_d3dContext->CSSetConstantBuffers(0, 1, m_gaussianBlurConstantBuffer.GetAddressOf());

    // Pass textures to compute shader
    m_d3dContext->CSSetShaderResources(0, 1, geometryPassSrv.GetAddressOf());
    m_d3dContext->CSSetShaderResources(1, 1, geometryPassHDRSrv.GetAddressOf());
    m_d3dContext->CSSetUnorderedAccessViews(0, 1, m_postProcUnorderedAccessView.GetAddressOf(), nullptr);

    // Dispatch horizontal blur pass
    m_d3dContext->CSSetShader(m_bloomComputeShader->GetComputeShader(), nullptr, 0); 
    m_d3dContext->Dispatch(m_outputWidth / 4, m_outputHeight / 4, 1);

    // Unbind textures after Dispatch
    m_d3dContext->CSSetUnorderedAccessViews(0, 1, &nullUav, nullptr);
    m_d3dContext->CSSetShaderResources(0, 1, &nullSrv);
    m_d3dContext->CSSetShaderResources(1, 1, &nullSrv);

    //
    // Image filter pass
    //
    static ImageFilterConstantBuffer ifcb = { 0, 0.0f, 0.0f, 0.0f, m_viewportSize.x, m_viewportSize.y, 0.0f, 0.0f };
    if(ifcb.filterIntensity > 0) {
        ifcb.time = m_timer.GetTotalSeconds();
        ifcb.resolution[0] = m_outputWidth;
        ifcb.resolution[1] = m_outputHeight;

        m_d3dContext->UpdateSubresource(m_imageFilterConstantBuffer.Get(), 0, nullptr, &ifcb, 0, 0);
        m_d3dContext->CSSetConstantBuffers(0, 1, m_imageFilterConstantBuffer.GetAddressOf());

        // Pass textures to compute shader
        m_d3dContext->CSSetUnorderedAccessViews(0, 1, m_postProcUnorderedAccessView.GetAddressOf(), nullptr);

        // Dispatch horizontal blur pass
        m_d3dContext->CSSetShader(m_imageFilterComputeShader->GetComputeShader(), nullptr, 0);
        m_d3dContext->Dispatch(m_outputWidth / 8, m_outputHeight / 8, 1);

        // Unbind textures after Dispatch
        m_d3dContext->CSSetUnorderedAccessViews(0, 1, &nullUav, nullptr);
        m_d3dContext->CSSetShaderResources(0, 1, &nullSrv);
        m_d3dContext->CSSetShaderResources(1, 1, &nullSrv);
    }

    //
	// Gui render pass to back buffer
	//
    // Bind render target to back buffer
    SetRenderTargetAndClear(m_renderTargetView.GetAddressOf(), m_depthStencilView.Get());

    // Render dockspace
    ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());

    // Render content in ImGui window
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2());
    ImGui::Begin("Viewport");
        // Detect viewport window resize
        ImVec2 curVpSize = ImGui::GetContentRegionAvail();
        if (m_viewportSize.x != curVpSize.x || m_viewportSize.y != curVpSize.y) {
            m_viewportSize = curVpSize;
            OnViewportSizeChanged();
        }
        
        // Create SRV and render to ImGui window
        ComPtr<ID3D11Resource> renderResource;
        m_postProcUnorderedAccessView->GetResource(renderResource.ReleaseAndGetAddressOf());
        ComPtr<ID3D11ShaderResourceView> renderSrv;
        m_d3dDevice->CreateShaderResourceView(renderResource.Get(), nullptr, renderSrv.ReleaseAndGetAddressOf());
        ImGui::Image(renderSrv.Get(), m_viewportSize);

        // Set up ImGuizmo
        ImGuizmo::SetDrawlist();
        ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, m_viewportSize.x, m_viewportSize.y);
    ImGui::End();
    ImGui::PopStyleVar();

    // Render frames per second window
    ImGui::Begin("Performance", (bool*)0, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::Text("%.3fms (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    ImGui::End();

    ImGui::Begin("Scene Controls", (bool*)0, ImGuiWindowFlags_AlwaysAutoResize);
        m_camera->RenderGUIControls();
        m_gameObject->RenderGUIControls(m_d3dDevice.Get(), m_camera.get());
    ImGui::End();

    ImGui::Begin("Post Processing Controls", (bool*)0, ImGuiWindowFlags_AlwaysAutoResize);
		if(ImGui::CollapsingHeader("Guassian Blur")) {
            ImGui::DragFloat("Size", &gbcb.size, 0.1f, 0.0f, 50.0f);
            ImGui::DragFloat("Quality", &gbcb.quality, 0.1f, 0.0f, 50.0f);
            ImGui::DragFloat("Directions", &gbcb.directions, 0.1f, 0.0f, 50.0f);
		}

        if (ImGui::CollapsingHeader("Image Filters")) {
            const char* items[] = { "Invert", "Greyscale", "Film Grain", "Vignette" };
            static int selection = ifcb.filterType;
            ImGui::Combo("Filter Type", &selection, items, 4);
            ifcb.filterType = selection;

            ImGui::DragFloat("Intensity", &ifcb.filterIntensity, 0.01f, 0.0f, 1.0f);
        }
    ImGui::End();

    // Render ImGui
    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    // Update and Render additional Platform Windows
    if (m_ioImGui->ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
    }

    Present();

}

void Game::SetupLightsForRender() {
    Light light;
    light.Enabled = static_cast<int>(true);
    light.LightType = PointLight;
    light.Color = XMFLOAT4(Colors::White);
    light.SpotAngle = XMConvertToRadians(45.0f);
    light.ConstantAttenuation = 1.0f;
    light.LinearAttenuation = .1f;
    light.QuadraticAttenuation = 0.01f;

    XMFLOAT4 LightPosition(0.0f, 4.0f, -4.0f, 1.0f);
    light.Position = LightPosition;
    XMVECTOR LightDirection = XMVectorSet(-LightPosition.x, -LightPosition.y, -LightPosition.z, 0.0f);
    LightDirection = XMVector3Normalize(LightDirection);
    XMStoreFloat4(&light.Direction, LightDirection);

    LightPropertiesConstantBuffer lightProperties;
    lightProperties.EyePosition = m_camera->GetCameraPosition();
    lightProperties.Lights[0] = light;
    m_d3dContext->UpdateSubresource(m_lightConstantBuffer.Get(), 0, nullptr, &lightProperties, 0, 0);
}

// Helper method to clear the back buffers.
void Game::SetRenderTargetAndClear(ID3D11RenderTargetView** rtv, ID3D11DepthStencilView* dsv, int numViews)
{
    // Clear the views.
    XMFLOAT4 clearColour = m_camera->GetBackgroundColour();
    m_d3dContext->ClearRenderTargetView(*rtv, &clearColour.x);
    m_d3dContext->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    m_d3dContext->OMSetRenderTargets(numViews, rtv, dsv);

    // Set the viewport.
    CD3D11_VIEWPORT viewport(0.0f, 0.0f, static_cast<float>(m_outputWidth), static_cast<float>(m_outputHeight));
    m_d3dContext->RSSetViewports(1, &viewport);
}

// Presents the back buffer contents to the screen.
void Game::Present()
{
    // The first argument instructs DXGI to block until VSync, putting the application
    // to sleep until the next VSync. This ensures we don't waste any cycles rendering
    // frames that will never be displayed to the screen.
    HRESULT hr = m_swapChain->Present(1, 0);

    // If the device was reset we must completely reinitialize the renderer.
    if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
    {
        OnDeviceLost();
    }
    else
    {
        DX::ThrowIfFailed(hr);
    }
}

// Message handlers
void Game::OnActivated()
{
    // TODO: Game is becoming active window.
}

void Game::OnDeactivated()
{
    // TODO: Game is becoming background window.
}

void Game::OnSuspending()
{
    // TODO: Game is being power-suspended (or minimized).
}

void Game::OnResuming()
{
    m_timer.ResetElapsedTime();

    // TODO: Game is being power-resumed (or returning from minimize).
}

void Game::OnWindowSizeChanged(int width, int height)
{
    m_outputWidth = std::max(width, 1);
    m_outputHeight = std::max(height, 1);

    CreateResources();

    // TODO: Game window is being resized.
}

void Game::OnViewportSizeChanged()
{
    if (m_camera)
        m_camera->SetAspectRatio(m_viewportSize.x / (float)m_viewportSize.y);

    CreateResources();
}

// Properties
void Game::GetDefaultSize(int& width, int& height) const noexcept
{
    // TODO: Change to desired default window size (note minimum size is 320x200).
    width = 1280;
    height = 720;
}

// These are the resources that depend on the device.
void Game::CreateDevice()
{
    UINT creationFlags = 0;

#ifdef _DEBUG
    creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    static const D3D_FEATURE_LEVEL featureLevels [] =
    {
        // TODO: Modify for supported Direct3D feature levels
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
        D3D_FEATURE_LEVEL_9_3,
        D3D_FEATURE_LEVEL_9_2,
        D3D_FEATURE_LEVEL_9_1,
    };

    // Create the DX11 API device object, and get a corresponding context.
    ComPtr<ID3D11Device> device;
    ComPtr<ID3D11DeviceContext> context;
    DX::ThrowIfFailed(D3D11CreateDevice(
        nullptr,                            // specify nullptr to use the default adapter
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        creationFlags,
        featureLevels,
        static_cast<UINT>(std::size(featureLevels)),
        D3D11_SDK_VERSION,
        device.ReleaseAndGetAddressOf(),    // returns the Direct3D device created
        &m_featureLevel,                    // returns feature level of device created
        context.ReleaseAndGetAddressOf()    // returns the device immediate context
        ));

#ifndef NDEBUG
    ComPtr<ID3D11Debug> d3dDebug;
    if (SUCCEEDED(device.As(&d3dDebug)))
    {
        ComPtr<ID3D11InfoQueue> d3dInfoQueue;
        if (SUCCEEDED(d3dDebug.As(&d3dInfoQueue)))
        {
#ifdef _DEBUG
            d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, true);
            d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, true);
#endif
            D3D11_MESSAGE_ID hide [] =
            {
                D3D11_MESSAGE_ID_SETPRIVATEDATA_CHANGINGPARAMS,
                // TODO: Add more message IDs here as needed.
            };
            D3D11_INFO_QUEUE_FILTER filter = {};
            filter.DenyList.NumIDs = static_cast<UINT>(std::size(hide));
            filter.DenyList.pIDList = hide;
            d3dInfoQueue->AddStorageFilterEntries(&filter);
        }
    }
#endif

    DX::ThrowIfFailed(device.As(&m_d3dDevice));
    DX::ThrowIfFailed(context.As(&m_d3dContext));

    // TODO: Initialize device dependent objects here (independent of window size).
}

// Allocate all memory resources that change on a window SizeChanged event.
void Game::CreateResources()
{
    // Clear the previous window size specific context.
    ID3D11RenderTargetView* nullViews [] = { nullptr };
    m_d3dContext->OMSetRenderTargets(static_cast<UINT>(std::size(nullViews)), nullViews, nullptr);
    m_renderTargetView.Reset();
    m_depthStencilView.Reset();
    m_d3dContext->Flush();

    const UINT backBufferWidth = static_cast<UINT>(m_outputWidth);
    const UINT backBufferHeight = static_cast<UINT>(m_outputHeight);
    const DXGI_FORMAT backBufferFormat = DXGI_FORMAT_B8G8R8A8_UNORM;
    const DXGI_FORMAT depthBufferFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
    constexpr UINT backBufferCount = 2;

    // If the swap chain already exists, resize it, otherwise create one.
    if (m_swapChain)
    {
        HRESULT hr = m_swapChain->ResizeBuffers(backBufferCount, backBufferWidth, backBufferHeight, backBufferFormat, 0);

        if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
        {
            // If the device was removed for any reason, a new device and swap chain will need to be created.
            OnDeviceLost();

            // Everything is set up now. Do not continue execution of this method. OnDeviceLost will reenter this method 
            // and correctly set up the new device.
            return;
        }
        else
        {
            DX::ThrowIfFailed(hr);
        }
    }
    else
    {
        // First, retrieve the underlying DXGI Device from the D3D Device.
        ComPtr<IDXGIDevice1> dxgiDevice;
        DX::ThrowIfFailed(m_d3dDevice.As(&dxgiDevice));

        // Identify the physical adapter (GPU or card) this device is running on.
        ComPtr<IDXGIAdapter> dxgiAdapter;
        DX::ThrowIfFailed(dxgiDevice->GetAdapter(dxgiAdapter.GetAddressOf()));

        // And obtain the factory object that created it.
        ComPtr<IDXGIFactory2> dxgiFactory;
        DX::ThrowIfFailed(dxgiAdapter->GetParent(IID_PPV_ARGS(dxgiFactory.GetAddressOf())));

        // Create a descriptor for the swap chain.
        DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
        swapChainDesc.Width = backBufferWidth;
        swapChainDesc.Height = backBufferHeight;
        swapChainDesc.Format = backBufferFormat;
        swapChainDesc.SampleDesc.Count = 1;
        swapChainDesc.SampleDesc.Quality = 0;
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.BufferCount = backBufferCount;

        DXGI_SWAP_CHAIN_FULLSCREEN_DESC fsSwapChainDesc = {};
        fsSwapChainDesc.Windowed = TRUE;

        // Create a SwapChain from a Win32 window.
        DX::ThrowIfFailed(dxgiFactory->CreateSwapChainForHwnd(
            m_d3dDevice.Get(),
            m_window,
            &swapChainDesc,
            &fsSwapChainDesc,
            nullptr,
            m_swapChain.ReleaseAndGetAddressOf()
            ));

        // This template does not support exclusive fullscreen mode and prevents DXGI from responding to the ALT+ENTER shortcut.
        DX::ThrowIfFailed(dxgiFactory->MakeWindowAssociation(m_window, DXGI_MWA_NO_ALT_ENTER));
    }

    // Obtain the backbuffer for this window which will be the final 3D rendertarget.
    ComPtr<ID3D11Texture2D> m_backBuffer;
    DX::ThrowIfFailed(m_swapChain->GetBuffer(0, IID_PPV_ARGS(m_backBuffer.GetAddressOf())));

    // Create a view interface on the rendertarget to use on bind.
    DX::ThrowIfFailed(m_d3dDevice->CreateRenderTargetView(m_backBuffer.Get(), nullptr, m_renderTargetView.ReleaseAndGetAddressOf()));

    // Allocate a 2-D surface as the depth/stencil buffer and
    // create a DepthStencil view on this surface to use on bind.
    CD3D11_TEXTURE2D_DESC depthStencilDesc(depthBufferFormat, backBufferWidth, backBufferHeight, 1, 1, D3D11_BIND_DEPTH_STENCIL);
    ComPtr<ID3D11Texture2D> depthStencil;
    DX::ThrowIfFailed(m_d3dDevice->CreateTexture2D(&depthStencilDesc, nullptr, depthStencil.GetAddressOf()));

    CD3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc(D3D11_DSV_DIMENSION_TEXTURE2D);
    DX::ThrowIfFailed(m_d3dDevice->CreateDepthStencilView(depthStencil.Get(), &depthStencilViewDesc, m_depthStencilView.ReleaseAndGetAddressOf()));

    // Create intermediate render texture and depth stencil
    Microsoft::WRL::ComPtr<ID3D11Texture2D> rttTex;
    D3D11_TEXTURE2D_DESC rttDesc = {};
    rttDesc.Width = backBufferWidth;
    rttDesc.Height = backBufferHeight;
    rttDesc.MipLevels = 1;
    rttDesc.ArraySize = 1;
    rttDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    rttDesc.SampleDesc.Count = 1;
    rttDesc.Usage = D3D11_USAGE_DEFAULT;
    rttDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    rttDesc.CPUAccessFlags = 0;
    rttDesc.MiscFlags = 0;
    DX::ThrowIfFailed(m_d3dDevice->CreateTexture2D(&rttDesc, nullptr, rttTex.GetAddressOf()));
    DX::ThrowIfFailed(m_d3dDevice->CreateRenderTargetView(rttTex.Get(), nullptr, m_rttRenderTargetViews.ReleaseAndGetAddressOf()));

    Microsoft::WRL::ComPtr<ID3D11Texture2D> rttTexHDR;
    DX::ThrowIfFailed(m_d3dDevice->CreateTexture2D(&rttDesc, nullptr, rttTexHDR.GetAddressOf()));
    DX::ThrowIfFailed(m_d3dDevice->CreateRenderTargetView(rttTexHDR.Get(), nullptr, m_rttRenderTargetViewsHDR.ReleaseAndGetAddressOf()));

    ComPtr<ID3D11Texture2D> rttDepthStencil;
    DX::ThrowIfFailed(m_d3dDevice->CreateTexture2D(&depthStencilDesc, nullptr, rttDepthStencil.GetAddressOf()));
    DX::ThrowIfFailed(m_d3dDevice->CreateDepthStencilView(rttDepthStencil.Get(), &depthStencilViewDesc, m_rttDepthStencilViews.ReleaseAndGetAddressOf()));

    // Create post processing Unordered Access Resource (UAV)
    Microsoft::WRL::ComPtr<ID3D11Texture2D> postProcUAVTex;
    D3D11_TEXTURE2D_DESC postProcTexDesc = {};
    postProcTexDesc.Width = backBufferWidth; //= std::max(m_viewportSize.x, 1.0f);
    postProcTexDesc.Height = backBufferHeight; //std::max(m_viewportSize.y, 1.0f);
    postProcTexDesc.MipLevels = 1;
    postProcTexDesc.ArraySize = 1;
    postProcTexDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    postProcTexDesc.SampleDesc.Count = 1;
    postProcTexDesc.SampleDesc.Quality = 0;
    postProcTexDesc.Usage = D3D11_USAGE_DEFAULT;
    postProcTexDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
    postProcTexDesc.CPUAccessFlags = 0;
    postProcTexDesc.MiscFlags = 0;

    D3D11_UNORDERED_ACCESS_VIEW_DESC postProcUAVDesc;
    postProcUAVDesc.Format = DXGI_FORMAT_UNKNOWN;
    postProcUAVDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
    postProcUAVDesc.Texture2D.MipSlice = 0;

    DX::ThrowIfFailed(m_d3dDevice->CreateTexture2D(&postProcTexDesc, nullptr, postProcUAVTex.GetAddressOf()));
    DX::ThrowIfFailed(m_d3dDevice->CreateUnorderedAccessView(postProcUAVTex.Get(), &postProcUAVDesc, m_postProcUnorderedAccessView.ReleaseAndGetAddressOf()));

}

void Game::InitialiseImGui(HWND hwnd)
{
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    m_ioImGui = &ImGui::GetIO(); (void)m_ioImGui;
    m_ioImGui->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
    m_ioImGui->ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
    m_ioImGui->ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
    m_ioImGui->ConfigWindowsMoveFromTitleBarOnly = true;

    ImGui::StyleColorsClassic();

    // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 5.0f;
    style.FrameRounding = 3.0f;
    style.ItemSpacing = ImVec2(6, 4);
    style.ItemInnerSpacing = ImVec2(4, 3);
    style.ScrollbarSize = 10.0f;
    style.ScrollbarRounding = 20.0f;

    ImVec4* colors = style.Colors;
    colors[ImGuiCol_Text] = ImVec4(0.86f, 0.93f, 0.89f, 0.78f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.86f, 0.93f, 0.89f, 0.28f);
    colors[ImGuiCol_WindowBg] = ImVec4(0.01f, 0.01f, 0.02f, 1.00f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.13f, 0.15f, 0.22f, 0.58f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.13f, 0.15f, 0.22f, 0.90f);
    colors[ImGuiCol_Border] = ImVec4(0.10f, 0.12f, 0.17f, 0.60f);
    colors[ImGuiCol_BorderShadow] = ImVec4(0.13f, 0.15f, 0.22f, 0.00f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.13f, 0.15f, 0.22f, 1.00f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.03f, 0.03f, 0.03f, 0.78f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.03f, 0.03f, 0.03f, 1.00f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.05f, 0.05f, 0.05f, 1.00f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.12f, 0.00f, 0.39f, 0.87f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.13f, 0.15f, 0.22f, 0.75f);
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.13f, 0.15f, 0.22f, 0.47f);
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.13f, 0.15f, 0.22f, 1.00f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.05f, 0.05f, 0.05f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.03f, 0.03f, 0.03f, 0.78f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.03f, 0.03f, 0.03f, 1.00f);
    colors[ImGuiCol_CheckMark] = ImVec4(0.56f, 0.56f, 0.56f, 1.00f);
    colors[ImGuiCol_SliderGrab] = ImVec4(0.18f, 0.22f, 0.32f, 1.00f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.56f, 0.56f, 0.56f, 1.00f);
    colors[ImGuiCol_Button] = ImVec4(0.24f, 0.29f, 0.42f, 1.00f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.03f, 0.03f, 0.03f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.56f, 0.56f, 0.56f, 1.00f);
    colors[ImGuiCol_Header] = ImVec4(0.03f, 0.03f, 0.03f, 0.76f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.03f, 0.03f, 0.03f, 0.86f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.56f, 0.56f, 0.56f, 1.00f);
    colors[ImGuiCol_Separator] = ImVec4(0.50f, 0.50f, 0.50f, 0.60f);
    colors[ImGuiCol_SeparatorHovered] = ImVec4(0.60f, 0.60f, 0.70f, 1.00f);
    colors[ImGuiCol_SeparatorActive] = ImVec4(0.70f, 0.70f, 0.90f, 1.00f);
    colors[ImGuiCol_ResizeGrip] = ImVec4(0.47f, 0.77f, 0.83f, 0.04f);
    colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.03f, 0.03f, 0.03f, 0.78f);
    colors[ImGuiCol_ResizeGripActive] = ImVec4(0.03f, 0.03f, 0.03f, 1.00f);
    colors[ImGuiCol_Tab] = ImVec4(0.13f, 0.15f, 0.22f, 0.40f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.56f, 0.56f, 0.56f, 1.00f);
    colors[ImGuiCol_TabActive] = ImVec4(0.03f, 0.03f, 0.03f, 1.00f);
    colors[ImGuiCol_TabUnfocused] = ImVec4(0.13f, 0.15f, 0.22f, 0.40f);
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.13f, 0.15f, 0.22f, 0.70f);
    colors[ImGuiCol_DockingPreview] = ImVec4(0.56f, 0.56f, 0.56f, 0.30f);
    colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    colors[ImGuiCol_PlotLines] = ImVec4(0.86f, 0.93f, 0.89f, 0.63f);
    colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.03f, 0.03f, 0.03f, 1.00f);
    colors[ImGuiCol_PlotHistogram] = ImVec4(0.86f, 0.93f, 0.89f, 0.63f);
    colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.03f, 0.03f, 0.03f, 1.00f);
    colors[ImGuiCol_TableHeaderBg] = ImVec4(0.27f, 0.27f, 0.38f, 1.00f);
    colors[ImGuiCol_TableBorderStrong] = ImVec4(0.31f, 0.31f, 0.45f, 1.00f);
    colors[ImGuiCol_TableBorderLight] = ImVec4(0.26f, 0.26f, 0.28f, 1.00f);
    colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.07f);
    colors[ImGuiCol_TextSelectedBg] = ImVec4(0.03f, 0.03f, 0.03f, 0.43f);
    colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
    colors[ImGuiCol_NavHighlight] = ImVec4(0.45f, 0.45f, 0.90f, 0.80f);
    colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
    colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
    colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.13f, 0.15f, 0.22f, 0.73f);

    if (m_ioImGui->ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(m_d3dDevice.Get(), m_d3dContext.Get());
}

void Game::CreateConstantBuffers() {
    // Create the constant buffer
    D3D11_BUFFER_DESC bd = {};
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(ConstantBuffer);
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bd.CPUAccessFlags = 0;
    HRESULT hr = m_d3dDevice->CreateBuffer(&bd, nullptr, m_constantBuffer.GetAddressOf());
    if (FAILED(hr))
        return;

    // Create the light constant buffer
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(LightPropertiesConstantBuffer);
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bd.CPUAccessFlags = 0;
    hr = m_d3dDevice->CreateBuffer(&bd, nullptr, m_lightConstantBuffer.GetAddressOf());
    if (FAILED(hr))
        return;

    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(GaussianBlurConstantBuffer);
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bd.CPUAccessFlags = 0;
    hr = m_d3dDevice->CreateBuffer(&bd, nullptr, m_gaussianBlurConstantBuffer.GetAddressOf());
    if (FAILED(hr))
        return;

    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(ImageFilterConstantBuffer);
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bd.CPUAccessFlags = 0;
    hr = m_d3dDevice->CreateBuffer(&bd, nullptr, m_imageFilterConstantBuffer.GetAddressOf());
    if (FAILED(hr))
        return;
}

void Game::CreateCameras(int width, int height) {
    // Create camera
    m_camera = std::make_shared<Camera>(XMFLOAT4(-4.0f, 4.0f, -4.0f, 1.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 0.0f),
                                        Camera::CAMERA_TYPE::PERSPECTIVE,
                                        width / (float)height,
                                        DirectX::XM_PIDIV2,
                                        0.01f, 100.0f);
}

void Game::CreateGameObjects() {
    // Create and initialise GameObject
    m_gameObject = std::make_shared<GameObject_Cube>();
    m_gameObject->InitMesh(m_d3dDevice.Get(), m_d3dContext.Get());
    D3D11_INPUT_ELEMENT_DESC layout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT , D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT , D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT , D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT , D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };
    UINT numElements = ARRAYSIZE(layout);
    m_gameObject->InitShader(m_d3dDevice.Get(), L"VertexShader", L"PixelShader", layout, numElements);

    m_bloomComputeShader = std::make_unique<ComputeShader>(m_d3dDevice.Get(), L"BloomShader");
    m_imageFilterComputeShader = std::make_unique<ComputeShader>(m_d3dDevice.Get(), L"ImageFilterShader");
}

void Game::OnDeviceLost()
{
    // TODO: Add Direct3D resource cleanup here.

    m_depthStencilView.Reset();
    m_renderTargetView.Reset();
    m_swapChain.Reset();
    m_d3dContext.Reset();
    m_d3dDevice.Reset();

    CreateDevice();

    CreateResources();
}