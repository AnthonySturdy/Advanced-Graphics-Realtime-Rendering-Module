#include "pch.h"
#include "RenderPipelineStage.h"

RenderPipelineStage::RenderPipelineStage(Microsoft::WRL::ComPtr<ID3D11Device> _device,
										Microsoft::WRL::ComPtr<ID3D11DeviceContext> _context)
	: device(_device), context(_context) {}
