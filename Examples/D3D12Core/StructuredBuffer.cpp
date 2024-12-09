#include "StructuredBuffer.h"
#include "Engine.h"
#include "Device.h"
#include "CommandQueue.h"
namespace hlab {
StructuredBuffer2::StructuredBuffer2()
{
}

StructuredBuffer2::~StructuredBuffer2()
{
}

void StructuredBuffer2::Init(uint32 elementSize, uint32 elementCount, void* initialData)
{
	m_elementSize = elementSize;
	m_elementCount = elementCount;
	m_resourceState = D3D12_RESOURCE_STATE_COMMON;

	// Buffer
	{
		uint64 bufferSize = static_cast<uint64>(m_elementSize) * m_elementCount;
		D3D12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
		D3D12_HEAP_PROPERTIES heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

		DEVICE->CreateCommittedResource(
			&heapProperties,
			D3D12_HEAP_FLAG_NONE,
			&desc,
			m_resourceState,
			nullptr,
			IID_PPV_ARGS(&m_buffer));

		if (initialData)
			CopyInitialData(bufferSize, initialData);
	}

	// SRV
	{
		D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
		srvHeapDesc.NumDescriptors = 1;
		srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		DEVICE->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&m_srvHeap));

		m_srvHeapBegin = m_srvHeap->GetCPUDescriptorHandleForHeapStart();

		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Format = DXGI_FORMAT_UNKNOWN;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		srvDesc.Buffer.FirstElement = 0;
		srvDesc.Buffer.NumElements = m_elementCount;
		srvDesc.Buffer.StructureByteStride = m_elementSize;
		srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

		DEVICE->CreateShaderResourceView(m_buffer.Get(), &srvDesc, m_srvHeapBegin);
	}
}

void StructuredBuffer2::PushGraphicsData(SRV_REGISTER reg)
{
	GEngine->GetGraphicsDescHeap()->SetSRV(m_srvHeapBegin, reg);
}

void StructuredBuffer2::CopyInitialData(uint64 bufferSize, void* initialData)
{
	ComPtr<ID3D12Resource> readBuffer = nullptr;
	D3D12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize, D3D12_RESOURCE_FLAG_NONE);
	D3D12_HEAP_PROPERTIES heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);

	DEVICE->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&desc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&readBuffer));

	uint8* dataBegin = nullptr;
	D3D12_RANGE readRange{ 0, 0 };
	readBuffer->Map(0, &readRange, reinterpret_cast<void**>(&dataBegin));
	memcpy(dataBegin, initialData, bufferSize);
	readBuffer->Unmap(0, nullptr);

	// Common -> Copy
	{
		D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_buffer.Get(),
			D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);

		RESOURCE_CMD_LIST->ResourceBarrier(1, &barrier);
	}

	RESOURCE_CMD_LIST->CopyBufferRegion(m_buffer.Get(), 0, readBuffer.Get(), 0, bufferSize);

	// Copy -> Common
	{
		D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_buffer.Get(),
			D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_COMMON);
		RESOURCE_CMD_LIST->ResourceBarrier(1, &barrier);
	}

	GEngine->GetGraphicsCmdQueue()->FlushResourceCommandQueue();

	m_resourceState = D3D12_RESOURCE_STATE_COMMON;
}
}