#include "TableDescriptorHeap.h"
#include "Engine.h"
#include "Device.h"
#include "CommandQueue.h"
#include "SwapChain.h"
namespace dengine {
void GraphicsDescriptorHeap::Init(uint32 count)
{
	m_groupCount = count;
	for (int i = 0; i < SWAP_CHAIN_BUFFER_COUNT; i++)
	{
		D3D12_DESCRIPTOR_HEAP_DESC desc = {};
		desc.NumDescriptors = count * (CBV_SRV_REGISTER_COUNT); // b0�� ����
		desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

		DEVICE->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_descHeap[i]));

		m_handleSize = DEVICE->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		m_groupSize = m_handleSize * (CBV_SRV_REGISTER_COUNT); // b0�� ���� -1 ����
	}
}

void GraphicsDescriptorHeap::Clear()
{// �ε��� 0�� �۷ι� ������ �ֵ鸸 ����
	m_currentGroupIndex = 1;
}

void GraphicsDescriptorHeap::SetGlobalCBV(D3D12_CPU_DESCRIPTOR_HANDLE srcHandle, CBV_REGISTER reg)
{
	D3D12_CPU_DESCRIPTOR_HANDLE destHandle = GetGlobalCPUHandle(reg);

	uint32 destRange = 1;
	uint32 srcRange = 1;
	DEVICE->CopyDescriptors(1, &destHandle, &destRange, 1, &srcHandle, &srcRange, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

void GraphicsDescriptorHeap::SetGlobalSRV(D3D12_CPU_DESCRIPTOR_HANDLE srcHandle, SRV_REGISTER reg, int count)
{
	D3D12_CPU_DESCRIPTOR_HANDLE destHandle = GetGlobalCPUHandle(reg);
	uint32 destRange = count;
	uint32 srcRange = count;

	DEVICE->CopyDescriptors(1, &destHandle, &destRange, 1, &srcHandle, &srcRange, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

void GraphicsDescriptorHeap::SetCBV(D3D12_CPU_DESCRIPTOR_HANDLE srcHandle, CBV_REGISTER reg)
{
	D3D12_CPU_DESCRIPTOR_HANDLE destHandle = GetCPUHandle(reg);

	uint32 destRange = 1;
	uint32 srcRange = 1;
	DEVICE->CopyDescriptors(1, &destHandle, &destRange, 1, &srcHandle, &srcRange, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

void GraphicsDescriptorHeap::SetSRV(D3D12_CPU_DESCRIPTOR_HANDLE srcHandle, SRV_REGISTER reg, int count)
{
	D3D12_CPU_DESCRIPTOR_HANDLE destHandle = GetCPUHandle(reg);
	uint32 destRange = count;
	uint32 srcRange = count;
	
	DEVICE->CopyDescriptors(1, &destHandle, &destRange, 1, &srcHandle, &srcRange, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}
void GraphicsDescriptorHeap::ClearSRV(SRV_REGISTER reg)
{
	// TODO. �׻� �� �ؽ�ó�� ������ �صα�, �� ���� ��� ������..
	D3D12_CPU_DESCRIPTOR_HANDLE srvHandle = GEngine->GetDefaultTexture()->GetSRVHandle();
	SetSRV(srvHandle, reg);
}

void GraphicsDescriptorHeap::CommitTable()
{
	// �׻� RootSignature�� ����ߵȴ�
	// t10�� ���뵥�������� 
	// TODO. ���� ��ü�� RootSignature�� �ٸ������� Commit ����ߵǴ°� �ٸ���..
	// �ϴ� ������ �� �ٸ� ������Ʈ �����ؼ� �����غ���
	GRAPHICS_CMD_LIST->SetGraphicsRootDescriptorTable(3, GetGPUHandle(CBV_REGISTER::b1));
	GRAPHICS_CMD_LIST->SetGraphicsRootDescriptorTable(4, GetGPUHandle(SRV_REGISTER::t0));
	GRAPHICS_CMD_LIST->SetGraphicsRootDescriptorTable(5, GetGPUHandle(SRV_REGISTER::t9));
	m_currentGroupIndex++;
}


void GraphicsDescriptorHeap::CommitTableForSampling()
{
	// �׻� RootSignature�� ����ߵȴ�
	// t10�� ���뵥�������� 
	//GRAPHICS_CMD_LIST->SetGraphicsRootDescriptorTable(1, GetGPUHandle(CBV_REGISTER::b0));
	GRAPHICS_CMD_LIST->SetGraphicsRootDescriptorTable(1, GetGPUHandle(SRV_REGISTER::t0));
	m_currentGroupIndex++;
}

void GraphicsDescriptorHeap::CommitGlobalTable()
{
	GRAPHICS_CMD_LIST->SetGraphicsRootDescriptorTable(1, GetGlobalGPUHandle(SRV_REGISTER::t10));
	// TODO. PSO Type���� ��Ʈ �����ڷ� �߰��ϴ°� ������ ���δ�.
	if (GEngine->GetPSOType() == PSOType::DEFAULT)
	{
		GRAPHICS_CMD_LIST->SetGraphicsRootDescriptorTable(6, GetGlobalGPUHandle(SRV_REGISTER::t15));
	}
}

ComPtr<ID3D12DescriptorHeap> GraphicsDescriptorHeap::GetDescriptorHeap()
{
	return m_descHeap[BACKBUFFER_INDEX];
}

D3D12_CPU_DESCRIPTOR_HANDLE GraphicsDescriptorHeap::GetCPUHandle(CBV_REGISTER reg)
{
	return GetCPUHandle(static_cast<uint8>(reg));
}

D3D12_CPU_DESCRIPTOR_HANDLE GraphicsDescriptorHeap::GetCPUHandle(SRV_REGISTER reg)
{
	return GetCPUHandle(static_cast<uint8>(reg));
}

D3D12_CPU_DESCRIPTOR_HANDLE GraphicsDescriptorHeap::GetCPUHandle(uint8 reg)
{
	assert(reg >= 0);
	D3D12_CPU_DESCRIPTOR_HANDLE handle = m_descHeap[BACKBUFFER_INDEX]->GetCPUDescriptorHandleForHeapStart();
	handle.ptr += m_currentGroupIndex * m_groupSize;
	handle.ptr += (reg) * m_handleSize;
	return handle;
}

D3D12_CPU_DESCRIPTOR_HANDLE GraphicsDescriptorHeap::GetGlobalCPUHandle(CBV_REGISTER reg)
{
	return GetGlobalCPUHandle(static_cast<uint8>(reg));
}
D3D12_CPU_DESCRIPTOR_HANDLE GraphicsDescriptorHeap::GetGlobalCPUHandle(SRV_REGISTER reg)
{
	return GetGlobalCPUHandle(static_cast<uint8>(reg));
}

D3D12_CPU_DESCRIPTOR_HANDLE GraphicsDescriptorHeap::GetGlobalCPUHandle(uint8 reg)
{
	assert(reg >= 0);
	D3D12_CPU_DESCRIPTOR_HANDLE handle = m_descHeap[BACKBUFFER_INDEX]->GetCPUDescriptorHandleForHeapStart();
	handle.ptr += (reg)*m_handleSize;
	return handle;
}

D3D12_GPU_DESCRIPTOR_HANDLE GraphicsDescriptorHeap::GetGPUHandle(CBV_REGISTER reg)
{
	return GetGPUHandle(static_cast<uint8>(reg));
}
D3D12_GPU_DESCRIPTOR_HANDLE GraphicsDescriptorHeap::GetGPUHandle(SRV_REGISTER reg)
{
	return GetGPUHandle(static_cast<uint8>(reg));
}
D3D12_GPU_DESCRIPTOR_HANDLE GraphicsDescriptorHeap::GetGPUHandle(uint8 reg)
{
	assert(reg >= 0);
	D3D12_GPU_DESCRIPTOR_HANDLE handle = m_descHeap[BACKBUFFER_INDEX]->GetGPUDescriptorHandleForHeapStart();
	handle.ptr += m_currentGroupIndex * m_groupSize;
	handle.ptr += (reg) * m_handleSize;
	return handle;
}

D3D12_GPU_DESCRIPTOR_HANDLE GraphicsDescriptorHeap::GetGlobalGPUHandle(CBV_REGISTER reg)
{
	return GetGlobalGPUHandle(static_cast<uint8>(reg));
}
D3D12_GPU_DESCRIPTOR_HANDLE GraphicsDescriptorHeap::GetGlobalGPUHandle(SRV_REGISTER reg)
{
	return GetGlobalGPUHandle(static_cast<uint8>(reg));
}
D3D12_GPU_DESCRIPTOR_HANDLE GraphicsDescriptorHeap::GetGlobalGPUHandle(uint8 reg)
{
	assert(reg >= 0);
	D3D12_GPU_DESCRIPTOR_HANDLE handle = m_descHeap[BACKBUFFER_INDEX]->GetGPUDescriptorHandleForHeapStart();
	handle.ptr += (reg)*m_handleSize;
	return handle;
}

}
