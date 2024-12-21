#include "Texture.h"
#include "Engine.h"
#include "CommandQueue.h"
#include "Device.h"
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
	// 파일 확장자 얻기
	wstring ext = fs::path(path).extension();

	if (ext == L".dds" || ext == L".DDS")
		DirectX::LoadFromDDSFile(path.c_str(), DDS_FLAGS_NONE, nullptr, m_image);
	else if (ext == L".tga" || ext == L".TGA")
		DirectX::LoadFromTGAFile(path.c_str(), nullptr, m_image);
	else // png, jpg, jpeg, bmp
		DirectX::LoadFromWICFile(path.c_str(), WIC_FLAGS_NONE, nullptr, m_image);

	HRESULT hr = DirectX::CreateTexture(DEVICE.Get(), m_image.GetMetadata(), &m_tex2D);
	if (FAILED(hr))
		assert(nullptr);

	m_desc = m_tex2D->GetDesc();

	vector<D3D12_SUBRESOURCE_DATA> subResources;

	hr = DirectX::PrepareUpload(DEVICE.Get(),
		m_image.GetImages(),
		m_image.GetImageCount(),
		m_image.GetMetadata(),
		subResources);

	if (FAILED(hr))
		assert(nullptr);

	const uint64 bufferSize = ::GetRequiredIntermediateSize(m_tex2D.Get(), 0, static_cast<uint32>(subResources.size()));

	D3D12_HEAP_PROPERTIES heapProperty = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	D3D12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);

	ComPtr<ID3D12Resource> textureUploadHeap;
	hr = DEVICE->CreateCommittedResource(
		&heapProperty,
		D3D12_HEAP_FLAG_NONE,
		&desc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(textureUploadHeap.GetAddressOf()));

	if (FAILED(hr))
		assert(nullptr);

	::UpdateSubresources(RESOURCE_CMD_LIST.Get(),
		m_tex2D.Get(),
		textureUploadHeap.Get(),
		0, 0,
		static_cast<unsigned int>(subResources.size()),
		subResources.data());

	GEngine->GetGraphicsCmdQueue()->FlushResourceCommandQueue();

	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
	srvHeapDesc.NumDescriptors = 1;
	srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;


	DEVICE->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&m_srvHeap));

	m_srvHeapBegin = m_srvHeap->GetCPUDescriptorHandleForHeapStart();

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = m_image.GetMetadata().format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Texture2D.MipLevels = 1;
	if (m_tex2D->GetDesc().DepthOrArraySize == 6 && isCubeMap) {
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
		srvDesc.TextureCube.MostDetailedMip = 0;
		srvDesc.TextureCube.MipLevels = m_tex2D->GetDesc().MipLevels;
		srvDesc.TextureCube.ResourceMinLODClamp = 0.0f;
	}
	else {
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels = m_tex2D->GetDesc().MipLevels;
		srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
	}
	DEVICE->CreateShaderResourceView(m_tex2D.Get(), &srvDesc, m_srvHeapBegin);
}

void Texture::Create(DXGI_FORMAT format, uint32 width, uint32 height,
	const D3D12_HEAP_PROPERTIES& heapProperty, D3D12_HEAP_FLAGS heapFlags,
	D3D12_RESOURCE_FLAGS resFlags, Vector4 clearColor)
{
	m_desc = CD3DX12_RESOURCE_DESC::Tex2D(format, width, height);
	m_desc.Flags = resFlags;

	D3D12_CLEAR_VALUE optimizedClearValue = {};
	D3D12_CLEAR_VALUE* pOptimizedClearValue = nullptr;

	D3D12_RESOURCE_STATES resourceStates = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COMMON;

	if (resFlags & D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL)
	{
		resourceStates = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_DEPTH_WRITE;
		optimizedClearValue = CD3DX12_CLEAR_VALUE(DXGI_FORMAT_D32_FLOAT, 1.0f, 0);
		pOptimizedClearValue = &optimizedClearValue;
	}
	else if (resFlags & D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET)
	{
		resourceStates = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COMMON;
		float arrFloat[4] = { clearColor.x, clearColor.y, clearColor.z, clearColor.w };
		optimizedClearValue = CD3DX12_CLEAR_VALUE(format, arrFloat);
		pOptimizedClearValue = &optimizedClearValue;
	}

	// Create Texture2D
	HRESULT hr = DEVICE->CreateCommittedResource(
		&heapProperty,
		heapFlags,
		&m_desc,
		resourceStates,
		pOptimizedClearValue,
		IID_PPV_ARGS(&m_tex2D));

	assert(SUCCEEDED(hr));

	CreateFromResource(m_tex2D);
}
void Texture::Create(D3D12_RESOURCE_DESC resourceDesc, const D3D12_HEAP_PROPERTIES& heapProperty,
	D3D12_HEAP_FLAGS heapFlags, D3D12_RESOURCE_FLAGS resFlags, Vector4 clearColor)
{
	m_desc = resourceDesc;
	D3D12_RESOURCE_STATES resourceStates = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COMMON;
	float arrFloat[4] = { clearColor.x, clearColor.y, clearColor.z, clearColor.w };
	D3D12_CLEAR_VALUE optimizedClearValue = CD3DX12_CLEAR_VALUE(m_desc.Format, arrFloat);
	D3D12_CLEAR_VALUE* pOptimizedClearValue = &optimizedClearValue;
	// Create Texture2D
	HRESULT hr = DEVICE->CreateCommittedResource(
		&heapProperty,
		heapFlags,
		&m_desc,
		resourceStates,
		pOptimizedClearValue,
		IID_PPV_ARGS(&m_tex2D));

	assert(SUCCEEDED(hr));

	CreateFromResource(m_tex2D);
}

void Texture::CreateFromResource(ComPtr<ID3D12Resource> tex2D)
{
	m_tex2D = tex2D;

	m_desc = tex2D->GetDesc();

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
			uavDesc.Format = m_image.GetMetadata().format;
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
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Format = m_image.GetMetadata().format;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;
		DEVICE->CreateShaderResourceView(m_tex2D.Get(), &srvDesc, m_srvHeapBegin);
	}
}


}
