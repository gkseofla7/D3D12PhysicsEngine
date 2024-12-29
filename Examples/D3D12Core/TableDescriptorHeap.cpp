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
	// 항상 RootSignature과 맞춰야된다
	// t10은 공용데이터지만 
	// TODO. 여러 객체가 RootSignature가 다를때마다 Commit 해줘야되는게 다른데..
	// 일단 마무리 후 다른 프로젝트 참고해서 수정해보자
	GRAPHICS_CMD_LIST->SetGraphicsRootDescriptorTable(3, GetGPUHandle(CBV_REGISTER::b1));
	GRAPHICS_CMD_LIST->SetGraphicsRootDescriptorTable(4, GetGPUHandle(SRV_REGISTER::t0));
	GRAPHICS_CMD_LIST->SetGraphicsRootDescriptorTable(5, GetGPUHandle(SRV_REGISTER::t9));
	m_currentGroupIndex++;
}

void GraphicsDescriptorHeap::ClearSRV()
{
	// TODO. 항상 빈 텍스처라도 셋팅을 해두기, 더 좋은 방법 없을까..
	D3D12_CPU_DESCRIPTOR_HANDLE srvHandle = GEngine->GetDefaultTexture()->GetSRVHandle();
	SetSRV(srvHandle, SRV_REGISTER::t0);
	SetSRV(srvHandle, SRV_REGISTER::t1);
	SetSRV(srvHandle, SRV_REGISTER::t2);
	SetSRV(srvHandle, SRV_REGISTER::t3);
	SetSRV(srvHandle, SRV_REGISTER::t4);
	SetSRV(srvHandle, SRV_REGISTER::t5);
}

void GraphicsDescriptorHeap::CommitTableForSampling()
{
	// 항상 RootSignature과 맞춰야된다
	// t10은 공용데이터지만 
	//GRAPHICS_CMD_LIST->SetGraphicsRootDescriptorTable(1, GetGPUHandle(CBV_REGISTER::b0));
	GRAPHICS_CMD_LIST->SetGraphicsRootDescriptorTable(1, GetGPUHandle(SRV_REGISTER::t0));
	m_currentGroupIndex++;
}

void GraphicsDescriptorHeap::CommitGlobalTextureTable()
{
	GRAPHICS_CMD_LIST->SetGraphicsRootDescriptorTable(1, GetGPUHandle(SRV_REGISTER::t10));
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
