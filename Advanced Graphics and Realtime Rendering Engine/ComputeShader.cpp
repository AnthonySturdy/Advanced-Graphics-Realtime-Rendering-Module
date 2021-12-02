#include "pch.h"
#include "ComputeShader.h"

#ifdef _MSC_VER
#pragma comment(lib, "d3dcompiler") // Automatically link with d3dcompiler.lib as we are using D3DCompileFromFile() below.
#endif

ComputeShader::ComputeShader(ID3D11Device* device, const WCHAR* shaderPathWithoutExt) {
	// Compile and create Vertex shader
    ID3DBlob* csBlob = nullptr;
    std::wstring compShaderCso(shaderPathWithoutExt); compShaderCso += L".cso";
    if (FAILED(D3DReadFileToBlob(compShaderCso.c_str(), &csBlob))) {        // Attempt to load pre-compiled vertex shader
        // Attempt to compile at runtime if precompiled shader fails
        std::wstring compShaderHlsl(shaderPathWithoutExt); compShaderHlsl += L".hlsl";
        if (FAILED(CompileShaderFromFile(compShaderHlsl.c_str(), "CS", "cs_5_0", &csBlob))) {
            MessageBox(nullptr, L"The HLSL file cannot be compiled. Ensure this executable is relative to the specified file path.", L"Error", MB_OK);
            return;
        }
    }
    
    if (FAILED(device->CreateComputeShader(csBlob->GetBufferPointer(), csBlob->GetBufferSize(), nullptr, &m_ComputeShader))) {
        csBlob->Release();
        return;
    }

    // Release blobs
    csBlob->Release();
}

ComputeShader::~ComputeShader() {}

HRESULT ComputeShader::CompileShaderFromFile(const WCHAR* fileName, LPCSTR entryPoint, LPCSTR shaderModel, ID3DBlob** blobOut) {
    HRESULT hr = S_OK;

    DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
    // Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
    // Improves shader debugging experience but still allows for optimisation.
    dwShaderFlags |= D3DCOMPILE_DEBUG;

    // Disable optimizations to further improve shader debugging.
    dwShaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    ID3DBlob* pErrorBlob = nullptr;
    hr = D3DCompileFromFile(fileName, nullptr, nullptr, entryPoint, shaderModel,
        dwShaderFlags, 0, blobOut, &pErrorBlob);

    if (FAILED(hr)) {
        // Output if D3DCompileFromFile has error
        if (pErrorBlob) {
            OutputDebugStringA(reinterpret_cast<const char*>(pErrorBlob->GetBufferPointer()));
            pErrorBlob->Release();
        }

        return hr;
    }

    if (pErrorBlob) pErrorBlob->Release();

    return S_OK;
}