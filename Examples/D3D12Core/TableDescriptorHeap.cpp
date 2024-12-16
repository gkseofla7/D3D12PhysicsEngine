#include "TableDescriptorHeap.h"
#include "Engine.h"
#include "Device.h"
#include "CommandQueue.h"

namespace dengine {
void GraphicsDescriptorHeap::Init(uint32 count)
{
	m_groupCount = count;

	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.NumDescriptors = count * (CBV_SRV_REGISTER_COUNT - 1); // b0는 전역
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

	DEVICE->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_descHeap));

	m_handleSize = DEVICE->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	m_groupSize = m_handleSize * (CBV_SRV_REGISTER_COUNT - 1); // b0는 전역
}

void GraphicsDescriptorHeap::Clear()
{
	m_currentGroupIndex = 0;
}

void GraphicsDescriptorHeap::SetCBV(D3D12_CPU_DESCRIPTOR_HANDLE srcHandle, CBV_REGISTER reg)
{
	D3D12_CPU_DESCRIPTOR_HANDLE destHandle = GetCPUHandle(reg);

	uint32 destRange = 1;
	uint32 srcRange = 1;
	DEVICE->CopyDescriptors(1, &destHandle, &destRange, 1, &srcHandle, &srcRange, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

void GraphicsDescriptorHeap::SetSRV(D3D12_CPU_DESCRIPTOR_HANDLE srcHandle, SRV_REGISTER reg)
{
	D3D12_CPU_DESCRIPTOR_HANDLE destHandle = GetCPUHandle(reg);

	uint32 destRange = 1;
	uint32 srcRange = 1;
	DEVICE->CopyDescriptors(1, &destHandle, &destRange, 1, &srcHandle, &srcRange, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

void GraphicsDescriptorHeap::CommitTable()
{ 
	GRAPHICS_CMD_LIST->SetGraphicsRootDescriptorTable(0, GetGPUHandle(SRV_REGISTER::t10)); 
	GRAPHICS_CMD_LIST->SetGraphicsRootDescriptorTable(1, GetGPUHandle(CBV_REGISTER::b0));
	GRAPHICS_CMD_LIST->SetGraphicsRootDescriptorTable(2, GetGPUHandle(SRV_REGISTER::t0));
	m_currentGroupIndex++;
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
	D3D12_CPU_DESCRIPTOR_HANDLE handle = m_descHeap->GetCPUDescriptorHandleForHeapStart();
	handle.ptr += m_currentGroupIndex * m_groupSize;
	handle.ptr += (reg) * m_handleSize;
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
	D3D12_GPU_DESCRIPTOR_HANDLE handle = m_descHeap->GetGPUDescriptorHandleForHeapStart();
	handle.ptr += m_currentGroupIndex * m_groupSize;
	handle.ptr += (reg) * m_handleSize;
	return handle;
}

}
