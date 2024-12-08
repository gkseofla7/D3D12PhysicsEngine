#include "GraphicsPSO.h"
#include "Engine.h"
#include "CommandQueue.h"

namespace hlab {
void GraphicsPSO::Init(ComPtr<ID3D12RootSignature>	rootSignature, ComPtr<ID3D12PipelineState> pipelineState)
{
	m_rootSignature = rootSignature;
	m_pipelineState = pipelineState;
}

void GraphicsPSO::UploadGraphicsPSO()
{
	// TODO. 이것만 하면될지 확인 필요
	GRAPHICS_CMD_LIST->SetGraphicsRootSignature(m_rootSignature.Get());

	GRAPHICS_CMD_LIST->IASetPrimitiveTopology(m_topology);
	GRAPHICS_CMD_LIST->SetPipelineState(m_pipelineState.Get());
}
} // namespace hlab