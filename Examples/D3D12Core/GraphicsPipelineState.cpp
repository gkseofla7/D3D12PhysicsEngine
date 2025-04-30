#include "GraphicsPipelineState.h"
#include "Engine.h"
#include "Device.h"
#include "RootSignature.h"
#include "Shader.h"
namespace dengine {
void GraphicsPipelineState::Init()
{
	const D3D12_INPUT_ELEMENT_DESC basicIEs[] = {
	{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
	D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
	{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12,
	D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
	{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24,
	D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
	{"TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32,
	D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
	};

	const D3D12_INPUT_ELEMENT_DESC skinnedIEs[] = {
	{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
	 D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
	{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12,
	 D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
	{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24,
	 D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
	{"TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32,
	 D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
	{"BLENDWEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 44,
	 D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
	{"BLENDWEIGHT", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 60,
	 D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
	{"BLENDINDICES", 0, DXGI_FORMAT_R8G8B8A8_UINT, 0, 76,
	 D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
	{"BLENDINDICES", 1, DXGI_FORMAT_R8G8B8A8_UINT, 0, 80,
	 D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
	};


	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	{
		D3D12_RASTERIZER_DESC solidRSDesc; // front only
		ZeroMemory(&solidRSDesc, sizeof(D3D12_RASTERIZER_DESC));
		solidRSDesc = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		solidRSDesc.FillMode = D3D12_FILL_MODE::D3D12_FILL_MODE_SOLID;
		solidRSDesc.CullMode = D3D12_CULL_MODE::D3D12_CULL_MODE_BACK;
		solidRSDesc.FrontCounterClockwise = false;
		solidRSDesc.DepthClipEnable = true;
		solidRSDesc.MultisampleEnable = true;

		psoDesc.InputLayout = { basicIEs, _countof(basicIEs) };
		psoDesc.pRootSignature = ROOTSIGNATURE->GetGraphicsRootSignature().Get();
		psoDesc.VS = CD3DX12_SHADER_BYTECODE(SHADER->GetBasicVS().Get());
		psoDesc.PS = CD3DX12_SHADER_BYTECODE(SHADER->GetBasicPS().Get());
		psoDesc.RasterizerState = solidRSDesc;
		psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);

		psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
		psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
		psoDesc.DepthStencilState.DepthEnable = true;
		psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL; // 깊이 쓰기 활성화
		psoDesc.DepthStencilState.StencilEnable = FALSE;
		

		psoDesc.SampleMask = UINT_MAX;
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		psoDesc.NumRenderTargets = 1;
		//TODO RTV Format 다시 확인 필요
		psoDesc.RTVFormats[0] = DXGI_FORMAT_R16G16B16A16_FLOAT;
		psoDesc.SampleDesc.Count = 4;
		ThrowIfFailed(DEVICE->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_defaultPipelineState)));
	}

	{
		D3D12_GRAPHICS_PIPELINE_STATE_DESC skinnedPsoDesc = psoDesc;
		skinnedPsoDesc.InputLayout = { skinnedIEs, _countof(skinnedIEs) };
		skinnedPsoDesc.VS = CD3DX12_SHADER_BYTECODE(SHADER->GetSkinnedVS().Get());
		ThrowIfFailed(DEVICE->CreateGraphicsPipelineState(&skinnedPsoDesc, IID_PPV_ARGS(&m_skinnedPipelineState)));
	}

	{
		//const D3D12_INPUT_ELEMENT_DESC billboardIEs[] = {
		//	{"POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, // Vector4
		//	 D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0} };
		//D3D12_GRAPHICS_PIPELINE_STATE_DESC billboardPsoDesc = psoDesc;
		//billboardPsoDesc.InputLayout = { billboardIEs, _countof(billboardIEs) };
		//billboardPsoDesc.pRootSignature = ROOTSIGNATURE->GetBillboardRootSignature().Get();
		//billboardPsoDesc.VS = CD3DX12_SHADER_BYTECODE(SHADER->GetBillboardVS().Get());
		//billboardPsoDesc.GS = CD3DX12_SHADER_BYTECODE(SHADER->GetBillboardGS().Get());
		//billboardPsoDesc.PS = CD3DX12_SHADER_BYTECODE(SHADER->GetBillboardPS().Get());
		//ThrowIfFailed(DEVICE->CreateGraphicsPipelineState(&billboardPsoDesc, IID_PPV_ARGS(&m_billboardPipelineState)));
	}

	{
		D3D12_GRAPHICS_PIPELINE_STATE_DESC skyboxRSDesc = psoDesc;
		skyboxRSDesc.pRootSignature = ROOTSIGNATURE->GetSkyboxRootSignature().Get();
		skyboxRSDesc.VS = CD3DX12_SHADER_BYTECODE(SHADER->GetSkyboxVS().Get());
		skyboxRSDesc.PS = CD3DX12_SHADER_BYTECODE(SHADER->GetSkyboxPS().Get());
		ThrowIfFailed(DEVICE->CreateGraphicsPipelineState(&skyboxRSDesc, IID_PPV_ARGS(&m_skyboxPipelineState)));
	}

	{
		D3D12_RASTERIZER_DESC rasterDesc; // front only
		ZeroMemory(&rasterDesc, sizeof(D3D12_RASTERIZER_DESC));
		rasterDesc = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		rasterDesc.FillMode = D3D12_FILL_MODE::D3D12_FILL_MODE_SOLID;
		rasterDesc.CullMode = D3D12_CULL_MODE::D3D12_CULL_MODE_BACK;
		rasterDesc.FrontCounterClockwise = false;
		rasterDesc.DepthClipEnable = false;


		D3D12_INPUT_ELEMENT_DESC inputElementDesc[] = {
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
		D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12,
		D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24,
		D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		};
		D3D12_GRAPHICS_PIPELINE_STATE_DESC postEffectRSDesc = psoDesc;
		postEffectRSDesc.InputLayout = { inputElementDesc, _countof(inputElementDesc) };
		postEffectRSDesc.pRootSignature = ROOTSIGNATURE->GetSamplingRootSignature().Get();
		postEffectRSDesc.RasterizerState = rasterDesc;
		postEffectRSDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		postEffectRSDesc.VS = CD3DX12_SHADER_BYTECODE(SHADER->GetSamplinigVS().Get());
		postEffectRSDesc.PS = CD3DX12_SHADER_BYTECODE(SHADER->GetSamplingPS().Get());

		postEffectRSDesc.SampleDesc.Count = 1;
		ThrowIfFailed(DEVICE->CreateGraphicsPipelineState(&postEffectRSDesc, IID_PPV_ARGS(&m_postEffectPipelineState)));
	}

	{
		D3D12_GRAPHICS_PIPELINE_STATE_DESC shadowPsoDesc = psoDesc;

		shadowPsoDesc.VS = CD3DX12_SHADER_BYTECODE(SHADER->GetDepthOnlyVS().Get());
		shadowPsoDesc.PS = CD3DX12_SHADER_BYTECODE(SHADER->GetDepthOnlyPS().Get());
		shadowPsoDesc.RTVFormats[0] = DXGI_FORMAT_R16G16B16A16_FLOAT;
		shadowPsoDesc.SampleDesc.Count = 1;
		ThrowIfFailed(DEVICE->CreateGraphicsPipelineState(&shadowPsoDesc, IID_PPV_ARGS(&m_shadowPipelineState)));
		
		shadowPsoDesc.InputLayout = { skinnedIEs, _countof(skinnedIEs) };
		shadowPsoDesc.VS = CD3DX12_SHADER_BYTECODE(SHADER->GetDepthOnlySkinnedVS().Get());
		ThrowIfFailed(DEVICE->CreateGraphicsPipelineState(&shadowPsoDesc, IID_PPV_ARGS(&m_shadowSkinnedPipelineState)));
	}
}
}