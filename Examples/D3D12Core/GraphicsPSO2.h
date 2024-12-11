#pragma once

#include "EnginePch.h"

namespace hlab {

class GraphicsPSO2 {
public:
    void Init(ComPtr<ID3D12RootSignature>	rootSignature, ComPtr<ID3D12PipelineState> pipelineState);

    void UploadGraphicsPSO();
private:
    ComPtr<ID3D12RootSignature>	m_rootSignature;
    ComPtr<ID3D12PipelineState> m_pipelineState;
    D3D_PRIMITIVE_TOPOLOGY m_topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    //D3D11_PRIMITIVE_TOPOLOGY m_primitiveTopology =
    //    D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
};

} // namespace hlab