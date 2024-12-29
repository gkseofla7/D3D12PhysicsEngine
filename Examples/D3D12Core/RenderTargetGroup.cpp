#include "RenderTargetGroup.h"
#include "Engine.h"
#include "Device.h"
#include "CommandQueue.h"
namespace dengine {
void RenderTargetGroup::Create(RENDER_TARGET_GROUP_TYPE groupType, const vector<RenderTarget>& rtVec, const vector<shared_ptr<Texture>>& dsTextures)
{
	m_groupType = groupType;
	m_rtVec = rtVec;
	m_rtCount = static_cast<uint32>(rtVec.size());
	m_dsTextures = dsTextures;
	m_dsCount = dsTextures.size();

	if (m_rtCount > 0)
	{
		D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
		rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		rtvHeapDesc.NumDescriptors = m_rtCount;
		rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		rtvHeapDesc.NodeMask = 0;
		ThrowIfFailed(DEVICE->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap)));

		m_rtvHeapSize = DEVICE->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		m_rtvHeapBegin = m_rtvHeap->GetCPUDescriptorHandleForHeapStart();

		for (uint32 i = 0; i < m_rtCount; i++)
		{
			uint32 destSize = 1;
			D3D12_CPU_DESCRIPTOR_HANDLE destHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_rtvHeapBegin, i * m_rtvHeapSize);

			uint32 srcSize = 1;
			ComPtr<ID3D12DescriptorHeap> srcRtvHeapBegin = m_rtVec[i].target->GetRTV();
			D3D12_CPU_DESCRIPTOR_HANDLE srcHandle = srcRtvHeapBegin->GetCPUDescriptorHandleForHeapStart();

			DEVICE->CopyDescriptors(1, &destHandle, &destSize, 1, &srcHandle, &srcSize, D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		}

		// SRV
		D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
		srvHeapDesc.NumDescriptors = 1;
		srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		DEVICE->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&m_srvHeap));
	}

	if (m_dsCount > 0)
	{
		// DSV
		D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
		heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		heapDesc.NumDescriptors = m_dsCount;
		heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		heapDesc.NodeMask = 0;
		DEVICE->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_dsvHeap));

		m_dtvHeapSize = DEVICE->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
		m_dsvHeapBegin = m_dsvHeap->GetCPUDescriptorHandleForHeapStart();

		for (uint32 i = 0; i < m_dsCount; i++)
		{
			uint32 destSize = 1;
			D3D12_CPU_DESCRIPTOR_HANDLE destHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_dsvHeapBegin, i * m_dtvHeapSize);

			uint32 srcSize = 1;
			ComPtr<ID3D12DescriptorHeap> dsvHeapBegin = dsTextures[i]->GetDSV();
			D3D12_CPU_DESCRIPTOR_HANDLE srcHandle = dsvHeapBegin->GetCPUDescriptorHandleForHeapStart();

			DEVICE->CopyDescriptors(1, &destHandle, &destSize, 1, &srcHandle, &srcSize, D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
		}
	}
	

	for (int i = 0; i < m_rtCount; ++i)
	{
		m_targetToResource[i] = CD3DX12_RESOURCE_BARRIER::Transition(m_rtVec[i].target->GetTex2D().Get(),
			D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COMMON);

		m_resourceToTarget[i] = CD3DX12_RESOURCE_BARRIER::Transition(m_rtVec[i].target->GetTex2D().Get(),
			D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_RENDER_TARGET);
	}
}

void RenderTargetGroup::OMSetRenderTargets(uint32 count, uint32 offset)
{
	D3D12_VIEWPORT vp = D3D12_VIEWPORT{ 0.f, 0.f, m_rtVec[0].target->GetWidth() , m_rtVec[0].target->GetHeight(), 0.f, 1.f };
	D3D12_RECT rect = D3D12_RECT{ 0, 0, static_cast<LONG>(m_rtVec[0].target->GetWidth()),  static_cast<LONG>(m_rtVec[0].target->GetHeight()) };

	GRAPHICS_CMD_LIST->RSSetViewports(1, &vp);
	GRAPHICS_CMD_LIST->RSSetScissorRects(1, &rect);

	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_rtvHeapBegin, offset * m_rtvHeapSize);
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_dsvHeapBegin, offset * m_dtvHeapSize);
	GRAPHICS_CMD_LIST->OMSetRenderTargets(count, &rtvHandle, FALSE/*1개*/, &dsvHandle);
}

void RenderTargetGroup::OMSetRenderTargets()
{
	D3D12_VIEWPORT vp = D3D12_VIEWPORT{ 0.f, 0.f, m_rtVec[0].target->GetWidth() , m_rtVec[0].target->GetHeight(), 0.f, 1.f };
	D3D12_RECT rect = D3D12_RECT{ 0, 0, static_cast<LONG>(m_rtVec[0].target->GetWidth()),  static_cast<LONG>(m_rtVec[0].target->GetHeight()) };

	GRAPHICS_CMD_LIST->RSSetViewports(1, &vp);
	GRAPHICS_CMD_LIST->RSSetScissorRects(1, &rect);

	GRAPHICS_CMD_LIST->OMSetRenderTargets(m_rtCount, &m_rtvHeapBegin, TRUE/*다중*/, &m_dsvHeapBegin);
}

void RenderTargetGroup::OMSetOnlyDepthStencil(uint32 count, uint32 offset)
{
	assert(m_dsTextures.size() > offset);

	D3D12_VIEWPORT vp = D3D12_VIEWPORT{ 0.f, 0.f, m_dsTextures[offset]->GetWidth() , m_dsTextures[offset]->GetHeight(), 0.f, 1.f};
	D3D12_RECT rect = D3D12_RECT{ 0, 0, static_cast<LONG>(m_dsTextures[offset]->GetWidth()),  static_cast<LONG>(m_dsTextures[offset]->GetHeight())};

	GRAPHICS_CMD_LIST->RSSetViewports(1, &vp);
	GRAPHICS_CMD_LIST->RSSetScissorRects(1, &rect);

	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_dsvHeapBegin, offset * m_dtvHeapSize);
	GRAPHICS_CMD_LIST->OMSetRenderTargets(count, nullptr, TRUE/*다중*/, &dsvHandle);
}

void RenderTargetGroup::ClearRenderTargetView(uint32 index)
{
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_rtvHeapBegin, index * m_rtvHeapSize);
	GRAPHICS_CMD_LIST->ClearRenderTargetView(rtvHandle, m_rtVec[index].clearColor, 0, nullptr);
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_dsvHeapBegin, index * m_dtvHeapSize);
	GRAPHICS_CMD_LIST->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.f, 0, 0, nullptr);
}

void RenderTargetGroup::ClearRenderTargetView()
{
	WaitResourceToTarget();

	for (uint32 i = 0; i < m_rtCount; i++)
	{
		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_rtvHeapBegin, i * m_rtvHeapSize);
		GRAPHICS_CMD_LIST->ClearRenderTargetView(rtvHandle, m_rtVec[i].clearColor, 0, nullptr);
	}
	for (uint32 i = 0; i < m_dsCount; i++)
	{
		D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_dsvHeapBegin, i * m_dtvHeapSize);
		GRAPHICS_CMD_LIST->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.f, 0, 0, nullptr);
	}
}

shared_ptr<Texture> RenderTargetGroup::GetRTTexture(uint32 index)
{
	return m_rtVec[index].target; 
}
shared_ptr<Texture> RenderTargetGroup::GetDSTexture(uint32 index)
{
	return m_dsTextures[index]; 
}
void RenderTargetGroup::WaitTargetToResource()
{
	GRAPHICS_CMD_LIST->ResourceBarrier(m_rtCount, m_targetToResource);
}

void RenderTargetGroup::WaitTargetToResource(int index)
{
	GRAPHICS_CMD_LIST->ResourceBarrier(1, &m_targetToResource[index]);
}

void RenderTargetGroup::WaitResourceToTarget()
{
	GRAPHICS_CMD_LIST->ResourceBarrier(m_rtCount, m_resourceToTarget);
}

void RenderTargetGroup::WaitResourceToTarget(int index)
{
	GRAPHICS_CMD_LIST->ResourceBarrier(1,& m_resourceToTarget[index]);
}
}

