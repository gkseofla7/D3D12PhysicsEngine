#pragma once
#include <d3d12.h>
#include "d3dx12.h"
#include <d3dcompiler.h>
#include <dxgi1_6.h>
#include "DirectXTex.h"
#include "DirectXTex.inl"
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <windows.h>
#include <wrl/client.h> // ComPtr
#include <unordered_map>
#include <string>
//#include "ThreadPool.h"
#include <memory>
#include <array>

#include <filesystem>
namespace fs = std::filesystem;

#include <directxtk/SimpleMath.h>
namespace hlab {
using int8 = __int8;
using int16 = __int16;
using int32 = __int32;
using int64 = __int64;
using uint8 = unsigned __int8;
using uint16 = unsigned __int16;
using uint32 = unsigned __int32;
using uint64 = unsigned __int64;

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

enum class RENDER_TARGET_GROUP_TYPE : uint8
{
	SWAP_CHAIN, // BACK_BUFFER, FRONT_BUFFER
	SHADOW, // SHADOW
	G_BUFFER, // POSITION, NORMAL, COLOR
	LIGHTING, // DIFFUSE LIGHT, SPECULAR LIGHT	
	END,
};

enum
{
	SWAP_CHAIN_BUFFER_COUNT = 3,
	CBV_REGISTER_COUNT = CBV_REGISTER::END,
	SRV_REGISTER_COUNT = static_cast<uint8>(SRV_REGISTER::END) - CBV_REGISTER_COUNT,
	CBV_SRV_REGISTER_COUNT = CBV_REGISTER_COUNT + SRV_REGISTER_COUNT,
	//UAV_REGISTER_COUNT = static_cast<uint8>(UAV_REGISTER::END) - CBV_SRV_REGISTER_COUNT,
	//TOTAL_REGISTER_COUNT = CBV_SRV_REGISTER_COUNT + UAV_REGISTER_COUNT
	RENDER_TARGET_GROUP_COUNT = static_cast<uint8>(RENDER_TARGET_GROUP_TYPE::END)
};



struct WindowInfo
{
	HWND	hwnd; // 출력 윈도우
	int32	width; // 너비
	int32	height; // 높이
	bool	windowed; // 창모드 or 전체화면
};

#define DEVICE				GEngine->GetDevice()->GetDevice()
#define ROOTSIGNATURE		GEngine->GetRootSignature()
#define SHADER				GEngine->GetShader()
#define GRAPHICS_CMD_LIST	GEngine->GetGraphicsCmdQueue()->GetGraphicsCmdList()
#define RESOURCE_CMD_LIST	GEngine->GetGraphicsCmdQueue()->GetResourceCmdList()
#define BACKBUFFER_INDEX	GEngine->GetSwapChain()->GetBackBufferIndex()

#define FRAMEBUFFER_COUNT 3

using Microsoft::WRL::ComPtr;
using std::shared_ptr;
using std::wstring;
using std::vector;
using std::array;
using std::unique_ptr;
using DirectX::SimpleMath::Vector4;

extern std::unique_ptr<class Engine> GEngine;


inline void ThrowIfFailed2(HRESULT hr) {
	if (FAILED(hr)) {
		throw std::exception();
	}
}
}
/*
		{
		if (error) {
			// 에러 블롭에서 메시지 추출
			const char* errorMsg = static_cast<const char*>(error->GetBufferPointer());
			// 디버그 출력
			OutputDebugStringA("Root Signature Serialization Error:\n");
			OutputDebugStringA(errorMsg);
			// 콘솔 출력
			std::cerr << "Root Signature Serialization Error: " << errorMsg << std::endl;
		}
	}*/