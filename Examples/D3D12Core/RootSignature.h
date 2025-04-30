#pragma once
#include "EnginePch.h"

namespace dengine {
// TODO. ���߿� Pooling���� Desc�� Ű�� ����ؼ� ���� �ʿ�
class RootSignature
{
public:
	void Init();

	ComPtr<ID3D12RootSignature>	GetGraphicsRootSignature() { return m_defaultRootSignature; }
	ComPtr<ID3D12RootSignature>	GetSkinnedRootSignature() { return m_skinnedRootSignature; }
	ComPtr<ID3D12RootSignature>	GetBillboardRootSignature() { return m_billboardRootSignature; }
	ComPtr<ID3D12RootSignature>	GetSkyboxRootSignature() { return m_skyboxRootSignature; }
	ComPtr<ID3D12RootSignature>	GetSamplingRootSignature() { return m_samplinigRootSignature; }
	ComPtr<ID3D12RootSignature>	GetShadowRootSignature() { return m_shadowRootSignature; }

private:
	void CreateDefaultRootSignature();
	void CreateBillboardRootSignature();
	//void CreateSkinnedRootSignature();
	void CreateSkyboxRootSignature();
	void CreateSamplingRootSignature();
	void CreateShadowRootSignature();
	void CreateRootSignature(ComPtr<ID3D12RootSignature>& rootSignature, vector<CD3DX12_ROOT_PARAMETER1>& rootParams);
private:
	ComPtr<ID3D12RootSignature> m_defaultRootSignature;
	ComPtr<ID3D12RootSignature> m_skinnedRootSignature;
	ComPtr<ID3D12RootSignature> m_billboardRootSignature;
	ComPtr<ID3D12RootSignature> m_skyboxRootSignature;
	ComPtr<ID3D12RootSignature> m_samplinigRootSignature;
	ComPtr<ID3D12RootSignature> m_shadowRootSignature;
};

}

