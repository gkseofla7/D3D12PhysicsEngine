#include "Texture.h"
#include "Engine.h"
#include "CommandQueue.h"
#include "Device.h"
#include "D3D12Utils.h"
//#include "Engine.h"
namespace dengine {
Texture::Texture()
{

}

Texture::~Texture()
{

}

void Texture::Load(const wstring& path, bool isCubeMap)
{
	m_isCubeMap = isCubeMap;
	D3D12Utils::LoadTextureAsync(path, false, shared_from_this());
}

void Texture::Create(D3D12_RESOURCE_DESC resourceDesc, const D3D12_HEAP_PROPERTIES& heapProperty,
	D3D12_HEAP_FLAGS heapFlags, D3D12_RESOURCE_FLAGS resFlags, Vector4 clearColor)
{
	resourceDesc.Flags = resFlags;
	D3D12Utils::CreateTexture(resourceDesc, heapProperty, heapFlags, resFlags, clearColor, shared_from_this());
}

void Texture::CreateFromResource(ComPtr<ID3D12Resource> tex2D)
{
	m_tex2D = tex2D;
	m_desc = tex2D->GetDesc();
	m_format = tex2D->GetDesc().Format;
	// 주요 조합
	// - DSV 단독 (조합X)
	// - SRV
	// - RTV + SRV
	if (m_desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL)
	{
		// DSV
		D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
		heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		heapDesc.NumDescriptors = 1;
		heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		heapDesc.NodeMask = 0;
		DEVICE->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_dsvHeap));

		D3D12_CPU_DESCRIPTOR_HANDLE hDSVHandle = m_dsvHeap->GetCPUDescriptorHandleForHeapStart();
		DEVICE->CreateDepthStencilView(m_tex2D.Get(), nullptr, hDSVHandle);
	}
	else
	{
		if (m_desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET)
		{
			// RTV
			D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
			heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
			heapDesc.NumDescriptors = 1;
			heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
			heapDesc.NodeMask = 0;
			DEVICE->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_rtvHeap));

			D3D12_CPU_DESCRIPTOR_HANDLE rtvHeapBegin = m_rtvHeap->GetCPUDescriptorHandleForHeapStart();
			DEVICE->CreateRenderTargetView(m_tex2D.Get(), nullptr, rtvHeapBegin);
		}

		if (m_desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS)
		{
			// UAV
			D3D12_DESCRIPTOR_HEAP_DESC uavHeapDesc = {};
			uavHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
			uavHeapDesc.NumDescriptors = 1;
			uavHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
			uavHeapDesc.NodeMask = 0;
			DEVICE->CreateDescriptorHeap(&uavHeapDesc, IID_PPV_ARGS(&m_uavHeap));

			m_uavHeapBegin = m_uavHeap->GetCPUDescriptorHandleForHeapStart();

			D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
			uavDesc.Format = m_format; 
			uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;

			DEVICE->CreateUnorderedAccessView(m_tex2D.Get(), nullptr, &uavDesc, m_uavHeapBegin);
		}

		// SRV
		D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
		srvHeapDesc.NumDescriptors = 1;
		srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		DEVICE->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&m_srvHeap));

		m_srvHeapBegin = m_srvHeap->GetCPUDescriptorHandleForHeapStart();

		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = m_format;

		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Texture2D.MipLevels = 1;
		if (m_tex2D->GetDesc().DepthOrArraySize == 6 && m_isCubeMap)
		{
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
			srvDesc.TextureCube.MostDetailedMip = 0;
			srvDesc.TextureCube.MipLevels = m_tex2D->GetDesc().MipLevels;
			srvDesc.TextureCube.ResourceMinLODClamp = 0.0f;
		}
		else 
		{
			if (m_desc.SampleDesc.Count > 1)
			{
				srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMS;
			}
			else
			{
				srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			}
			srvDesc.Texture2D.MostDetailedMip = 0;
			srvDesc.Texture2D.MipLevels = m_tex2D->GetDesc().MipLevels;
			srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
		}
		DEVICE->CreateShaderResourceView(m_tex2D.Get(), &srvDesc, m_srvHeapBegin);

		SetLoadType(ELoadType::Loaded);
	}
}

D3D12_CPU_DESCRIPTOR_HANDLE Texture::GetSRVHandle()
{
	if (IsLoaded())
	{
		return m_srvHeapBegin;
	}
	else
	{
		return GEngine->GetDefaultTexture()->GetSRVHandle();
	}
}
D3D12_CPU_DESCRIPTOR_HANDLE Texture::GetUAVHandle()
{
	if (IsLoaded())
	{
		return m_uavHeapBegin;
	}
	else
	{
		return GEngine->GetDefaultTexture()->GetUAVHandle();
	}
}

}
