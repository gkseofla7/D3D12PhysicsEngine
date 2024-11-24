#pragma once

#include "D3D12Utils.h"
namespace hlab {

namespace Graphics{

extern ComPtr<ID3D12DescriptorHeap> rtvCbvHeap;
extern ComPtr<ID3D12DescriptorHeap> samplerHeap;

extern ComPtr<ID3DBlob> basicVS;
extern ComPtr<ID3DBlob> skinnedVS;
extern ComPtr<ID3DBlob> basicPS;

// Rasterizer States
D3D12_RASTERIZER_DESC solidRSDesc; // front only

extern ComPtr<ID3D12RootSignature> defaultRootSignature;

extern ComPtr<ID3D12PipelineState> defaultSolidPSO;

extern vector< D3D12_SAMPLER_DESC> sampDescs;
extern D3D12_BLEND_DESC mirrorBSDesc;
extern D3D12_BLEND_DESC accumulateBSDesc;
extern D3D12_BLEND_DESC alphaBSDesc;
void InitSamplerDescs();
void InitDescriptorHeap(ComPtr<ID3D12Device>& device);
void InitShaders(ComPtr<ID3D12Device>& device);
void InitRasterizerDesc();
void InitBlendStates(ComPtr<ID3D12Device>& device);
void InitRootSignature(ComPtr<ID3D12Device>& device);
void InitPipelineStates(ComPtr<ID3D12Device>& device);


}


}
