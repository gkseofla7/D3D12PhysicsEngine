#pragma once
#include "D3D12Utils.h"
namespace hlab {


enum class CBV_REGISTER : UINT8
{
	b0,
	b1,
	b2,
	b3,
	b4,

	END
};

enum class SRV_REGISTER : UINT8
{
	t0 = static_cast<UINT8>(CBV_REGISTER::END),
	t1,
	t2,
	t3,
	t4,
	t5,
	t6,
	t7,
	t8,
	t9,
	t10,
	t11,
	t12,
	t13,
	t14,
	t15,

	END
};

enum class SAMPLE_REGISTER : UINT8
{
	s0,
	s1,
	s2,
	s3,
	s4,
	s5,

	END
};

// ************************
// GraphicsDescriptorHeap
// ************************
class GraphicsDescriptorHeap
{
public:
	void Init(ComPtr<ID3D12Device>& device);

	void SetCBV(ComPtr<ID3D12Device>& device, D3D12_CPU_DESCRIPTOR_HANDLE srcHandle, CBV_REGISTER reg);
	void SetSRV(ComPtr<ID3D12Device>& device, D3D12_CPU_DESCRIPTOR_HANDLE srcHandle, SRV_REGISTER reg);
	void SetSampleView(ComPtr<ID3D12Device>& device, D3D12_CPU_DESCRIPTOR_HANDLE srcHandle, SAMPLE_REGISTER reg);

	void CommitTable(ComPtr<ID3D12GraphicsCommandList> commandList);

	ComPtr<ID3D12DescriptorHeap> GetDescriptorHeap() { return m_descHeap; }

	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle(CBV_REGISTER reg);
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle(SRV_REGISTER reg);

private:
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle(UINT8 reg);
private:

	ComPtr<ID3D12DescriptorHeap> m_descHeap;
	ComPtr<ID3D12DescriptorHeap> m_sampleDescHeap;
	
	int	m_handleSize = 0;
	int	m_sampleHandleSize = 0;
};
}