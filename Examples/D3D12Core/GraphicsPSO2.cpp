#include "GraphicsPSO2.h"
#include "Engine.h"
#include "CommandQueue.h"

namespace dengine {
void GraphicsPSO::Init(ComPtr<ID3D12RootSignature>	rootSignature, ComPtr<ID3D12PipelineState> pipelineState, PSOType psoType)
{
	m_rootSignature = rootSignature;
	m_pipelineState = pipelineState;

	m_psoType = psoType;
}

void GraphicsPSO::UploadGraphicsPSO()
{
	GRAPHICS_CMD_LIST->SetGraphicsRootSignature(m_rootSignature.Get());

	GRAPHICS_CMD_LIST->IASetPrimitiveTopology(m_topology);
	GRAPHICS_CMD_LIST->SetPipelineState(m_pipelineState.Get());

	GEngine->SetPSOType(m_psoType);
	GEngine->CommintGlobalData();
}
} // namespace dengine