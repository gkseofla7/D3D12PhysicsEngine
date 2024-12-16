#pragma once
#include "EnginePch.h"

namespace dengine {
class Samplers
{
public:
	void Init();
	ComPtr<ID3D12DescriptorHeap> GetDescHeap() { return m_descHeap; }
	const vector<D3D12_SAMPLER_DESC>& GetSampleDesc() { return m_sampDescs; }
private:
	ComPtr<ID3D12DescriptorHeap> m_descHeap;
	vector<D3D12_SAMPLER_DESC> m_sampDescs;
};
}


