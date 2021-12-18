#pragma once

#include "StepTimer.h"
#include "Structures.h"

#include "GameObject.h"
#include "GameObject_Cube.h"
#include "GameObject_Quad.h"
#include "Camera.h"
#include "ComputeShader.h"
#include "RenderPipelineStage.h"
#include "RenderPipelineGeometryPass.h"

// A basic game implementation that creates a D3D11 device and
// provides a game loop.
class Game
{
public:

    Game() noexcept;
    ~Game() = default;

    Game(Game&&) = default;
    Game& operator= (Game&&) = default;

    Game(Game const&) = delete;
    Game& operator= (Game const&) = delete;

    // Initialization and management
    void Initialise(HWND window, int width, int height);

    // Basic game loop
    void Tick();

    // Messages
    void OnActivated();
    void OnDeactivated();
    void OnSuspending();
    void OnResuming();
    void OnWindowSizeChanged(int width, int height);
    void OnViewportSizeChanged();

    // Properties
    void GetDefaultSize( int& width, int& height ) const noexcept;

private:

    void Update(DX::StepTimer const& timer);
    void Render();

    void SetRenderTargetAndClear(ID3D11RenderTargetView** rtv, ID3D11DepthStencilView* dsv, int numViews = 1);
    void Present();

    void CreateDevice();
    void CreateResources();
    void InitialiseImGui(HWND hwnd);
    void CreateConstantBuffers();
    void CreateCameras(int width, int height);
    void CreateGameObjects();

    void OnDeviceLost();

    // Device resources.
    HWND                                                m_window;
    int                                                 m_outputWidth;
    int                                                 m_outputHeight;

    D3D_FEATURE_LEVEL                                   m_featureLevel;
    Microsoft::WRL::ComPtr<ID3D11Device1>               m_d3dDevice;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext1>        m_d3dContext;

    Microsoft::WRL::ComPtr<IDXGISwapChain1>             m_swapChain;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView>      m_renderTargetView;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilView>      m_depthStencilView;

    
    Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView>   m_postProcUnorderedAccessView;  // Post processing passes read/write, final image rendered to viewport

    ImVec2                                              m_viewportSize;

    // Rendering loop timer.
    DX::StepTimer                                       m_timer;

    // ImGui
    ImGuiIO* m_ioImGui = nullptr;

    // Scene
    std::shared_ptr<Camera> m_camera;
    std::vector<std::shared_ptr<GameObject>> m_gameObjects;

    std::shared_ptr<ComputeShader> m_depthOfFieldComputeShader;
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_depthOfFieldConstantBuffer;

    std::shared_ptr<ComputeShader> m_bloomComputeShader;
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_bloomConstantBuffer;

    std::shared_ptr<ComputeShader> m_imageFilterComputeShader;
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_imageFilterConstantBuffer;

    RenderPipelineGeometryPass* geometryPass;
};
