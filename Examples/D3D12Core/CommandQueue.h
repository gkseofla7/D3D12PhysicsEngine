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
	void WaitSync(uint64 fenceValue);
	void WaitSyncGPU(uint64 fenceValue);
	void WaitFrameSync(int frameIndex);
	void WaitFrameSyncGPU(int frameIndex);
	void WaitGPUResourceSync();
	void FenceFrame(int index);
	uint64 Fence();

	void RenderBegin();
	void RenderEnd();
	void FlushResourceCommandQueue(ResourceCommandList& rscCommandList, bool bBlocking = false);

	void SetSwapChain(shared_ptr<SwapChain>	swapChain) { m_swapChain = swapChain; }

	ComPtr<ID3D12CommandQueue> GetCmdQueue() { return m_cmdQueue; }
	ComPtr<ID3D12GraphicsCommandList> GetGraphicsCmdList(int index) { return m_cmdList[index]; }
	ComPtr<ID3D12GraphicsCommandList> GetCurrentGraphicsCmdList();
	ResourceCommandList GetResourceCmdList();
private:
	ComPtr<ID3D12CommandQueue>			m_cmdQueue;
	ComPtr<ID3D12CommandAllocator>		m_cmdAlloc[SWAP_CHAIN_BUFFER_COUNT];
	ComPtr<ID3D12GraphicsCommandList>	m_cmdList[SWAP_CHAIN_BUFFER_COUNT];
	uint64	m_lastFenceValue[SWAP_CHAIN_BUFFER_COUNT] = {};

	// 리소스 관련 commandList를 여러개 만들고 Pooling
	std::queue<ResourceCommandList> m_resCmdLists;
	std::mutex m_rscMutex;
	std::condition_variable m_rscCv;

	ComPtr<ID3D12Fence>					m_fence;
	std::atomic<uint64>					m_fenceValue = 0;
	HANDLE								m_fenceEvent = INVALID_HANDLE_VALUE;

	std::mutex m_fenceMutex;

	std::atomic<uint64> m_lastResUploadFenceValue = 0;

	shared_ptr<SwapChain>		m_swapChain;
};

}
