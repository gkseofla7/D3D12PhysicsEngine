#pragma once
#include "EnginePch.h"

// 구현 자체에 디스크립터 낭비가 좀 있지만..
namespace dengine {
class GraphicsDescriptorHeap
{
public:
	void Init(uint32 count);

	void Clear();
	void SetGlobalCBV(D3D12_CPU_DESCRIPTOR_HANDLE srcHandle, CBV_REGISTER reg);
	void SetGlobalSRV(D3D12_CPU_DESCRIPTOR_HANDLE srcHandle, SRV_REGISTER reg, int count = 1);
	void SetGlobalSRVForAllFrame(D3D12_CPU_DESCRIPTOR_HANDLE srcHandle, SRV_REGISTER reg, int count = 1);
	void SetCBV(D3D12_CPU_DESCRIPTOR_HANDLE srcHandle, CBV_REGISTER reg);
	void SetSRV(D3D12_CPU_DESCRIPTOR_HANDLE srcHandle, SRV_REGISTER reg, int count = 1);
	void ClearSRV(SRV_REGISTER reg);

	void SetGraphicsRootDescriptorTable(int inRootParameterIndex, CBV_REGISTER inRegister);
	void SetGraphicsRootDescriptorTable(int inRootParameterIndex, SRV_REGISTER inRegister);
	void CommitTable();

	void CommitTableForSampling();
	void CommitGlobalTable();


	ComPtr<ID3D12DescriptorHeap> GetDescriptorHeap();

	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle(CBV_REGISTER reg);
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle(SRV_REGISTER reg);

	D3D12_CPU_DESCRIPTOR_HANDLE GetGlobalCPUHandle(CBV_REGISTER reg);
	D3D12_CPU_DESCRIPTOR_HANDLE GetGlobalCPUHandle(SRV_REGISTER reg);

	D3D12_CPU_DESCRIPTOR_HANDLE GetGlobalCPUHandle(SRV_REGISTER reg, int frameIndex);

	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle(CBV_REGISTER reg);
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle(SRV_REGISTER reg);

	D3D12_GPU_DESCRIPTOR_HANDLE GetGlobalGPUHandle(CBV_REGISTER reg);
	D3D12_GPU_DESCRIPTOR_HANDLE GetGlobalGPUHandle(SRV_REGISTER reg);
private:
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle(uint8 reg);
	D3D12_CPU_DESCRIPTOR_HANDLE GetGlobalCPUHandle(uint8 reg);
	D3D12_CPU_DESCRIPTOR_HANDLE GetGlobalCPUHandle(uint8 reg, int frameIndex);

	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle(uint8 reg);
	D3D12_GPU_DESCRIPTOR_HANDLE GetGlobalGPUHandle(uint8 reg);

private:

	ComPtr<ID3D12DescriptorHeap> m_descHeap[SWAP_CHAIN_BUFFER_COUNT];
	uint64					m_handleSize = 0;
	uint64					m_groupSize = 0;
	uint64					m_groupCount = 0;

	uint32					m_currentGroupIndex = 0;
};
}

