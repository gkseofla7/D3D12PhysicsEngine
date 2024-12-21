#include "Shader.h"
#include "Engine.h"
#include "D3D12Utils.h"
#include "Device.h"
namespace dengine {
Shader::Shader()
{

}

Shader::~Shader()
{

}

void Shader::Init()
{
	// Vertex Shader
	D3D12Utils::CreateVertexShader(DEVICE, L"BasicVS.hlsl", m_basicVS);
	D3D12Utils::CreateVertexShader(DEVICE, L"BasicVS.hlsl", m_skinnedVS,
		vector<D3D_SHADER_MACRO>{ {"SKINNED", "1"}, { NULL, NULL }});
	D3D12Utils::CreateVertexShader(DEVICE, L"SkyboxVS.hlsl", m_skyboxVS);
	D3D12Utils::CreateVertexShader(DEVICE, L"SamplingVS.hlsl", m_samplingVS);


	// Pixel Shader
	D3D12Utils::CreatePixelShader(DEVICE, L"BasicPS.hlsl", m_basicPS);
	D3D12Utils::CreatePixelShader(DEVICE, L"SkyboxPS.hlsl", m_skyboxPS);
	D3D12Utils::CreatePixelShader(DEVICE, L"SamplingPS.hlsl", m_samplingPS);
	D3D12Utils::CreatePixelShader(DEVICE, L"PostEffectsPS.hlsl", m_postEffectsPS);
}


//void Shader::Update()
//{
//	if (GetShaderType() == SHADER_TYPE::COMPUTE)
//		COMPUTE_CMD_LIST->SetPipelineState(_pipelineState.Get());
//	else
//	{
//		GRAPHICS_CMD_LIST->IASetPrimitiveTopology(_info.topology);
//		GRAPHICS_CMD_LIST->SetPipelineState(_pipelineState.Get());
//	}	
//}


}
