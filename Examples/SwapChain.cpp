#include "SwapChain.h"
namespace hlab {
void SwapChain::Init(const HWND& mainWindow, ComPtr<IDXGIFactory4>& dxgi, ComPtr<ID3D12CommandQueue>& cmdQueue,
	int screenWidth, int screenHeight)
{
	CreateSwapChain( mainWindow, dxgi, cmdQueue,
		screenWidth, screenHeight);
}

void SwapChain::Present()
{
	// Present the frame.
	m_swapChain->Present(0, 0);
}

void SwapChain::SwapIndex()
{
	m_backBufferIndex = (m_backBufferIndex + 1) % 2;
}

void SwapChain::CreateSwapChain(const HWND& mainWindow, ComPtr<IDXGIFactory4>& dxgi, ComPtr<ID3D12CommandQueue>& cmdQueue,
	int screenWidth, int screenHeight)
{
	m_swapChain.Reset();
	// Describe and create the swap chain.
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.BufferCount = 2;
	swapChainDesc.Width = screenWidth;
	swapChainDesc.Height = screenHeight;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.SampleDesc.Count = 1;

	ComPtr<IDXGISwapChain1> swapChain;
	ThrowIfFailed(dxgi->CreateSwapChainForHwnd(
		cmdQueue.Get(),        // Swap chain needs the queue so that it can force a flush on it.
		mainWindow,
		&swapChainDesc,
		nullptr,
		nullptr,
		&swapChain
	));
	    ThrowIfFailed(dxgi->MakeWindowAssociation(mainWindow, DXGI_MWA_NO_ALT_ENTER));
	ThrowIfFailed(swapChain.As(&m_swapChain));


}
}