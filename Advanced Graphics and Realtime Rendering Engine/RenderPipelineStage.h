#pragma once
class RenderPipelineStage {
private:
	static Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> m_unorderedAccessView;
	static Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_unorderedAccessSRV;

	void InitialiseUAV();

protected:
	Microsoft::WRL::ComPtr<ID3D11Device> device;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> context;
	DirectX::XMINT2	m_resolution;

	Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> GetUnorderedAccessView() {
		if (m_unorderedAccessView == nullptr)
			InitialiseUAV();
		
		return m_unorderedAccessView;
	}
	static void ResetUnorderedAccessView() { m_unorderedAccessView.Reset(); }

public:
	enum RENDER_PASS {
		SHADOW = 0,
		GEOMETRY,
		DEPTH_OF_FIELD,
		BLOOM,
		IMAGE_FILTER
	};

	RenderPipelineStage(Microsoft::WRL::ComPtr<ID3D11Device> _device, 
						Microsoft::WRL::ComPtr<ID3D11DeviceContext> _context,
						DirectX::XMINT2	resolution);
	~RenderPipelineStage() = default;

	virtual void Initialise() = 0;
	virtual void Render() = 0;

	const Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>& GetUavSrv() const { return m_unorderedAccessSRV; }
};