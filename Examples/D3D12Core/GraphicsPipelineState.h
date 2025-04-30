#pragma once
#include "EnginePch.h"
namespace dengine {
class GraphicsPipelineState
{
public:
	void Init();

	ComPtr<ID3D12PipelineState> GetDefaultPipelineState() { return m_defaultPipelineState; }
	ComPtr<ID3D12PipelineState> GetSkinnedPipelineState() { return m_skinnedPipelineState; }
	ComPtr<ID3D12PipelineState> GetSkyboxPipelineState() { return m_skyboxPipelineState; }
	ComPtr<ID3D12PipelineState> GetPostEffectPipelineState() { return m_postEffectPipelineState; }
	ComPtr<ID3D12PipelineState> GetShadowPipelineState() { return m_shadowPipelineState; }
	ComPtr<ID3D12PipelineState> GetShadowSkinnedPipelineState() { return m_shadowSkinnedPipelineState; }
private:
	ComPtr<ID3D12PipelineState> m_defaultPipelineState;
	ComPtr<ID3D12PipelineState> m_skinnedPipelineState;
	ComPtr<ID3D12PipelineState> m_billboardPipelineState;
	ComPtr<ID3D12PipelineState> m_skyboxPipelineState;
	ComPtr<ID3D12PipelineState> m_postEffectPipelineState;
	ComPtr<ID3D12PipelineState> m_shadowPipelineState;
	ComPtr<ID3D12PipelineState> m_shadowSkinnedPipelineState;
};


}
