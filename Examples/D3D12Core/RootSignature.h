#pragma once
#include "EnginePch.h"

namespace dengine {
class RootSignature
{
public:
	void Init();

	ComPtr<ID3D12RootSignature>	GetGraphicsRootSignature() { return defaultRootSignature; }

private:
	void CreateDefaultRootSignature();
	void CreateGraphicsRootSignature();
	void CreateRootSignature(ComPtr<ID3D12RootSignature>& rootSignature, vector<CD3DX12_ROOT_PARAMETER1>& rootParams);
private:
	D3D12_STATIC_SAMPLER_DESC	_samplerDesc; 
	ComPtr<ID3D12RootSignature>	_graphicsRootSignature;	

	ComPtr<ID3D12RootSignature> defaultRootSignature;
};

}

