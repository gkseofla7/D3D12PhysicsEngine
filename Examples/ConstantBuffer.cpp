#include "ConstantBuffer.h"
#include "GraphicsCommonD3D12.h"

namespace hlab {
	ConstantBuffer::ConstantBuffer()
{
}

ConstantBuffer::~ConstantBuffer()
{
	if (m_cbvBuffer)
	{
		if (m_cbvBuffer != nullptr)
			m_cbvBuffer->Unmap(0, nullptr);

		m_cbvBuffer = nullptr;
	}
}

void ConstantBuffer::Init(ComPtr<ID3D12Device> device, CBV_REGISTER reg, uint32 size)
{
	m_reg = reg;

	// 상수 버퍼는 256 바이트 배수로 만들어야 한다
	// 0 256 512 768
	m_elementSize = (size + 255) & ~255;

	CreateBuffer(device);
	CreateView(device);
}

void ConstantBuffer::CreateBuffer(ComPtr<ID3D12Device> device)
{
	uint32 bufferSize = m_elementSize;
	D3D12_HEAP_PROPERTIES heapProperty = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	D3D12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);

	device->CreateCommittedResource(
		&heapProperty,
		D3D12_HEAP_FLAG_NONE,
		&desc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_cbvBuffer));

	m_cbvBuffer->Map(0, nullptr, reinterpret_cast<void**>(&m_mappedBuffer));
	// We do not need to unmap until we are done with the resource.  However, we must not write to
	// the resource while it is in use by the GPU (so we must use synchronization techniques).
}

void ConstantBuffer::CreateView(ComPtr<ID3D12Device> device)
{
	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
	cbvDesc.BufferLocation = m_cbvBuffer->GetGPUVirtualAddress();
	cbvDesc.SizeInBytes = m_elementSize;
	CD3DX12_GPU_DESCRIPTOR_HANDLE cbvGPUHandle;
	DGraphics::RegisterCBVHeap(m_cbvBuffer, &cbvDesc, m_cpuHandle, cbvGPUHandle);
}

void ConstantBuffer::Clear()
{
}

void ConstantBuffer::UpdateBuffer(void* buffer, uint32 size)
{
	assert(m_elementSize == ((size + 255) & ~255));

	::memcpy(&m_mappedBuffer[0], buffer, size);
}

D3D12_CPU_DESCRIPTOR_HANDLE ConstantBuffer::GetCpuHandle(uint32 index)
{
	return m_cpuHandle;
}
}
