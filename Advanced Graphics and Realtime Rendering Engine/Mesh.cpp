#include "pch.h"
#include "Mesh.h"

using namespace DirectX;

Mesh::Mesh(ID3D11Device* device, std::vector<SimpleVertex>& vertices, std::vector<WORD>& indices) {
	InitialiseMeshData(device, vertices, indices);
}

HRESULT Mesh::InitialiseMeshData(ID3D11Device* device, std::vector<SimpleVertex>& vertices, std::vector<WORD>& indices) {
	numIndices = indices.size();

	// Populate tangent and bitangent in vertices
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

	// Create index buffer
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(WORD) * indices.size();        // 36 vertices needed for 12 triangles in a triangle list
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;
	InitData.pSysMem = &indices[0];
	hr = device->CreateBuffer(&bd, &InitData, &m_indexBuffer);
	if (FAILED(hr))
		return hr;

	return S_OK;
}

/***********************************************
MARKING SCHEME: Normal Mapping, Parallax Mapping
DESCRIPTION: Calculate tangent and binormal for each face/triangle
***********************************************/
void Mesh::PopulateBinormalTangent(std::vector<SimpleVertex>& vertices, const std::vector<WORD>& indices) {
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

void Mesh::CalculateTangentBinormal2(SimpleVertex v0, SimpleVertex v1, SimpleVertex v2, XMFLOAT3& outNormal, XMFLOAT3& outTangent, XMFLOAT3& outBinormal) {
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
