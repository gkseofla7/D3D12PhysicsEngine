#pragma once
#include "EnginePch.h"
namespace hlab {
using namespace DirectX;
class Texture
{
public:
	Texture();
	virtual ~Texture();

	void Load(const wstring& path);

public:
	void Create(DXGI_FORMAT format, uint32 width, uint32 height,
		const D3D12_HEAP_PROPERTIES& heapProperty, D3D12_HEAP_FLAGS heapFlags,
		D3D12_RESOURCE_FLAGS resFlags, Vector4 clearColor = Vector4());

	void CreateFromResource(ComPtr<ID3D12Resource> tex2D);

public:
	ComPtr<ID3D12Resource> GetTex2D() { return m_tex2D; }
	ComPtr<ID3D12DescriptorHeap> GetSRV() { return m_srvHeap; }
	ComPtr<ID3D12DescriptorHeap> GetRTV() { return m_rtvHeap; }
	ComPtr<ID3D12DescriptorHeap> GetDSV() { return m_dsvHeap; }
	ComPtr<ID3D12DescriptorHeap> GetUAV() { return m_uavHeap; }

	D3D12_CPU_DESCRIPTOR_HANDLE GetSRVHandle() { return m_srvHeapBegin; }
	D3D12_CPU_DESCRIPTOR_HANDLE GetUAVHandle() { return m_uavHeapBegin; }

	float GetWidth() { return static_cast<float>(m_desc.Width); }
	float GetHeight() { return static_cast<float>(m_desc.Height); }

private:
	ScratchImage			 		m_image;
	D3D12_RESOURCE_DESC				m_desc;
	ComPtr<ID3D12Resource>			m_tex2D;

	ComPtr<ID3D12DescriptorHeap>	m_srvHeap;
	ComPtr<ID3D12DescriptorHeap>	m_rtvHeap;
	ComPtr<ID3D12DescriptorHeap>	m_dsvHeap;
	ComPtr<ID3D12DescriptorHeap>	m_uavHeap;

private:
	D3D12_CPU_DESCRIPTOR_HANDLE		m_srvHeapBegin = {};
	D3D12_CPU_DESCRIPTOR_HANDLE		m_uavHeapBegin = {};
};


}
