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
	// Create vertices and indices
	// TODO: Move this into a mesh class - import from model file?
	std::vector<SimpleVertex> vertices =
	{
		{ XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(1.0f, 1.0f) },

		{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT2(1.0f, 1.0f) },
		{ XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(1.0f, 1.0f) },
		{ XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },

		{ XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(1.0f, 1.0f) },
		{ XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(1.0f, 0.0f) },

		{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f) , XMFLOAT2(1.0f, 1.0f) },
		{ XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },

		{ XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 1.0f) },
		{ XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 0.0f) },
	};

	std::vector<WORD> indices =
	{
		3,1,0,
		2,1,3,

		6,4,5,
		7,4,6,

		11,9,8,
		10,9,11,

		14,12,13,
		15,12,14,

		19,17,16,
		18,17,19,

		22,20,21,
		23,20,22
	};

	PopulateBinormalTangent(vertices, indices);
	
	// Create vertex buffer
	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(SimpleVertex) * vertices.size();
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA InitData = {};
	InitData.pSysMem = &vertices[0];
	HRESULT hr = device->CreateBuffer(&bd, &InitData, &m_vertexBuffer);
	if (FAILED(hr))
		return hr;

	// Set vertex buffer
	UINT stride = sizeof(SimpleVertex);
	UINT offset = 0;
	context->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);

	// Create index buffer
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(WORD) * indices.size();        // 36 vertices needed for 12 triangles in a triangle list
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;
	InitData.pSysMem = &indices[0];
	hr = device->CreateBuffer(&bd, &InitData, &m_indexBuffer);
	if (FAILED(hr))
		return hr;

	// Set index buffer
	context->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);

	// Set primitive topology
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// load and setup textures
	hr = CreateDDSTextureFromFile(device, L"Resources\\stone.dds", nullptr, &m_textureResourceView);
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

	// Create the material constant buffer
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
	context->PSSetShaderResources(0, 1, m_textureResourceView.GetAddressOf());
	context->PSSetShaderResources(1, 1, m_normalResourceView.GetAddressOf());
	context->PSSetSamplers(0, 1, m_samplerLinear.GetAddressOf());
	
	context->DrawIndexed(NUM_INDICES, 0, 0);
}

void GameObject::RenderGUIControls() {
	if (ImGui::CollapsingHeader("GameObject Controls")) {
		ImGui::DragFloat3("Position", &m_position.x, 0.01f);
		ImGui::DragFloat3("Rotation", &m_rotation.x, 0.01f);
		ImGui::DragFloat3("Scale", &m_scale.x, 0.01f);
	}
}

void GameObject::PopulateBinormalTangent(std::vector<SimpleVertex>& vertices, std::vector<WORD>& indices) {
	const int faceCount = indices.size() / 3;

	for (int i = 0; i < faceCount; ++i) {
		const int baseIndex = i * 3;

		XMFLOAT3 normal, tangent, binormal;
		CalculateTangentBinormal2(vertices[indices[baseIndex + 0]], 
									vertices[indices[baseIndex + 1]], 
									vertices[indices[baseIndex + 2]], 
									normal, tangent, binormal);

		vertices[indices[baseIndex + 0]].Tangent = tangent;
		vertices[indices[baseIndex + 1]].Tangent = tangent;
		vertices[indices[baseIndex + 2]].Tangent = tangent;

		vertices[indices[baseIndex + 0]].BiTangent = binormal;
		vertices[indices[baseIndex + 1]].BiTangent = binormal;
		vertices[indices[baseIndex + 2]].BiTangent = binormal;
	}
}

void GameObject::CalculateTangentBinormal2(SimpleVertex v0, SimpleVertex v1, SimpleVertex v2, XMFLOAT3& outNormal, XMFLOAT3& outTangent, XMFLOAT3& outBinormal) {
	// http://softimage.wiki.softimage.com/xsidocs/tex_tangents_binormals_AboutTangentsandBinormals.html

	// 1. CALCULATE THE NORMAL
	XMVECTOR vv0 = XMLoadFloat3(&v0.Pos);
	XMVECTOR vv1 = XMLoadFloat3(&v1.Pos);
	XMVECTOR vv2 = XMLoadFloat3(&v2.Pos);

	XMVECTOR P = vv1 - vv0;
	XMVECTOR Q = vv2 - vv0;

	XMVECTOR e01cross = XMVector3Cross(P, Q);
	XMFLOAT3 normalOut;
	XMStoreFloat3(&normalOut, e01cross);
	outNormal = normalOut;

	// 2. CALCULATE THE TANGENT from texture space
	float s1 = v1.TexCoord.x - v0.TexCoord.x;
	float t1 = v1.TexCoord.y - v0.TexCoord.y;
	float s2 = v2.TexCoord.x - v0.TexCoord.x;
	float t2 = v2.TexCoord.y - v0.TexCoord.y;

	float tmp = 0.0f;
	if (fabsf(s1 * t2 - s2 * t1) <= 0.0001f) {
		tmp = 1.0f;
	} else {
		tmp = 1.0f / (s1 * t2 - s2 * t1);
	}

	XMFLOAT3 PF3, QF3;
	XMStoreFloat3(&PF3, P);
	XMStoreFloat3(&QF3, Q);

	outTangent.x = (t2 * PF3.x - t1 * QF3.x);
	outTangent.y = (t2 * PF3.y - t1 * QF3.y);
	outTangent.z = (t2 * PF3.z - t1 * QF3.z);

	outTangent.x = outTangent.x * tmp;
	outTangent.y = outTangent.y * tmp;
	outTangent.z = outTangent.z * tmp;

	XMVECTOR vn = XMLoadFloat3(&outNormal);
	XMVECTOR vt = XMLoadFloat3(&outTangent);

	// 3. CALCULATE THE BINORMAL
	// left hand system b = t cross n (rh would be b = n cross t)
	XMVECTOR vb = XMVector3Cross(vt, vn);

	vn = XMVector3Normalize(vn);
	vt = XMVector3Normalize(vt);
	vb = XMVector3Normalize(vb);

	XMStoreFloat3(&outNormal, vn);
	XMStoreFloat3(&outTangent, vt);
	XMStoreFloat3(&outBinormal, vb);
}