#pragma once
#include "EnginePch.h"
#include "Texture.h"

namespace hlab {

struct RenderTarget
{
	shared_ptr<Texture> target;
	float clearColor[4];
};

class RenderTargetGroup
{
public:
	void Create(RENDER_TARGET_GROUP_TYPE groupType, vector<RenderTarget>& rtVec, shared_ptr<Texture> dsTexture);

	void OMSetRenderTargets(uint32 count, uint32 offset);
	void OMSetRenderTargets();

	void ClearRenderTargetView(uint32 index);
	void ClearRenderTargetView();

	ComPtr<ID3D12DescriptorHeap> GetRenderTargetHeap() { return m_rtvHeap; }

	shared_ptr<Texture> GetRTTexture(uint32 index) { return m_rtVec[index].target; }
	shared_ptr<Texture> GetDSTexture() { return m_dsTexture; }

	void WaitTargetToResource();
	void WaitResourceToTarget();

private:
	RENDER_TARGET_GROUP_TYPE		m_groupType;
	vector<RenderTarget>			m_rtVec;
	uint32							m_rtCount;
	shared_ptr<Texture>				m_dsTexture;
	ComPtr<ID3D12DescriptorHeap>	m_rtvHeap;

private:
	uint32							m_rtvHeapSize;
	D3D12_CPU_DESCRIPTOR_HANDLE		m_rtvHeapBegin;
	D3D12_CPU_DESCRIPTOR_HANDLE		m_dsvHeapBegin;

private:
	D3D12_RESOURCE_BARRIER			m_targetToResource[8];
	D3D12_RESOURCE_BARRIER			m_resourceToTarget[8];
};


}
