#include "GraphicsPipelineState.h"
#include "Engine.h"
#include "Device.h"
#include "RootSignature.h"
#include "Shader.h"
namespace dengine {
void GraphicsPipelineState::Init()
{
	D3D12_INPUT_ELEMENT_DESC basicIEs[] = {
	{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
	 D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
	{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12,
	 D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
	{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24,
	 D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
	{"TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32,
	 D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
	};


	D3D12_RASTERIZER_DESC solidRSDesc; // front only
	ZeroMemory(&solidRSDesc, sizeof(D3D12_RASTERIZER_DESC));
	solidRSDesc = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	solidRSDesc.FillMode = D3D12_FILL_MODE::D3D12_FILL_MODE_SOLID;
	solidRSDesc.CullMode = D3D12_CULL_MODE::D3D12_CULL_MODE_BACK;
	solidRSDesc.FrontCounterClockwise = false;
	solidRSDesc.DepthClipEnable = true;
	solidRSDesc.MultisampleEnable = true;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.InputLayout = { basicIEs, _countof(basicIEs) };
	psoDesc.pRootSignature = ROOTSIGNATURE->GetGraphicsRootSignature().Get();
	psoDesc.VS = CD3DX12_SHADER_BYTECODE(SHADER->GetBasicVS().Get());
	psoDesc.PS = CD3DX12_SHADER_BYTECODE(SHADER->GetBasicPS().Get());
	psoDesc.RasterizerState = solidRSDesc;
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState.DepthEnable = FALSE;
	psoDesc.DepthStencilState.StencilEnable = FALSE;
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	//TODO RTV Format 다시 확인 필요
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.SampleDesc.Count = 1;
	ThrowIfFailed2(DEVICE->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_defaultPipelineState)));



	D3D12_GRAPHICS_PIPELINE_STATE_DESC skyboxRSDesc = psoDesc;
	skyboxRSDesc.VS = CD3DX12_SHADER_BYTECODE(SHADER->GetSkyboxVS().Get());
	skyboxRSDesc.PS = CD3DX12_SHADER_BYTECODE(SHADER->GetSkyboxPS().Get());
	ThrowIfFailed2(DEVICE->CreateGraphicsPipelineState(&skyboxRSDesc, IID_PPV_ARGS(&m_skyboxPipelineState)));
}
}