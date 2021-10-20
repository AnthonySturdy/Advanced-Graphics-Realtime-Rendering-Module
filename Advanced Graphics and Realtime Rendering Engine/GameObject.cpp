#include "pch.h"
#include "GameObject.h"

using namespace DirectX;

#define NUM_INDICES 36

GameObject::GameObject() : m_position(XMFLOAT3()), m_rotation(XMFLOAT3()), m_scale(XMFLOAT3(1, 1, 1)), m_world(XMFLOAT4X4()) {
	// Initialise world matrix
	XMStoreFloat4x4(&m_world, XMMatrixIdentity());
}

GameObject::~GameObject() {
}

HRESULT GameObject::InitMesh(ID3D11Device* device, ID3D11DeviceContext* context) {
	m_mesh = std::make_shared<Mesh>(device, context);

	// Set primitive topology
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// load and setup textures
	HRESULT hr = CreateDDSTextureFromFile(device, L"Resources\\stone.dds", nullptr, &m_textureResourceView);
	if (FAILED(hr))
		return hr;
	
	hr = CreateDDSTextureFromFile(device, L"Resources\\conenormal.dds", nullptr, &m_normalResourceView);
	if (FAILED(hr))
		return hr;

	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	hr = device->CreateSamplerState(&sampDesc, m_samplerLinear.GetAddressOf());
	if (FAILED(hr))
		return hr;

	m_material.Material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_material.Material.Specular = XMFLOAT4(1.0f, 0.2f, 0.2f, 1.0f);
	m_material.Material.SpecularPower = 32.0f;
	m_material.Material.UseTexture = true;
	m_material.Material.UseNormal = true;

	// Create the material constant buffer
	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(MaterialPropertiesConstantBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	hr = device->CreateBuffer(&bd, nullptr, &m_materialConstantBuffer);
	if (FAILED(hr))
		return hr;

	return hr;
}

void GameObject::InitShader(ID3D11Device* device, const WCHAR* vertexShaderPathWithoutExt, const WCHAR* pixelShaderPathWithoutExt, D3D11_INPUT_ELEMENT_DESC* vertexLayout, UINT numElements) {
	m_shader = std::make_shared<Shader>(device, vertexShaderPathWithoutExt, pixelShaderPathWithoutExt, vertexLayout, numElements);
}

void GameObject::Update(float t, ID3D11DeviceContext* context) {
	static float cummulativeTime = 0;
	cummulativeTime += t;

	XMMATRIX scale = XMMatrixScalingFromVector(XMLoadFloat3(&m_scale));
	XMMATRIX rotation = XMMatrixRotationRollPitchYawFromVector(XMLoadFloat3(&m_rotation));
	XMMATRIX translation = XMMatrixTranslationFromVector(XMLoadFloat3(&m_position));
	
	XMMATRIX world = scale * rotation * translation;
	XMStoreFloat4x4(&m_world, world);

	context->UpdateSubresource(m_materialConstantBuffer.Get(), 0, nullptr, &m_material, 0, 0);
}

void GameObject::Render(ID3D11DeviceContext* context) {
	ID3D11Buffer* indexBuf = m_mesh->GetVertexBuffer();
	UINT stride = sizeof(SimpleVertex);
	UINT offset = 0;
	context->IASetVertexBuffers(0, 1, &indexBuf, &stride, &offset);
	context->IASetIndexBuffer(m_mesh->GetIndexBuffer(), DXGI_FORMAT_R16_UINT, 0);

	context->PSSetShaderResources(0, 1, m_textureResourceView.GetAddressOf());
	context->PSSetShaderResources(1, 1, m_normalResourceView.GetAddressOf());
	context->PSSetSamplers(0, 1, m_samplerLinear.GetAddressOf());
	
	context->DrawIndexed(NUM_INDICES, 0, 0);
}

void GameObject::RenderGUIControls() {
	if (ImGui::CollapsingHeader("GameObject Controls")) {
		ImGui::Text("Transform");
		ImGui::DragFloat3("Position", &m_position.x, 0.01f);
		ImGui::DragFloat3("Rotation", &m_rotation.x, 0.01f);
		ImGui::DragFloat3("Scale", &m_scale.x, 0.01f);
		ImGui::Text("Material");
		ImGui::DragFloat3("Emissive", &m_material.Material.Emissive.x, 0.001f);
		ImGui::DragFloat3("Ambient", &m_material.Material.Ambient.x, 0.001f);
		ImGui::DragFloat3("Diffuse", &m_material.Material.Diffuse.x, 0.001f);
		ImGui::DragFloat3("Specular", &m_material.Material.Specular.x, 0.001f);
		ImGui::DragFloat("SpecularPow", &m_material.Material.SpecularPower, 0.1f);
		ImGui::Checkbox("Diffuse Texture", (bool*)&m_material.Material.UseTexture);
		ImGui::Checkbox("Normal Texture", (bool*)&m_material.Material.UseNormal);
	}
}