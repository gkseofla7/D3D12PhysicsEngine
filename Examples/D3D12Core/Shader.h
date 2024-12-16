#pragma once
#include "EnginePch.h"

namespace dengine {
class Shader
{
public:
	Shader();
	virtual ~Shader();

	void Init();
	
	ComPtr<ID3DBlob> GetBasicVS() { return m_basicVS; }
	ComPtr<ID3DBlob> GetSkinnedVS() { return m_skinnedVS; }
	ComPtr<ID3DBlob> GetSkyboxVS() { return m_skyboxVS; }


	ComPtr<ID3DBlob> GetBasicPS() { return m_basicPS; }
	ComPtr<ID3DBlob> GetSkyboxPS() { return m_skyboxPS; }
	//void Update();


	//static D3D12_PRIMITIVE_TOPOLOGY_TYPE GetTopologyType(D3D_PRIMITIVE_TOPOLOGY topology);

private:
	// GraphicsShader
	ComPtr<ID3DBlob> m_basicVS;
	ComPtr<ID3DBlob> m_skinnedVS;
	ComPtr<ID3DBlob> m_skyboxVS;

	ComPtr<ID3DBlob> m_basicPS;
	ComPtr<ID3DBlob> m_skyboxPS;

};


}
