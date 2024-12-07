#pragma once
#include "EnginePch.h"
class SwapChain;
class DescriptorHeap;

namespace hlab {
class GraphicsCommandQueue
{
public:
	~GraphicsCommandQueue();

	void Init(ComPtr<ID3D12Device> device, shared_ptr<SwapChain> swapChain);
	void WaitSync();

	void RenderBegin();
	void RenderEnd();

	void FlushResourceCommandQueue();

	ComPtr<ID3D12CommandQueue> GetCmdQueue() { return m_cmdQueue; }
	ComPtr<ID3D12GraphicsCommandList> GetGraphicsCmdList() { return	m_cmdList; }
	ComPtr<ID3D12GraphicsCommandList> GetResourceCmdList() { return	m_resCmdList; }

private:
	ComPtr<ID3D12CommandQueue>			m_cmdQueue;
	ComPtr<ID3D12CommandAllocator>		m_cmdAlloc;
	ComPtr<ID3D12GraphicsCommandList>	m_cmdList;

	ComPtr<ID3D12CommandAllocator>		m_resCmdAlloc;
	ComPtr<ID3D12GraphicsCommandList>	m_resCmdList;

	ComPtr<ID3D12Fence>					m_fence;
	uint32								m_fenceValue = 0;
	HANDLE								m_fenceEvent = INVALID_HANDLE_VALUE;

	shared_ptr<SwapChain>		m_swapChain;
};

}
