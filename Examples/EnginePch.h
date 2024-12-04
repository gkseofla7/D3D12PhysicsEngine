#pragma once
#include <d3d12.h>
#include "d3dx12.h"
#include <d3dcompiler.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <windows.h>
#include <wrl/client.h> // ComPtr
#include <unordered_map>
#include "ThreadPool.h"
#include <memory>
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

#define DEVICE				GEngine->GetDevice()
#define GRAPHICS_CMD_LIST	GEngine->GetGraphicsCmdQueue()->GetGraphicsCmdList()
#define RESOURCE_CMD_LIST	GEngine->GetGraphicsCmdQueue()->GetResourceCmdList()

extern std::unique_ptr<class DAppBase> GEngine;
}