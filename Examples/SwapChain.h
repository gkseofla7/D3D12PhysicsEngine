#pragma once
#include "D3D12Utils.h"
namespace hlab {
class SwapChain
{
public:
	void Init(const HWND& m_mainWindow, ComPtr<IDXGIFactory4>& dxgi, ComPtr<ID3D12CommandQueue>& cmdQueue,
		int screenWidth, int screenHeight);
	void Present();
	void SwapIndex();

	ComPtr<IDXGISwapChain3> GetSwapChain() { return m_swapChain; }
	UINT8 GetBackBufferIndex() { return m_backBufferIndex; }

private:
	void CreateSwapChain(const HWND& m_mainWindow, ComPtr<IDXGIFactory4>& dxgi, ComPtr<ID3D12CommandQueue>& cmdQueue,
		int screenWidth, int screenHeight);

private:
	ComPtr<IDXGISwapChain3> m_swapChain;
	UINT32					m_backBufferIndex = 0;
};

}