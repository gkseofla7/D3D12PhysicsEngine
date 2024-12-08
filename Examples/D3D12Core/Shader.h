#pragma once
#include "EnginePch.h"

namespace hlab {
enum class SHADER_TYPE : uint8
{
	DEFERRED,
	FORWARD,
	LIGHTING,
	PARTICLE,
	COMPUTE,
	SHADOW,
};

enum class RASTERIZER_TYPE : uint8
{
	CULL_NONE,
	CULL_FRONT,
	CULL_BACK,
	WIREFRAME,
};

enum class DEPTH_STENCIL_TYPE : uint8
{
	LESS,
	LESS_EQUAL,
	GREATER,
	GREATER_EQUAL,
	NO_DEPTH_TEST, // ±Ì¿Ã ≈◊Ω∫∆Æ(X) + ±Ì¿Ã ±‚∑œ(O)
	NO_DEPTH_TEST_NO_WRITE, // ±Ì¿Ã ≈◊Ω∫∆Æ(X) + ±Ì¿Ã ±‚∑œ(X)
	LESS_NO_WRITE, // ±Ì¿Ã ≈◊Ω∫∆Æ(O) + ±Ì¿Ã ±‚∑œ(X)
};

enum class BLEND_TYPE : uint8
{
	DEFAULT,
	ALPHA_BLEND,
	ONE_TO_ONE_BLEND,
	END,
};

struct ShaderInfo
{
	SHADER_TYPE shaderType = SHADER_TYPE::FORWARD;
	RASTERIZER_TYPE rasterizerType = RASTERIZER_TYPE::CULL_BACK;
	DEPTH_STENCIL_TYPE depthStencilType = DEPTH_STENCIL_TYPE::LESS;
	BLEND_TYPE blendType = BLEND_TYPE::DEFAULT;
	D3D_PRIMITIVE_TOPOLOGY topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
};

struct ShaderArg
{
	const string vs = "VS_Main";
	const string hs;
	const string ds;
	const string gs;
	const string ps = "PS_Main";
};

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
