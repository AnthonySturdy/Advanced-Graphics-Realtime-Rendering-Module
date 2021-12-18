#pragma once
class RenderPipelineStage {
protected:
	Microsoft::WRL::ComPtr<ID3D11Device> device;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> context;

public:
	RenderPipelineStage(Microsoft::WRL::ComPtr<ID3D11Device> _device, 
						Microsoft::WRL::ComPtr<ID3D11DeviceContext> _context);
	~RenderPipelineStage() = default;

	virtual void Initialise() = 0;
	virtual void Render() = 0;
};