#pragma once
#include "D3D12Utils.h"
#include "EngineDef.h"
namespace hlab {


class ConstantBuffer
{
public:
	ConstantBuffer();
	~ConstantBuffer();

	void Init(ComPtr<ID3D12Device> device, CBV_REGISTER reg, uint32 size);

	void Clear();
	void UpdateBuffer(void* buffer, uint32 size);

	D3D12_CPU_DESCRIPTOR_HANDLE GetCpuHandle(uint32 index);

private:
	void CreateBuffer(ComPtr<ID3D12Device> device);
	void CreateView(ComPtr<ID3D12Device> device);

private:
	ComPtr<ID3D12Resource>	m_cbvBuffer;
	BYTE* m_mappedBuffer = nullptr;
	uint32					m_elementSize = 0;

	CD3DX12_CPU_DESCRIPTOR_HANDLE			m_cpuHandle= {};

	CBV_REGISTER			m_reg = {};
};

}