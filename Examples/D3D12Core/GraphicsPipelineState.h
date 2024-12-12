#pragma once
#include "EnginePch.h"
namespace hlab {
class GraphicsPipelineState
{
public:
	void Init();

	ComPtr<ID3D12PipelineState> GetDefaultPipelineState() { return m_defaultPipelineState; }
	ComPtr<ID3D12PipelineState> GetSkyboxPipelineState() { return m_skyboxPipelineState; }
private:
	ComPtr<ID3D12PipelineState> m_defaultPipelineState;
	ComPtr<ID3D12PipelineState> m_skyboxPipelineState;
};


}
