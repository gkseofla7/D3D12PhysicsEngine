#pragma once
#include "EnginePch.h"
#include "Engine.h"
#include "CommandQueue.h"

namespace dengine {
template <typename T_ELEMENT>
class StructuredBuffer
{
public:
	void Init(SRV_REGISTER inRegister)
	{ 
		m_reg = inRegister;

		m_elementSize = sizeof(T_ELEMENT);
		m_elementCount = m_cpu.size();
		m_structuredCount = SWAP_CHAIN_BUFFER_COUNT;
		m_resourceState = D3D12_RESOURCE_STATE_COMMON;

		// Buffer
		{
			uint64 bufferSize = static_cast<uint64>(m_elementSize) * m_elementCount * m_structuredCount;
			D3D12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
			D3D12_HEAP_PROPERTIES heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

			DEVICE->CreateCommittedResource(
				&heapProperties,
				D3D12_HEAP_FLAG_NONE,
				&desc,
				m_resourceState,
				nullptr,
				IID_PPV_ARGS(&m_buffer));
		}

		// SRV
		{
			D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
			srvHeapDesc.NumDescriptors = m_structuredCount;
			srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
			srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
			DEVICE->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&m_srvHeap));

			m_srvHeapBegin = m_srvHeap->GetCPUDescriptorHandleForHeapStart();

			m_handleSize = DEVICE->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

			for (int i = 0; i < m_structuredCount; i++)
			{
				D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
				srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
				srvDesc.Format = DXGI_FORMAT_UNKNOWN;
				srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
				srvDesc.Buffer.FirstElement = i * m_elementCount;
				srvDesc.Buffer.NumElements = m_elementCount;
				srvDesc.Buffer.StructureByteStride = m_elementSize;
				srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
				D3D12_CPU_DESCRIPTOR_HANDLE handle = m_srvHeapBegin;
				handle.ptr += i * m_handleSize;

				DEVICE->CreateShaderResourceView(m_buffer.Get()
					, &srvDesc, handle);
			}

		}

		//// CBV
		//{
		//	D3D12_DESCRIPTOR_HEAP_DESC cbvDesc = {};
		//	cbvDesc.NumDescriptors = m_structuredCount;
		//	cbvDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;.
		//	cbvDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		//	DEVICE->CreateDescriptorHeap(&cbvDesc, IID_PPV_ARGS(&m_cbvHeap));


		//	//D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
		//	//cbvDesc.BufferLocation = m_cbvBuffer->GetGPUVirtualAddress() + static_cast<uint64>(m_elementSize) * i;
		//	//cbvDesc.SizeInBytes = m_elementSize;   // CB size is required to be 256-byte aligned.

		//	//DEVICE->CreateConstantBufferView(&cbvDesc, cbvHandle);
		//}
		m_init = true;
	}

	void Upload()
	{
		if (m_init == false)
		{
			return;
		}
		m_structuredIndex = (m_structuredIndex + 1) % m_structuredCount;
		UINT bufferSize = GetBufferSize();
		D3D12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize, D3D12_RESOURCE_FLAG_NONE);
		D3D12_HEAP_PROPERTIES heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);

		DEVICE->CreateCommittedResource(
			&heapProperties,
			D3D12_HEAP_FLAG_NONE,
			&desc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&(m_readBuffers[m_structuredIndex])));

		uint8* dataBegin = nullptr;
		D3D12_RANGE readRange{ 0, 0 };
		m_readBuffers[m_structuredIndex]->Map(0, &readRange, reinterpret_cast<void**>(&dataBegin));
		memcpy(dataBegin, reinterpret_cast<void*>(&m_cpu[0]), bufferSize);
		m_readBuffers[m_structuredIndex]->Unmap(0, nullptr);

		// Common -> Copy
		ResourceCommandList rscCommandList = RESOURCE_CMD_LIST;
		{
			D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_buffer.Get(),
				D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);
			
			rscCommandList.m_resCmdList->ResourceBarrier(1, &barrier);
		}

		uint64 structuredOffset = m_structuredIndex * m_elementCount*m_elementSize;
		rscCommandList.m_resCmdList->CopyBufferRegion(m_buffer.Get(), structuredOffset, m_readBuffers[m_structuredIndex].Get(), 0, bufferSize);

		// Copy -> Common
		{
			D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_buffer.Get(),
				D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_COMMON);
			rscCommandList.m_resCmdList->ResourceBarrier(1, &barrier);
		}

		GEngine->GetGraphicsCmdQueue()->FlushResourceCommandQueue(rscCommandList);
		m_resourceState = D3D12_RESOURCE_STATE_COMMON;
	}

	void PushGraphicsData()
	{
		if (m_init == false)
		{
			return;
		}
		D3D12_CPU_DESCRIPTOR_HANDLE handle = m_srvHeapBegin;
		handle.ptr += m_structuredIndex * m_handleSize;
		GEngine->GetGraphicsDescHeap()->SetSRV(handle, m_reg);
	}

	vector<T_ELEMENT>& GetCpu() { return m_cpu; }
	ComPtr<ID3D12DescriptorHeap> GetSRV() { return m_srvHeap; }

	void SetResourceState(D3D12_RESOURCE_STATES state) { m_resourceState = state; }
	D3D12_RESOURCE_STATES GetResourceState() { return m_resourceState; }
	ComPtr<ID3D12Resource> GetBuffer() { return m_buffer; }

	uint32	GetElementSize() { return m_elementSize; }
	uint32	GetElementCount() { return m_elementCount; }
	UINT	GetBufferSize() { return m_elementSize * m_elementCount; }

private:


private:
	vector<T_ELEMENT> m_cpu;
	ComPtr<ID3D12Resource>			m_buffer;
	ComPtr<ID3D12DescriptorHeap>	m_srvHeap;
	ComPtr<ID3D12DescriptorHeap>	m_cbvHeap;

	uint32						m_elementSize = 0;
	uint32						m_elementCount = 0;
	uint32						m_structuredCount = 0;
	uint32						m_structuredIndex = 0;
	D3D12_RESOURCE_STATES		m_resourceState = {};

	std::array<ComPtr<ID3D12Resource>, SWAP_CHAIN_BUFFER_COUNT> m_readBuffers;
private:
	D3D12_CPU_DESCRIPTOR_HANDLE m_srvHeapBegin = {};
	D3D12_CPU_DESCRIPTOR_HANDLE _uavHeapBegin = {};

	uint64					m_handleSize = 0;

	SRV_REGISTER			m_reg = {};

	bool m_init = false;
};


}
