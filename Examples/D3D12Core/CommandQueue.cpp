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

	device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_cmdAlloc));
	device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_cmdAlloc.Get(), nullptr, IID_PPV_ARGS(&m_cmdList));
	m_cmdList->Close();

	for (int i = 0; i < 5; i++)
	{
		ResourceCommandList ResCommandList;
		device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&ResCommandList.m_resCmdAlloc));
		device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, ResCommandList.m_resCmdAlloc.Get(), nullptr, IID_PPV_ARGS(&ResCommandList.m_resCmdList));
		m_resCmdLists.push(ResCommandList);
	}

	// CreateFence
	// - CPU�� GPU�� ����ȭ �������� ���δ�
	device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence));
	m_fenceEvent = ::CreateEvent(nullptr, FALSE, FALSE, nullptr);
}

void GraphicsCommandQueue::WaitSync()
{
	// Advance the fence value to mark commands up to this fence point.
	m_fenceValue++;

	// Add an instruction to the command queue to set a new fence point.  Because we 
	// are on the GPU timeline, the new fence point won't be set until the GPU finishes
	// processing all the commands prior to this Signal().
	m_cmdQueue->Signal(m_fence.Get(), m_fenceValue);

	// Wait until the GPU has completed commands up to this fence point.
	if (m_fence->GetCompletedValue() < m_fenceValue)
	{
		// Fire event when GPU hits current fence.  
		m_fence->SetEventOnCompletion(m_fenceValue, m_fenceEvent);

		// Wait until the GPU hits current fence event is fired.
		::WaitForSingleObject(m_fenceEvent, INFINITE);
	}
}

void GraphicsCommandQueue::RenderBegin()
{
	m_cmdAlloc->Reset();
	m_cmdList->Reset(m_cmdAlloc.Get(), nullptr);

	int8 backIndex = m_swapChain->GetBackBufferIndex();
	D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
		GEngine->GetRTGroup(RENDER_TARGET_GROUP_TYPE::SWAP_CHAIN)->GetRTTexture(backIndex)->GetTex2D().Get(),
		D3D12_RESOURCE_STATE_PRESENT, // ȭ�� ���
		D3D12_RESOURCE_STATE_RENDER_TARGET); // ���� �����
	m_cmdList->ResourceBarrier(1, &barrier);
	GEngine->GetRTGroup(RENDER_TARGET_GROUP_TYPE::FLOAT)->WaitResourceToTarget(backIndex);
	GEngine->GetRTGroup(RENDER_TARGET_GROUP_TYPE::FLOAT)->ClearRenderTargetView(backIndex);
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
		GEngine->GetRTGroup(RENDER_TARGET_GROUP_TYPE::SWAP_CHAIN)->GetRTTexture(backIndex)->GetTex2D().Get(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, // ���� �����
		D3D12_RESOURCE_STATE_PRESENT); // ȭ�� ���

	m_cmdList->ResourceBarrier(1, &barrier);
	m_cmdList->Close();

	// Ŀ�ǵ� ����Ʈ ����
	ID3D12CommandList* cmdListArr[] = { m_cmdList.Get() };
	m_cmdQueue->ExecuteCommandLists(_countof(cmdListArr), cmdListArr);

	m_swapChain->Present();

	WaitSync();

	m_swapChain->SwapIndex();
}

void GraphicsCommandQueue::FlushResourceCommandQueue(ResourceCommandList& rscCommandList)
{
	// TODO. Excute �ϴ� ������ ��������̾�ߵ����ʳ� ������..
	// Fence���� ����ȵǴٺ���
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
