#pragma once
#include "D3D12Utils.h"
#include "EnginePch.h"
namespace hlab {
using DirectX::SimpleMath::Vector4;
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

	CD3DX12_CPU_DESCRIPTOR_HANDLE GetSRVHandle() { return m_srvHandle; }

	float GetWidth() { return static_cast<float>(_desc.Width); }
	float GetHeight() { return static_cast<float>(_desc.Height); }

private:
	ScratchImage			 		_image;
	D3D12_RESOURCE_DESC				_desc;
	ComPtr<ID3D12Resource>			m_tex2D;

	

private:
	CD3DX12_CPU_DESCRIPTOR_HANDLE		m_srvHandle;
};

}

