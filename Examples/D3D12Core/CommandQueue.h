#pragma once
#include "EnginePch.h"
#include <queue>
#include <mutex>
namespace dengine {
class SwapChain;
class DescriptorHeap;
struct ResourceCommandList
{
	ComPtr<ID3D12CommandAllocator>		m_resCmdAlloc;
	ComPtr<ID3D12GraphicsCommandList>	m_resCmdList;
};
class GraphicsCommandQueue
{
public:
	~GraphicsCommandQueue();

	void Init(ComPtr<ID3D12Device> device);
	void WaitSync();

	void RenderBegin();
	void RenderEnd();

	void FlushResourceCommandQueue(ResourceCommandList& rscCommandList);

	void SetSwapChain(shared_ptr<SwapChain>	swapChain) { m_swapChain = swapChain; }

	ComPtr<ID3D12CommandQueue> GetCmdQueue() { return m_cmdQueue; }
	ComPtr<ID3D12GraphicsCommandList> GetGraphicsCmdList() { return	m_cmdList; }
	ResourceCommandList GetResourceCmdList();
private:
	ComPtr<ID3D12CommandQueue>			m_cmdQueue;
	ComPtr<ID3D12CommandAllocator>		m_cmdAlloc;
	ComPtr<ID3D12GraphicsCommandList>	m_cmdList;

	// 리소스 관련 commandList를 여러개 만들고 Pooling
	std::queue<ResourceCommandList> m_resCmdLists;
	std::mutex m_rscMutex;
	std::condition_variable m_rscCv;

	ComPtr<ID3D12Fence>					m_fence;
	uint32								m_fenceValue = 0;
	HANDLE								m_fenceEvent = INVALID_HANDLE_VALUE;

	shared_ptr<SwapChain>		m_swapChain;
};

}
