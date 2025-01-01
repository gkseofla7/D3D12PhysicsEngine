#pragma once
#include "EnginePch.h"
#include "Texture.h"

namespace dengine {

struct RenderTarget
{
	shared_ptr<Texture> target;
	float clearColor[4];
};

class RenderTargetGroup
{
public:
	void Create(RENDER_TARGET_GROUP_TYPE groupType, const vector<RenderTarget>& rtVec, const vector<shared_ptr<Texture>>& dsTexture);

	void OMSetRenderTargets(uint32 count, uint32 offset);
	void OMSetRenderTargets();
	void OMSetOnlyDepthStencil(uint32 count, uint32 offset);

	void ClearRenderTargetView(uint32 index);
	void ClearRenderTargetView();

	ComPtr<ID3D12DescriptorHeap> GetRenderTargetHeap() { return m_rtvHeap; }
	ComPtr<ID3D12DescriptorHeap> GetShaderResourceHeap() { return m_srvHeap; }
	ComPtr<ID3D12DescriptorHeap> GetDepthStencilViewHeap() { return m_dsvHeap; }

	shared_ptr<Texture> GetRTTexture(uint32 index);
	shared_ptr<Texture> GetDSTexture(uint32 index);

	void WaitTargetToResource();
	void WaitTargetToResource(int index);
	void WaitResourceToTarget();
	void WaitResourceToTarget(int index);

private:
	RENDER_TARGET_GROUP_TYPE		m_groupType;
	
	vector<RenderTarget>			m_rtVec;
	uint32							m_rtCount;

	vector<shared_ptr<Texture>>		m_dsTextures;
	uint32							m_dsCount;
	
	ComPtr<ID3D12DescriptorHeap>	m_rtvHeap;
	ComPtr<ID3D12DescriptorHeap>	m_srvHeap;
	ComPtr<ID3D12DescriptorHeap>	m_dsvHeap;
private:
	uint32							m_rtvHeapSize;
	uint32							m_dtvHeapSize;
	uint32							m_srvHeapSize;
	D3D12_CPU_DESCRIPTOR_HANDLE		m_rtvHeapBegin;
	D3D12_CPU_DESCRIPTOR_HANDLE		m_dsvHeapBegin;
	D3D12_CPU_DESCRIPTOR_HANDLE		m_srvHeapBegin;

private:
	D3D12_RESOURCE_BARRIER			m_targetToResource[8];
	D3D12_RESOURCE_BARRIER			m_resourceToTarget[8];
};


}
