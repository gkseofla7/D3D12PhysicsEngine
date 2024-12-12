#include "RootSignature.h"
#include "Engine.h"
#include "Device.h"
#include "Samplers2.h"
namespace hlab {
void RootSignature::Init()
{
	//CreateGraphicsRootSignature();
	CreateDefaultRootSignature();
}

void RootSignature::CreateGraphicsRootSignature()
{
	_samplerDesc = CD3DX12_STATIC_SAMPLER_DESC(0);

	CD3DX12_DESCRIPTOR_RANGE ranges[] =
	{
		CD3DX12_DESCRIPTOR_RANGE(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, CBV_REGISTER_COUNT - 1, 1), // b1~b4
		CD3DX12_DESCRIPTOR_RANGE(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, SRV_REGISTER_COUNT, 0), // t0~t4
	};

	CD3DX12_ROOT_PARAMETER param[2];
	param[0].InitAsConstantBufferView(static_cast<uint32>(CBV_REGISTER::b0)); // b0
	param[1].InitAsDescriptorTable(_countof(ranges), ranges);	

	D3D12_ROOT_SIGNATURE_DESC sigDesc = CD3DX12_ROOT_SIGNATURE_DESC(_countof(param), param, 1, &_samplerDesc);
	sigDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT; // �Է� ������ �ܰ�

	ComPtr<ID3DBlob> blobSignature;
	ComPtr<ID3DBlob> blobError;
	::D3D12SerializeRootSignature(&sigDesc, D3D_ROOT_SIGNATURE_VERSION_1, &blobSignature, &blobError);
	DEVICE->CreateRootSignature(0, blobSignature->GetBufferPointer(), blobSignature->GetBufferSize(), IID_PPV_ARGS(&_graphicsRootSignature));
}
void RootSignature::CreateDefaultRootSignature()
{
	// MeshConstants, MaterialConstants

	vector<CD3DX12_ROOT_PARAMETER1> rootParameters;
	rootParameters.resize(9);
	// ���� ������
	// b0, GlobalConstants
	rootParameters[0].InitAsConstantBufferView(0); 
	// t10, t11, t12, t13
	{
		CD3DX12_DESCRIPTOR_RANGE1 range[4];
		range[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 10);
		range[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 11);
		range[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 12);
		range[3].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 13);
		rootParameters[1].InitAsDescriptorTable(1, &range[0], D3D12_SHADER_VISIBILITY_ALL);
		rootParameters[2].InitAsDescriptorTable(1, &range[1], D3D12_SHADER_VISIBILITY_ALL);
		rootParameters[3].InitAsDescriptorTable(1, &range[2], D3D12_SHADER_VISIBILITY_ALL);
		rootParameters[4].InitAsDescriptorTable(1, &range[3], D3D12_SHADER_VISIBILITY_ALL);
	}
	// ���� ������
	{
		CD3DX12_DESCRIPTOR_RANGE1 ranges[2];
		// b1 : MeshConstants, b2 : MaterialConstants
		ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 2, 1, 0, D3D12_DESCRIPTOR_RANGE_FLAG_NONE);
		rootParameters[5].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_ALL);
		// t0 : height,	 t1 : albedo,	t2 : normal,	t3: ao
		// t4 : metallicRoughness,	 t5 : emissive
		ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 6, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_NONE);
		rootParameters[6].InitAsDescriptorTable(1, &ranges[1], D3D12_SHADER_VISIBILITY_ALL);
		
		size_t sampleSize = GEngine->GetSamples()->GetSampleDesc().size();
		CD3DX12_DESCRIPTOR_RANGE1 sampleRanges[7];
		for (size_t i = 0; i < sampleSize; ++i) {
			sampleRanges[i].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, static_cast<UINT>(i));  // �� ���÷��� �ϳ��� �Ҵ�
		}
		rootParameters[7].InitAsDescriptorTable(static_cast<UINT>(sampleSize), &sampleRanges[0], D3D12_SHADER_VISIBILITY_ALL);
	}
	// TODO �ӽ÷�, ���� �ʿ�
	const int MAX_LIGHTS = 3;
	CD3DX12_DESCRIPTOR_RANGE1 shadowMapRange;
	shadowMapRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, MAX_LIGHTS, 15); // Start from t15
	rootParameters[8].InitAsDescriptorTable(1, &shadowMapRange, D3D12_SHADER_VISIBILITY_ALL);

	CreateRootSignature(defaultRootSignature, rootParameters);
}

void RootSignature::CreateRootSignature(ComPtr<ID3D12RootSignature>& rootSignature, vector<CD3DX12_ROOT_PARAMETER1>& rootParams)
{
	D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};

	// This is the highest version the sample supports. If CheckFeatureSupport succeeds, the HighestVersion returned will not be greater than this.
	featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

	if (FAILED(DEVICE->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
	{
		featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
	}

	// Allow input layout and deny uneccessary access to certain pipeline stages.
	D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init_1_1(static_cast<UINT>(rootParams.size()), rootParams.data(), 0, nullptr, rootSignatureFlags);

	ComPtr<ID3DBlob> signature;
	ComPtr<ID3DBlob> error;
	//ThrowIfFailed2(D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, featureData.HighestVersion, &signature, &error));
	ThrowIfFailed2(D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, featureData.HighestVersion, &signature, &error));
	ThrowIfFailed2(DEVICE->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&rootSignature)));
}
}

