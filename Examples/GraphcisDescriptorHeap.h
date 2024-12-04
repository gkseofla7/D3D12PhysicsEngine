#pragma once
#include "D3D12Utils.h"
#include "EngineDef.h"
namespace hlab {

// ************************
// GraphicsDescriptorHeap
// ************************
class GraphicsDescriptorHeap
{
public:
	void Init(ComPtr<ID3D12Device>& device);

	void SetCBV(ComPtr<ID3D12Device>& device, D3D12_CPU_DESCRIPTOR_HANDLE srcHandle, CBV_REGISTER reg);
	void SetSRV(ComPtr<ID3D12Device>& device, D3D12_CPU_DESCRIPTOR_HANDLE srcHandle, SRV_REGISTER reg);
	void SetSampleView(ComPtr<ID3D12Device>& device, D3D12_CPU_DESCRIPTOR_HANDLE srcHandle, SAMPLE_REGISTER reg);

	void CommitTable(ComPtr<ID3D12GraphicsCommandList> commandList);

	ComPtr<ID3D12DescriptorHeap> GetDescriptorHeap() { return m_descHeap; }

	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle(CBV_REGISTER reg);
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle(SRV_REGISTER reg);

private:
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle(UINT8 reg);
private:

	ComPtr<ID3D12DescriptorHeap> m_descHeap;
	ComPtr<ID3D12DescriptorHeap> m_sampleDescHeap;
	
	int	m_handleSize = 0;
	int	m_sampleHandleSize = 0;
};
}