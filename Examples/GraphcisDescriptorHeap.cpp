#include "GraphcisDescriptorHeap.h"
#include "GraphicsCommonD3D12.h"
namespace hlab{

void GraphicsDescriptorHeap::Init(ComPtr<ID3D12Device>& device)
{
	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.NumDescriptors = 256; // b0는 전역
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	ThrowIfFailed(device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_descHeap)));
	m_handleSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	//D3D12_DESCRIPTOR_HEAP_DESC samplerHeapDesc = {};
	//samplerHeapDesc.NumDescriptors = 256;
	//samplerHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
	//samplerHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;  // 셰이더에서 참조할 수 있도록 설정
	//ThrowIfFailed(device->CreateDescriptorHeap(&samplerHeapDesc, IID_PPV_ARGS(&m_sampleDescHeap)));
	//m_sampleHandleSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
}

void GraphicsDescriptorHeap::SetCBV(ComPtr<ID3D12Device>& device, D3D12_CPU_DESCRIPTOR_HANDLE srcHandle, CBV_REGISTER reg)
{
	D3D12_CPU_DESCRIPTOR_HANDLE destHandle = GetCPUHandle(reg);

	UINT32 destRange = 1;
	UINT32 srcRange = 1;
	device->CopyDescriptors(1, &destHandle, &destRange, 1, &srcHandle, &srcRange, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

void GraphicsDescriptorHeap::SetSRV(ComPtr<ID3D12Device>& device, D3D12_CPU_DESCRIPTOR_HANDLE srcHandle, SRV_REGISTER reg)
{
	D3D12_CPU_DESCRIPTOR_HANDLE destHandle = GetCPUHandle(reg);

	UINT32 destRange = 1;
	UINT32 srcRange = 1;
	device->CopyDescriptors(1, &destHandle, &destRange, 1, &srcHandle, &srcRange, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

void GraphicsDescriptorHeap::SetSampleView(ComPtr<ID3D12Device>& device, D3D12_CPU_DESCRIPTOR_HANDLE srcHandle, SAMPLE_REGISTER reg)
{
	D3D12_CPU_DESCRIPTOR_HANDLE handle = m_sampleDescHeap->GetCPUDescriptorHandleForHeapStart();
	handle.ptr += (static_cast<UINT8>(reg) - 1) * m_sampleHandleSize;

	UINT32 destRange = 1;
	UINT32 srcRange = 1;
	device->CopyDescriptors(1, &handle, &destRange, 1, &srcHandle, &srcRange, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
}

void GraphicsDescriptorHeap::CommitTable(ComPtr<ID3D12GraphicsCommandList> commandList)
{
	D3D12_GPU_DESCRIPTOR_HANDLE handle = m_descHeap->GetGPUDescriptorHandleForHeapStart();
	commandList->SetGraphicsRootDescriptorTable(0, handle);

	D3D12_GPU_DESCRIPTOR_HANDLE sampleHandle = DGraphics::samplerHeap->GetGPUDescriptorHandleForHeapStart();
	commandList->SetGraphicsRootDescriptorTable(1, sampleHandle);
}

D3D12_CPU_DESCRIPTOR_HANDLE GraphicsDescriptorHeap::GetCPUHandle(CBV_REGISTER reg)
{
	return GetCPUHandle(static_cast<UINT8>(reg));
}

D3D12_CPU_DESCRIPTOR_HANDLE GraphicsDescriptorHeap::GetCPUHandle(SRV_REGISTER reg)
{
	return GetCPUHandle(static_cast<UINT8>(reg));
}

D3D12_CPU_DESCRIPTOR_HANDLE GraphicsDescriptorHeap::GetCPUHandle(UINT8 reg)
{
	assert(reg > 0);
	D3D12_CPU_DESCRIPTOR_HANDLE handle = m_descHeap->GetCPUDescriptorHandleForHeapStart();
	handle.ptr += (reg - 1) * m_handleSize;
	return handle;
}
}