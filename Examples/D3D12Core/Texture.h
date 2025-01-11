#pragma once
#include "EnginePch.h"
#include "Resource.h"
namespace dengine {
using namespace DirectX;
class Texture : public Resource
{
public:
	Texture();
	virtual ~Texture();

	void Load(const wstring& path, const bool async = true, const bool isCubeMap = false, const bool usSRGB = false);
	virtual shared_ptr<Texture> GetTexture() { return std::dynamic_pointer_cast<Texture>(shared_from_this()); }
public:
	void Create(D3D12_RESOURCE_DESC resourceDesc, const D3D12_HEAP_PROPERTIES& heapProperty, 
		D3D12_HEAP_FLAGS heapFlags, D3D12_RESOURCE_FLAGS resFlags, Vector4 clearColor = Vector4());

	void CreateFromResource(ComPtr<ID3D12Resource> tex2D);
public:
	ComPtr<ID3D12Resource> GetTex2D() { return m_tex2D; }
	ComPtr<ID3D12DescriptorHeap> GetSRV() { return m_srvHeap; }
	ComPtr<ID3D12DescriptorHeap> GetRTV() { return m_rtvHeap; }
	ComPtr<ID3D12DescriptorHeap> GetDSV() { return m_dsvHeap; }
	ComPtr<ID3D12DescriptorHeap> GetUAV() { return m_uavHeap; }

	D3D12_CPU_DESCRIPTOR_HANDLE GetSRVHandle();
	D3D12_CPU_DESCRIPTOR_HANDLE GetUAVHandle();

	float GetWidth() { return static_cast<float>(m_desc.Width); }
	float GetHeight() { return static_cast<float>(m_desc.Height); }

private:
	DXGI_FORMAT						m_format;
	D3D12_RESOURCE_DESC				m_desc;
	ComPtr<ID3D12Resource>			m_tex2D;
	bool							m_isCubeMap = false;

	ComPtr<ID3D12DescriptorHeap>	m_srvHeap;
	ComPtr<ID3D12DescriptorHeap>	m_rtvHeap;
	ComPtr<ID3D12DescriptorHeap>	m_dsvHeap;
	ComPtr<ID3D12DescriptorHeap>	m_uavHeap;

private:
	D3D12_CPU_DESCRIPTOR_HANDLE		m_srvHeapBegin = {};
	D3D12_CPU_DESCRIPTOR_HANDLE		m_uavHeapBegin = {};
};


}
