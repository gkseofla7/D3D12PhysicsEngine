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
	D3D12Utils::CreateVertexShader(DEVICE, L"D3D12Core/Shaders/BasicVS.hlsl", m_basicVS);
	D3D12Utils::CreateVertexShader(DEVICE, L"D3D12Core/Shaders/BasicVS.hlsl", m_skinnedVS,
		vector<D3D_SHADER_MACRO>{ {"SKINNED", "1"}, { NULL, NULL }});
	D3D12Utils::CreateVertexShader(DEVICE, L"D3D12Core/Shaders/BillboardVS.hlsl", m_billboardVS);
	D3D12Utils::CreateVertexShader(DEVICE, L"D3D12Core/Shaders/SkyboxVS.hlsl", m_skyboxVS);
	D3D12Utils::CreateVertexShader(DEVICE, L"D3D12Core/Shaders/SamplingVS.hlsl", m_samplingVS);
	D3D12Utils::CreateVertexShader(DEVICE, L"D3D12Core/Shaders/DepthOnlyVS.hlsl", m_depthOnlyVS);
	D3D12Utils::CreateVertexShader(DEVICE, L"D3D12Core/Shaders/DepthOnlyVS.hlsl", m_depthOnlySkinnedVS,
		vector<D3D_SHADER_MACRO>{ {"SKINNED", "1"}, { NULL, NULL }});


	// Geometry Shader
	D3D12Utils::CreateGeometryShader(DEVICE, L"D3D12Core/Shaders/BillboardGS.hlsl", m_billboardGS);

	// Pixel Shader
	D3D12Utils::CreatePixelShader(DEVICE, L"D3D12Core/Shaders/BasicPS.hlsl", m_basicPS);
	D3D12Utils::CreatePixelShader(DEVICE, L"D3D12Core/Shaders/FireballPS.hlsl", m_billboardPS);
	D3D12Utils::CreatePixelShader(DEVICE, L"D3D12Core/Shaders/SkyboxPS.hlsl", m_skyboxPS);
	D3D12Utils::CreatePixelShader(DEVICE, L"D3D12Core/Shaders/SamplingPS.hlsl", m_samplingPS);
	D3D12Utils::CreatePixelShader(DEVICE, L"D3D12Core/Shaders/PostEffectsPS.hlsl", m_postEffectsPS);
	D3D12Utils::CreatePixelShader(DEVICE, L"D3D12Core/Shaders/DepthOnlyPS.hlsl", m_depthOnlyPS);
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
