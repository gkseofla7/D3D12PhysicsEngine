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
	ComPtr<ID3DBlob> GetSamplinigVS() { return m_samplingVS; }
	ComPtr<ID3DBlob> GetDepthOnlyVS() { return m_depthOnlyVS; }
	ComPtr<ID3DBlob> GetDepthOnlySkinnedVS() { return m_depthOnlySkinnedVS; }


	ComPtr<ID3DBlob> GetBasicPS() { return m_basicPS; }
	ComPtr<ID3DBlob> GetSkyboxPS() { return m_skyboxPS; }
	ComPtr<ID3DBlob> GetSamplingPS() { return m_samplingPS; }
	ComPtr<ID3DBlob> GetPostEffectsPS() { return m_postEffectsPS; }
	ComPtr<ID3DBlob> GetDepthOnlyPS() { return m_depthOnlyPS; }
	//void Update();


	//static D3D12_PRIMITIVE_TOPOLOGY_TYPE GetTopologyType(D3D_PRIMITIVE_TOPOLOGY topology);

private:
	// GraphicsShader
	ComPtr<ID3DBlob> m_basicVS;
	ComPtr<ID3DBlob> m_skinnedVS;
	ComPtr<ID3DBlob> m_skyboxVS;
	ComPtr<ID3DBlob> m_samplingVS;
	ComPtr<ID3DBlob> m_depthOnlyVS;
	ComPtr<ID3DBlob> m_depthOnlySkinnedVS;

	ComPtr<ID3DBlob> m_basicPS;
	ComPtr<ID3DBlob> m_skyboxPS;
	ComPtr<ID3DBlob> m_samplingPS;
	ComPtr<ID3DBlob> m_postEffectsPS;
	ComPtr<ID3DBlob> m_depthOnlyPS;
};


}
