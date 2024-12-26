#pragma once
#include "EnginePch.h"


namespace dengine {
class GraphicsDescriptorHeap
{
public:
	void Init(uint32 count);

	void Clear();
	void SetCBV(D3D12_CPU_DESCRIPTOR_HANDLE srcHandle, CBV_REGISTER reg);
	void SetSRV(D3D12_CPU_DESCRIPTOR_HANDLE srcHandle, SRV_REGISTER reg);

	void ClearSRV();

	void CommitTable();
	void CommitTableForSampling();

	void CommitGlobalTextureTable();

	ComPtr<ID3D12DescriptorHeap> GetDescriptorHeap() { return m_descHeap; }

	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle(CBV_REGISTER reg);
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle(SRV_REGISTER reg);

	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle(CBV_REGISTER reg);
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle(SRV_REGISTER reg);
private:
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle(uint8 reg);


	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle(uint8 reg);

private:

	ComPtr<ID3D12DescriptorHeap> m_descHeap;
	uint64					m_handleSize = 0;
	uint64					m_groupSize = 0;
	uint64					m_groupCount = 0;

	uint32					m_currentGroupIndex = 0;
};
}

