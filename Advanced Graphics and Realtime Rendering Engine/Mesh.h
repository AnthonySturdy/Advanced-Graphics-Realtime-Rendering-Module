#pragma once
#include <vector>
#include "Structures.h"

class Mesh {
public:
	Mesh(ID3D11Device* device, std::vector<SimpleVertex>& vertices, std::vector<WORD>& indices);
	~Mesh() = default;

	ID3D11Buffer* GetVertexBuffer() const { return m_vertexBuffer.Get();}
	ID3D11Buffer* GetIndexBuffer() const { return m_indexBuffer.Get(); }
	int GetNumIndices() const { return numIndices; }

private:
	HRESULT InitialiseMeshData(ID3D11Device* device, std::vector<SimpleVertex>& vertices, std::vector<WORD>& indices);

	void PopulateBinormalTangent(std::vector<SimpleVertex>& vertices, const std::vector<WORD>& indices);
	void CalculateTangentBinormal2(SimpleVertex v0, SimpleVertex v1, SimpleVertex v2, DirectX::XMFLOAT3& outNormal, DirectX::XMFLOAT3& outTangent, DirectX::XMFLOAT3& outBinormal);

	Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_indexBuffer;
	int numIndices;
};