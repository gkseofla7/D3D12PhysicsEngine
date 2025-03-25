#include "Device.h"
#include <d3d12.h>
#include <dxgi1_6.h>
#include <d3d12sdklayers.h>
#include <wrl.h>
namespace dengine {
void Device::Init()
{
	// D3D12 디버그층 활성화
	// - VC++ 출력창에 상세한 디버깅 메시지 출력
	// - riid : 디바이스의 COM ID
	// - ppDevice : 생성된 장치가 매개변수에 설정
#ifdef _DEBUG
	/*D3D12GetDebugInterface(IID_PPV_ARGS(&m_debugController));
	m_debugController->EnableDebugLayer();

	ID3D12Debug5* pDebugController5 = nullptr;
	if (S_OK == m_debugController->QueryInterface(IID_PPV_ARGS(&pDebugController5)))
	{
		pDebugController5->SetEnableGPUBasedValidation(TRUE);
		pDebugController5->SetEnableAutoName(TRUE);
		pDebugController5->Release();
	}*/
#endif

	// DXGI(DirectX Graphics Infrastructure)
	// Direct3D와 함께 쓰이는 API
	// - 전체 화면 모드 전환
	// - 지원되는 디스플레이 모드 열거 등
	// CreateDXGIFactory
	// - riid : 디바이스의 COM ID
	// - ppDevice : 생성된 장치가 매개변수에 설정
	CreateDXGIFactory(IID_PPV_ARGS(&m_dxgi));

	// CreateDevice
	// - 디스플레이 어댑터(그래픽 카드)를 나타내는 객체
	// - pAdapter : nullptr 지정하면 시스템 기본 디스플레이 어댑터
	// - MinimumFeatureLevel : 응용 프로그램이 요구하는 최소 기능 수준 (구닥다리 걸러낸다)
	// - riid : 디바이스의 COM ID
	// - ppDevice : 생성된 장치가 매개변수에 설정
	D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_device));
}
}
