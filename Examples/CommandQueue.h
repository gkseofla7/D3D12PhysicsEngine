#pragma once
#include <d3d12.h>
#include "d3dx12.h"
namespace hlab {

using Microsoft::WRL::ComPtr;

// ************************
// GraphicsCommandQueue
// ************************

class GraphicsCommandQueue
{
public:
	~GraphicsCommandQueue();

	void Init(ComPtr<ID3D12Device> device, shared_ptr<SwapChain> swapChain);
	void WaitSync();

	void RenderBegin(ComPtr<ID3D12Resource>& renderTarget);
	void RenderEnd(ComPtr<ID3D12Resource>& renderTarget);

	void FlushResourceCommandQueue();

	ComPtr<ID3D12CommandQueue> GetCmdQueue() { return _cmdQueue; }
	ComPtr<ID3D12GraphicsCommandList> GetGraphicsCmdList() { return	_cmdList; }
	ComPtr<ID3D12GraphicsCommandList> GetResourceCmdList() { return	_resCmdList; }

private:
	ComPtr<ID3D12CommandQueue>			m_cmdQueue;
	ComPtr<ID3D12CommandAllocator>		m_cmdAlloc;
	ComPtr<ID3D12GraphicsCommandList>	m_cmdList;

	ComPtr<ID3D12CommandAllocator>		_resCmdAlloc;
	ComPtr<ID3D12GraphicsCommandList>	_resCmdList;

	ComPtr<ID3D12Fence>					m_fence;
	UINT32								m_fenceValue = 0;
	HANDLE								m_fenceEvent = INVALID_HANDLE_VALUE;

	shared_ptr<SwapChain>		_swapChain;
};

