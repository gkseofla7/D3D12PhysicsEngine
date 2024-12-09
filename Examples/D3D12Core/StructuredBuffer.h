#pragma once
#include "EnginePch.h"
namespace hlab {
class StructuredBuffer2
{
public:
	StructuredBuffer2();
	~StructuredBuffer2();

	void Init(uint32 elementSize, uint32 elementCount, void* initialData = nullptr);

	void PushGraphicsData(SRV_REGISTER reg);

	ComPtr<ID3D12DescriptorHeap> GetSRV() { return m_srvHeap; }

	void SetResourceState(D3D12_RESOURCE_STATES state) { m_resourceState = state; }
	D3D12_RESOURCE_STATES GetResourceState() { return m_resourceState; }
	ComPtr<ID3D12Resource> GetBuffer() { return m_buffer; }

	uint32	GetElementSize() { return m_elementSize; }
	uint32	GetElementCount() { return m_elementCount; }
	UINT	GetBufferSize() { return m_elementSize * m_elementCount; }

private:
	void CopyInitialData(uint64 bufferSize, void* initialData);

private:
	ComPtr<ID3D12Resource>			m_buffer;
	ComPtr<ID3D12DescriptorHeap>	m_srvHeap;

	uint32						m_elementSize = 0;
	uint32						m_elementCount = 0;
	D3D12_RESOURCE_STATES		m_resourceState = {};

private:
	D3D12_CPU_DESCRIPTOR_HANDLE m_srvHeapBegin = {};
	D3D12_CPU_DESCRIPTOR_HANDLE _uavHeapBegin = {};
};


}
