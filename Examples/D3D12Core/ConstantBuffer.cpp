#include "ConstantBuffer.h"
#include "Engine.h"
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



void ConstantBuffer::Init(CBV_REGISTER reg, uint32 size, uint32 count)
{
	m_reg = reg;

	// 상수 버퍼는 256 바이트 배수로 만들어야 한다
	// 0 256 512 768
	m_elementSize = (size + 255) & ~255;
	m_elementCount = count;

	CreateBuffer();
	CreateView();
}

void ConstantBuffer::CreateBuffer()
{
	uint32 bufferSize = m_elementSize * m_elementCount;
	D3D12_HEAP_PROPERTIES heapProperty = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	D3D12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);

	DEVICE->CreateCommittedResource(
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

void ConstantBuffer::CreateView()
{
	D3D12_DESCRIPTOR_HEAP_DESC cbvDesc = {};
	cbvDesc.NumDescriptors = m_elementCount;
	cbvDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	cbvDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	DEVICE->CreateDescriptorHeap(&cbvDesc, IID_PPV_ARGS(&m_cbvHeap));

	m_cpuHandleBegin = m_cbvHeap->GetCPUDescriptorHandleForHeapStart();
	m_handleIncrementSize = DEVICE->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	for (uint32 i = 0; i < m_elementCount; ++i)
	{
		D3D12_CPU_DESCRIPTOR_HANDLE cbvHandle = GetCpuHandle(i);

		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
		cbvDesc.BufferLocation = m_cbvBuffer->GetGPUVirtualAddress() + static_cast<uint64>(m_elementSize) * i;
		cbvDesc.SizeInBytes = m_elementSize;   // CB size is required to be 256-byte aligned.

		DEVICE->CreateConstantBufferView(&cbvDesc, cbvHandle);
	}
}

void ConstantBuffer::Clear()
{
	m_currentIndex = 0;
}

void ConstantBuffer::PushGraphicsData(void* buffer, uint32 size)
{
	assert(m_currentIndex < m_elementCount);
	assert(m_elementSize == ((size + 255) & ~255));

	::memcpy(&m_mappedBuffer[m_currentIndex * m_elementSize], buffer, size);

	D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = GetCpuHandle(m_currentIndex);
	GEngine->GetGraphicsDescHeap()->SetCBV(cpuHandle, m_reg);

	m_currentIndex++;
}

void ConstantBuffer::SetGraphicsGlobalData(void* buffer, uint32 size)
{
	assert(m_elementSize == ((size + 255) & ~255));
	::memcpy(&m_mappedBuffer[0], buffer, size);
	GRAPHICS_CMD_LIST->SetGraphicsRootConstantBufferView(0, GetGpuVirtualAddress(0));
}

void ConstantBuffer::PushComputeData(void* buffer, uint32 size)
{
	assert(m_currentIndex < m_elementCount);
	assert(m_elementSize == ((size + 255) & ~255));

	::memcpy(&m_mappedBuffer[m_currentIndex * m_elementSize], buffer, size);

	D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = GetCpuHandle(m_currentIndex);
	GEngine->GetComputeDescHeap()->SetCBV(cpuHandle, m_reg);

	m_currentIndex++;
}

D3D12_GPU_VIRTUAL_ADDRESS ConstantBuffer::GetGpuVirtualAddress(uint32 index)
{
	D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = m_cbvBuffer->GetGPUVirtualAddress();
	objCBAddress += index * m_elementSize;
	return objCBAddress;
}

D3D12_CPU_DESCRIPTOR_HANDLE ConstantBuffer::GetCpuHandle(uint32 index)
{
	return CD3DX12_CPU_DESCRIPTOR_HANDLE(m_cpuHandleBegin, index * m_handleIncrementSize);
}
}
