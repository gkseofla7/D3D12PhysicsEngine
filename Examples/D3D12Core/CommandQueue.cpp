#include "CommandQueue.h"
#include "SwapChain.h"
#include "RenderTargetGroup.h"
#include "Engine.h"
#include "Samplers2.h"

// ************************
// GraphicsCommandQueue
// ************************
namespace dengine {
GraphicsCommandQueue::~GraphicsCommandQueue()
{
	::CloseHandle(m_fenceEvent);
}

void GraphicsCommandQueue::Init(ComPtr<ID3D12Device> device)
{
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

	device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_cmdQueue));
	
	for (int i = 0; i < SWAP_CHAIN_BUFFER_COUNT; i++)
	{
		device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_cmdAlloc[i]));
		device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_cmdAlloc[i].Get(), nullptr, IID_PPV_ARGS(&m_cmdList[i]));
		m_cmdList[i]->Close();
	}


	for (int i = 0; i < 5; i++)
	{
		ResourceCommandList ResCommandList;
		device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&ResCommandList.m_resCmdAlloc));
		device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, ResCommandList.m_resCmdAlloc.Get(), nullptr, IID_PPV_ARGS(&ResCommandList.m_resCmdList));
		m_resCmdLists.push(ResCommandList);
	}

	// CreateFence
	// - CPU와 GPU의 동기화 수단으로 쓰인다
	device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence));
	m_fenceEvent = ::CreateEvent(nullptr, FALSE, FALSE, nullptr);
}
ComPtr<ID3D12GraphicsCommandList> GraphicsCommandQueue::GetCurrentGraphicsCmdList()
{
	return m_cmdList[BACKBUFFER_INDEX];
}
void GraphicsCommandQueue::WaitSync()
{
	static thread_local HANDLE threadFenceEvent = ::CreateEvent(nullptr, FALSE, FALSE, nullptr);

	uint64 fenceValue = Fence();

	// Wait until the GPU has completed commands up to this fence point.
	if (m_fence->GetCompletedValue() < m_fenceValue)
	{
		// Fire event when GPU hits current fence.  
		m_fence->SetEventOnCompletion(m_fenceValue, threadFenceEvent);

		// Wait until the GPU hits current fence event is fired.
		::WaitForSingleObject(threadFenceEvent, INFINITE);
	}
}

void GraphicsCommandQueue::WaitFrameSync(int frameIndex)
{
	// Wait until the GPU has completed commands up to this fence point.
	if (m_fence->GetCompletedValue() < m_lastFenceValue[frameIndex])
	{
		// Fire event when GPU hits current fence.  
		m_fence->SetEventOnCompletion(m_fenceValue, m_fenceEvent);

		// Wait until the GPU hits current fence event is fired.
		::WaitForSingleObject(m_fenceEvent, INFINITE);
	}
}

void GraphicsCommandQueue::FenceFrame(int index)
{
	m_lastFenceValue[index] = Fence();
}

uint64 GraphicsCommandQueue::Fence()
{
	std::lock_guard<std::mutex> lock(m_fenceMutex);
	// Advance the fence value to mark commands up to this fence point.
	m_fenceValue++;

	// Add an instruction to the command queue to set a new fence point.  Because we 
	// are on the GPU timeline, the new fence point won't be set until the GPU finishes
	// processing all the commands prior to this Signal().
	m_cmdQueue->Signal(m_fence.Get(), m_fenceValue);
	return m_fenceValue;
}

void GraphicsCommandQueue::RenderBegin()
{
	const int8 backIndex = BACKBUFFER_INDEX;

	m_cmdAlloc[backIndex]->Reset();
	m_cmdList[backIndex]->Reset(m_cmdAlloc[backIndex].Get(), nullptr);

	D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
		GEngine->GetRTGroup(RENDER_TARGET_GROUP_TYPE::SWAP_CHAIN)->GetRTTexture(0)->GetTex2D().Get(),
		D3D12_RESOURCE_STATE_PRESENT, // 화면 출력
		D3D12_RESOURCE_STATE_RENDER_TARGET); // 외주 결과물
	m_cmdList[backIndex]->ResourceBarrier(1, &barrier);
	GEngine->GetRTGroup(RENDER_TARGET_GROUP_TYPE::FLOAT)->WaitResourceToTarget();
	GEngine->GetRTGroup(RENDER_TARGET_GROUP_TYPE::FLOAT)->ClearRenderTargetView();
	GEngine->GetGraphicsDescHeap()->Clear();

	ID3D12DescriptorHeap* descHeap[2];
	descHeap[0] = GEngine->GetGraphicsDescHeap()->GetDescriptorHeap().Get();
	descHeap[1] = GEngine->GetSamples()->GetDescHeap().Get();
	GRAPHICS_CMD_LIST->SetDescriptorHeaps(2, descHeap);
}

void GraphicsCommandQueue::RenderEnd()
{
	int8 backIndex = m_swapChain->GetBackBufferIndex();

	D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
		GEngine->GetRTGroup(RENDER_TARGET_GROUP_TYPE::SWAP_CHAIN)->GetRTTexture(0)->GetTex2D().Get(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, // 외주 결과물
		D3D12_RESOURCE_STATE_PRESENT); // 화면 출력

	m_cmdList[backIndex]->ResourceBarrier(1, &barrier);
	m_cmdList[backIndex]->Close();

	// 커맨드 리스트 수행
	ID3D12CommandList* cmdListArr[] = { m_cmdList[backIndex].Get()};
	m_cmdQueue->ExecuteCommandLists(_countof(cmdListArr), cmdListArr);

	m_swapChain->Present();
	FenceFrame(backIndex);
	//WaitSync();

	m_swapChain->SwapIndex();
}

void GraphicsCommandQueue::FlushResourceCommandQueue(ResourceCommandList& rscCommandList)
{
	// TODO. Excute 하는 시점에 경계조건이어야되지않나 싶은데..
	// Fence값이 보장안되다보니
	rscCommandList.m_resCmdList->Close();

	ID3D12CommandList* cmdListArr[] = { rscCommandList.m_resCmdList.Get() };
	m_cmdQueue->ExecuteCommandLists(_countof(cmdListArr), cmdListArr);

	WaitSync();

	rscCommandList.m_resCmdAlloc->Reset();
	rscCommandList.m_resCmdList->Reset(rscCommandList.m_resCmdAlloc.Get(), nullptr);
	m_resCmdLists.push(rscCommandList);
	m_rscCv.notify_one();
}

ResourceCommandList GraphicsCommandQueue::GetResourceCmdList()
{
	while (true) 
	{
		std::unique_lock<std::mutex> lock(m_rscMutex);
		m_rscCv.wait(lock, [this]() { return !m_resCmdLists.empty(); });

		ResourceCommandList rcsCommandList = m_resCmdLists.front();
		m_resCmdLists.pop();
		lock.unlock();
		return rcsCommandList;
	}
}

}
