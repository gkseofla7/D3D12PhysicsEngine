#pragma once
#include "EnginePch.h"

namespace hlab {
class Shader
{
public:
	Shader();
	virtual ~Shader();

	void Init();
	
	ComPtr<ID3DBlob> GetBasicVS() { return m_basicVS;; }
	ComPtr<ID3DBlob> GetSkinnedVS() { return m_skinnedVS;; }
	ComPtr<ID3DBlob> GetBasicPS() { return m_basicPS;; }
	//void Update();


	//static D3D12_PRIMITIVE_TOPOLOGY_TYPE GetTopologyType(D3D_PRIMITIVE_TOPOLOGY topology);

private:
	// GraphicsShader
	ComPtr<ID3DBlob> m_basicVS;
	ComPtr<ID3DBlob> m_skinnedVS;
	ComPtr<ID3DBlob> m_basicPS;

};


}
