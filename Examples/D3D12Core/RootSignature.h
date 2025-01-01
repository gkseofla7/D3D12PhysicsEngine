#pragma once
#include "EnginePch.h"

namespace dengine {
// TODO. 나중엔 Pooling으로 Desc를 키로 사용해서 변경 필요
class RootSignature
{
public:
	void Init();

	ComPtr<ID3D12RootSignature>	GetGraphicsRootSignature() { return m_defaultRootSignature; }
	ComPtr<ID3D12RootSignature>	GetSkinnedRootSignature() { return m_skinnedRootSignature; }
	ComPtr<ID3D12RootSignature>	GetSkyboxRootSignature() { return m_skyboxRootSignature; }
	ComPtr<ID3D12RootSignature>	GetSamplingRootSignature() { return m_samplinigRootSignature; }
	ComPtr<ID3D12RootSignature>	GetShadowRootSignature() { return m_shadowRootSignature; }

private:
	void CreateDefaultRootSignature();
	//void CreateSkinnedRootSignature();
	void CreateSkyboxRootSignature();
	void CreateSamplingRootSignature();
	void CreateShadowRootSignature();
	void CreateRootSignature(ComPtr<ID3D12RootSignature>& rootSignature, vector<CD3DX12_ROOT_PARAMETER1>& rootParams);
private:
	ComPtr<ID3D12RootSignature> m_defaultRootSignature;
	ComPtr<ID3D12RootSignature> m_skinnedRootSignature;
	ComPtr<ID3D12RootSignature> m_skyboxRootSignature;
	ComPtr<ID3D12RootSignature> m_samplinigRootSignature;
	ComPtr<ID3D12RootSignature> m_shadowRootSignature;
};

}

