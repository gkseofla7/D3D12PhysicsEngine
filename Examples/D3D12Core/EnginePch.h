#pragma once
#include <d3dx12.h>
#include <d3d12.h>
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
#include <thread>
//#include "ThreadPool.h"
#include <array>
#include <comdef.h>  // _com_error를 사용하여 HRESULT 메시지 변환
#include <filesystem>
namespace fs = std::filesystem;

#include <directxtk/SimpleMath.h>
namespace dengine {
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
	t15_1,
	t15_2,
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
	FLOAT,
	RESOLVE,
	LIGHTING, // DIFFUSE LIGHT, SPECULAR LIGHT	
	G_BUFFER,
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
	RENDER_TARGET_GROUP_COUNT = static_cast<uint8>(RENDER_TARGET_GROUP_TYPE::END),
	RENDER_TARGET_G_BUFFER_GROUP_MEMBER_COUNT = 3,
};

enum class PSOType
{
	DEFAULT,
	SKYBOX,
	SAMPLING,
	SHADOW
};

enum class ELoadType
{
	NotLoaded,
	Loading,
	Loaded,
};
enum class EResourceType
{
	None,
	Texture,
};

struct WindowInfo
{
	HWND	hwnd; // 출력 윈도우
	int32	width; // 너비
	int32	height; // 높이
	bool	windowed; // 창모드 or 전체화면
};

#define DECLARE_SINGLE(type)		\
private:							\
	type() {}						\
	~type() {}						\
public:								\
	static type* GetInstance()		\
	{								\
		static type instance;		\
		return &instance;			\
	}								\

#define GET_SINGLE(type)	type::GetInstance()

#define DEVICE				GEngine->GetDevice()->GetDevice()
#define ROOTSIGNATURE		GEngine->GetRootSignature()
#define SHADER				GEngine->GetShader()
#define GRAPHICS_CMD_LIST	GEngine->GetGraphicsCmdQueue()->GetCurrentGraphicsCmdList()
#define RESOURCE_CMD_LIST	GEngine->GetResourceCmdQueue()->GetResourceCmdList()
#define BACKBUFFER_INDEX	GEngine->GetSwapChain()->GetBackBufferIndex()
#define MAX_LIGHTS_COUNT 3
#define MAIN_THREAD_ID g_MainThreadId
using Microsoft::WRL::ComPtr;
using std::shared_ptr;
using std::wstring;
using std::vector;
using std::array;
using std::unique_ptr;
using DirectX::SimpleMath::Vector3;
using DirectX::SimpleMath::Vector4;
using DirectX::SimpleMath::Matrix;


extern std::unique_ptr<class Engine> GEngine;
extern std::thread::id g_MainThreadId;

//inline void ThrowIfFailed(HRESULT hr) {
//	if (FAILED(hr)) {
//		throw std::exception(); 
//	}
//}
inline void ThrowIfFailed(HRESULT hr, const char* function = "", const char* file = "", int line = 0) {
	if (FAILED(hr)) {

		// HRESULT를 문자열로 변환
		_com_error err(hr);
		const TCHAR* errorMessage = err.ErrorMessage();
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