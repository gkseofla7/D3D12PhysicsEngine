#pragma once
#include "EnginePch.h"
namespace hlab {
class GraphicsPipelineState
{
public:
	void Init();

	ComPtr<ID3D12PipelineState> GetDefaultPipelineState() { return m_defaultPipelineState; }
private:
	ComPtr<ID3D12PipelineState> m_defaultPipelineState;
};


}
