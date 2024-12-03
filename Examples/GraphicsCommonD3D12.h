#pragma once

#include "D3D12Utils.h"
#include <dxgi.h>                       // DXGIFactory
#include <dxgi1_4.h>                    // DXGIFactory4
namespace hlab {

namespace DGraphics{

static const uint32_t s_NumDescriptorsPerHeap = 256;

ComPtr<ID3D12CommandAllocator> commandAllocator;
ComPtr<ID3D12GraphicsCommandList> commandList;

extern ComPtr<ID3D12DescriptorHeap> srvCbvHeap;
extern ComPtr<ID3D12DescriptorHeap> rtvHeap;
extern ComPtr<ID3D12DescriptorHeap> samplerHeap;

//CD3DX12_CPU_DESCRIPTOR_HANDLE
extern CD3DX12_CPU_DESCRIPTOR_HANDLE cpuSrvCbvHandle;
extern CD3DX12_GPU_DESCRIPTOR_HANDLE gpuSrvCbvHandle;
extern CD3DX12_CPU_DESCRIPTOR_HANDLE cpuRtvHandle;
extern CD3DX12_GPU_DESCRIPTOR_HANDLE gpuRtvHandle;

extern UINT srvCbvDescriptorSize;
extern UINT rtvDescriptorSize;

extern ComPtr<ID3DBlob> basicVS;
extern ComPtr<ID3DBlob> skinnedVS;
extern ComPtr<ID3DBlob> basicPS;

// Rasterizer States
D3D12_RASTERIZER_DESC solidRSDesc; // front only

extern ComPtr<ID3D12RootSignature> defaultRootSignature;

extern ComPtr<ID3D12PipelineState> defaultSolidPSO;

extern std::vector< D3D12_SAMPLER_DESC> sampDescs;
extern std::vector< D3D12_BLEND_DESC> blendDescs;
extern D3D12_BLEND_DESC mirrorBSDesc;
extern D3D12_BLEND_DESC accumulateBSDesc;
extern D3D12_BLEND_DESC alphaBSDesc;

void InitCommonStates(ComPtr<ID3D12Device>& device);
void InitShaders(ComPtr<ID3D12Device>& device);
void InitSamplerDescs();
void InitRasterizerDesc();
void InitDescriptorHeap(ComPtr<ID3D12Device>& device);
void InitBlendStates();
void InitRootSignature(ComPtr<ID3D12Device>& device);
void InitPipelineStates(ComPtr<ID3D12Device>& device);

void RegisterSrvHeap(ComPtr<ID3D12Device>& device, const ComPtr<ID3D12Resource>& resource
	, const D3D12_SHADER_RESOURCE_VIEW_DESC* srvDesc, CD3DX12_CPU_DESCRIPTOR_HANDLE& srvHandle,
	CD3DX12_GPU_DESCRIPTOR_HANDLE& cbvGPUHandle);
void RegisterCBVHeap(ComPtr<ID3D12Device>& device, const ComPtr<ID3D12Resource>& resource,
	const D3D12_CONSTANT_BUFFER_VIEW_DESC* cbvDesc, CD3DX12_CPU_DESCRIPTOR_HANDLE& cbvCPUHandle,
	CD3DX12_GPU_DESCRIPTOR_HANDLE& cbvGPUHandle);
void RegisterRtvHeap(ComPtr<ID3D12Device>& device, const ComPtr<ID3D12Resource>& resource
	, const D3D12_RENDER_TARGET_VIEW_DESC* rtvDesc, CD3DX12_CPU_DESCRIPTOR_HANDLE& rtvHandle);

}


}

