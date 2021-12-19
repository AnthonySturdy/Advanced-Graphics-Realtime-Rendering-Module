#pragma once
class RenderPipelineStage {
private:
	static Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> m_postProcUnorderedAccessView;
	static Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_unorderedAccessSRV;

	void TryInitialiseUAV();

protected:
	Microsoft::WRL::ComPtr<ID3D11Device> device;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> context;
	DirectX::XMINT2	m_resolution;

	Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> GetUnorderedAccessView() {
		TryInitialiseUAV();
		return m_postProcUnorderedAccessView;
	}

public:
	RenderPipelineStage(Microsoft::WRL::ComPtr<ID3D11Device> _device, 
						Microsoft::WRL::ComPtr<ID3D11DeviceContext> _context,
						DirectX::XMINT2	resolution);
	~RenderPipelineStage() = default;

	virtual void Initialise() = 0;
	virtual void Render() = 0;
};