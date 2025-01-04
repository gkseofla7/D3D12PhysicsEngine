#pragma once
#include "EnginePch.h"
#include "Engine.h"
#include "Device.h"
// "Common.hlsli"와 동일해야 함
#define LIGHT_OFF 0x00
#define LIGHT_DIRECTIONAL 0x01
#define LIGHT_POINT 0x02
#define LIGHT_SPOT 0x04
#define LIGHT_SHADOW 0x10

namespace dengine {

using DirectX::SimpleMath::Matrix;
using DirectX::SimpleMath::Vector2;
using DirectX::SimpleMath::Vector3;
	// DirectX-Graphics-Samples/MiniEngine을 따라서 파일이름 변경
// __declspec(align(256)) : DX12에서는 256 align (예습)

// 주로 Vertex/Geometry 쉐이더에서 사용
__declspec(align(256)) struct MeshConstants2
{
    Matrix world;
    Matrix worldIT;
    Matrix worldInv;
    int useHeightMap = 0;
    float heightScale = 0.0f;
    float windTrunk = 0.0f;
    float windLeaves = 0.0f;
};

// 주로 Pixel 쉐이더에서 사용
__declspec(align(256)) struct MaterialConstants2
{

    Vector3 albedoFactor = Vector3(1.0f);
    float roughnessFactor = 1.0f;
    float metallicFactor = 1.0f;
    Vector3 emissionFactor = Vector3(0.0f);

    // 여러 옵션들에 uint 플래그 하나만 사용할 수도 있습니다.
    int useAlbedoMap = 0;
    int useNormalMap = 0;
    int useAOMap = 0;
    int invertNormalMapY = 0;
    int useMetallicMap = 0;
    int useRoughnessMap = 0;
    int useEmissiveMap = 0;
    float dummy = 0.0f;

    // 참고 flags 구현
    /* union {
        uint32_t flags;
        struct {
            // UV0 or UV1 for each texture
            uint32_t baseColorUV : 1;
            uint32_t metallicRoughnessUV : 1;
            uint32_t occlusionUV : 1;
            uint32_t emissiveUV : 1;
            uint32_t normalUV : 1;

            // Three special modes
            uint32_t twoSided : 1;
            uint32_t alphaTest : 1;
            uint32_t alphaBlend : 1;

            uint32_t _pad : 8;

            uint32_t alphaRef : 16; // half float
        };
    };*/
};

struct Light2
{
    Vector3 radiance = Vector3(5.0f); // strength
    float fallOffStart = 0.0f;
    Vector3 direction = Vector3(0.0f, 0.0f, 1.0f);
    float fallOffEnd = 20.0f;
    Vector3 position = Vector3(0.0f, 0.0f, -2.0f);
    float spotPower = 6.0f;

    // Light type bitmasking
    // ex) LIGHT_SPOT | LIGHT_SHADOW
    uint32_t type = LIGHT_OFF;
    float radius = 0.035f; // 반지름

    float haloRadius = 0.0f;
    float haloStrength = 0.0f;

    Matrix viewProj; // 그림자 렌더링에 필요
    Matrix invProj;  // 그림자 렌더링 디버깅용
};

// register(b1) 사용
__declspec(align(256)) struct GlobalConstants {
    Matrix view;
    Matrix proj;
    Matrix invProj; // 역프로젝션행렬
    Matrix viewProj;
    Matrix invViewProj; // Proj -> World
    Matrix invView;

    Vector3 eyeWorld;
    float strengthIBL = 0.0f;

    int textureToDraw = 0; // 0: Env, 1: Specular, 2: Irradiance, 그외: 검은색
    float envLodBias = 0.0f; // 환경맵 LodBias
    float lodBias = 2.0f;    // 다른 물체들 LodBias
    float globalTime = 0.0f;

    Light2 lights[MAX_LIGHTS_COUNT];
};

// bone 개수 제약을 없애기 위해 StructuredBuffer로 교체
// __declspec(align(256)) struct SkinnedConsts {
//    Matrix boneTransforms[52]; // bone 개수
//};

enum class CONSTANT_BUFFER_TYPE : uint8
{
	GLOBAL,
	TRANSFORM,
	MATERIAL,
	END
};

enum
{
	CONSTANT_BUFFER_COUNT = static_cast<uint8>(CONSTANT_BUFFER_TYPE::END)
};

template <typename T_CONSTS>
class ConstantBuffer
{
public:
    ~ConstantBuffer()
    {
        if (m_cbvBuffer)
        {
            if (m_cbvBuffer != nullptr)
                m_cbvBuffer->Unmap(0, nullptr);

            m_cbvBuffer = nullptr;
        }
    }

    void Init(CBV_REGISTER reg, uint32 count)
    {
        m_reg = reg;

        // 상수 버퍼는 256 바이트 배수로 만들어야 한다
        // 0 256 512 768
        m_elementSize = (sizeof(T_CONSTS) + 255) & ~255;
        m_elementCount = count;

        CreateBuffer();
        CreateView();
    }

    void Clear()
    {
        m_currentIndex = 0;
    }

    void Upload()
    {
        assert(m_currentIndex < m_elementCount);

        ::memcpy(&m_mappedBuffer[m_currentIndex * m_elementSize], &m_cpu, m_elementSize);
    }

    void PushGraphicsData(bool bIncreaseIndex = true)
    {
        D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = GetCpuHandle(m_currentIndex);
        GEngine->GetGraphicsDescHeap()->SetCBV(cpuHandle, m_reg);
        if (bIncreaseIndex)
        {
            m_currentIndex = (m_currentIndex + 1) % m_elementCount;
        }
    }

    D3D12_GPU_VIRTUAL_ADDRESS GetGpuVirtualAddress(uint32 index)
    {
        D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = m_cbvBuffer->GetGPUVirtualAddress();
        objCBAddress += index * m_elementSize;
        return objCBAddress;
    }

    D3D12_CPU_DESCRIPTOR_HANDLE GetCpuHandle(uint32 index)
    {
        return CD3DX12_CPU_DESCRIPTOR_HANDLE(m_cpuHandleBegin, index * m_handleIncrementSize);
    }

    T_CONSTS& GetCpu() { return m_cpu; }

private:
    void CreateBuffer()
    {
        uint32 bufferSize = m_elementSize * m_elementCount;
        D3D12_HEAP_PROPERTIES heapProperty = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
        D3D12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);

        DEVICE->CreateCommittedResource(
            &heapProperty,
            D3D12_HEAP_FLAG_NONE,
            &desc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&m_cbvBuffer));

        m_cbvBuffer->Map(0, nullptr, reinterpret_cast<void**>(&m_mappedBuffer));
        // We do not need to unmap until we are done with the resource.  However, we must not write to
        // the resource while it is in use by the GPU (so we must use synchronization techniques).
    }
    void CreateView()
    {
        D3D12_DESCRIPTOR_HEAP_DESC cbvDesc = {};
        cbvDesc.NumDescriptors = m_elementCount;
        cbvDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;// TODO.
        cbvDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        DEVICE->CreateDescriptorHeap(&cbvDesc, IID_PPV_ARGS(&m_cbvHeap));

        m_cpuHandleBegin = m_cbvHeap->GetCPUDescriptorHandleForHeapStart();
        m_handleIncrementSize = DEVICE->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

        for (uint32 i = 0; i < m_elementCount; ++i)
        {
            D3D12_CPU_DESCRIPTOR_HANDLE cbvHandle = GetCpuHandle(i);

            D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
            cbvDesc.BufferLocation = m_cbvBuffer->GetGPUVirtualAddress() + static_cast<uint64>(m_elementSize) * i;
            cbvDesc.SizeInBytes = m_elementSize;   // CB size is required to be 256-byte aligned.

            DEVICE->CreateConstantBufferView(&cbvDesc, cbvHandle);
        }
    }

private:
    T_CONSTS m_cpu;
	ComPtr<ID3D12Resource>	m_cbvBuffer;

	BYTE*					m_mappedBuffer = nullptr;
	uint32					m_elementSize = 0;
	uint32					m_elementCount = 0;

	ComPtr<ID3D12DescriptorHeap>		m_cbvHeap;
	D3D12_CPU_DESCRIPTOR_HANDLE			m_cpuHandleBegin = {};
	uint32								m_handleIncrementSize = 0;

	uint32					m_currentIndex = 0;

	CBV_REGISTER			m_reg = {};
};


}
