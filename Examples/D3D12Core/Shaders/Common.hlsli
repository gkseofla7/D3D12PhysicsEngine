#ifndef __COMMON_HLSLI__
#define __COMMON_HLSLI__

// 쉐이더에서 include할 내용들은 .hlsli 파일에 작성
// Properties -> Item Type: Does not participate in build으로 설정

#define MAX_LIGHTS 3 // 쉐이더에서도 #define 사용 가능
#define LIGHT_OFF 0x00
#define LIGHT_DIRECTIONAL 0x01
#define LIGHT_POINT 0x02
#define LIGHT_SPOT 0x04
#define LIGHT_SHADOW 0x10

// 샘플러들을 모든 쉐이더에서 공통으로 사용
SamplerState linearWrapSampler : register(s0);
SamplerState linearClampSampler : register(s1);
SamplerState shadowPointSampler : register(s2);
SamplerComparisonState shadowCompareSampler : register(s3);
SamplerState pointWrapSampler : register(s4);
SamplerState linearMirrorSampler : register(s5);
SamplerState pointClampSampler : register(s6);

// 공용 텍스춰들 t10 부터 시작
TextureCube envIBLTex : register(t10);
TextureCube specularIBLTex : register(t11);
TextureCube irradianceIBLTex : register(t12);
Texture2D brdfTex : register(t13);

Texture2D shadowMaps[MAX_LIGHTS] : register(t15);
//Texture2D shadowMap1 : register(t16);
//Texture2D shadowMap2 : register(t17);

//static Texture2D shadowMaps[MAX_LIGHTS] = { shadowMap0, shadowMap1, shadowMap2 };

struct Light
{
    float3 radiance; // Strength
    float fallOffStart;
    float3 direction;
    float fallOffEnd;
    float3 position;
    float spotPower;
    
    uint type;
    float radius;
    float haloRadius;
    float haloStrength;

    matrix viewProj;
    matrix invProj;
};

// 공용 Constants
cbuffer GlobalConstants : register(b0)
{
    matrix view;
    matrix proj;
    matrix invProj; // 역프로젝션행렬
    matrix viewProj;
    matrix invViewProj; // Proj -> World
    matrix invView;

    float3 eyeWorld;
    float strengthIBL;

    int textureToDraw = 0; // 0: Env, 1: Specular, 2: Irradiance, 그외: 검은색
    float envLodBias = 0.0f; // 환경맵 LodBias
    float lodBias = 2.0f; // 다른 물체들 LodBias
    float globalTime;
    
    Light lights[MAX_LIGHTS];
};

cbuffer MeshConstants : register(b1)
{
    matrix world; // Model(또는 Object) 좌표계 -> World로 변환
    matrix worldIT; // World의 InverseTranspose
    matrix worldInv;
    int useHeightMap;
    float heightScale;
    float windTrunk;
    float windLeaves;
};

cbuffer MaterialConstants : register(b2)
{
    float3 albedoFactor; // baseColor
    float roughnessFactor;
    float metallicFactor;
    float3 emissionFactor;

    int useAlbedoMap;
    int useNormalMap;
    int useAOMap; // Ambient Occlusion
    int invertNormalMapY;
    int useMetallicMap;
    int useRoughnessMap;
    int useEmissiveMap;
    float dummy2;
};

#ifdef SKINNED

// 관절 개수 제약을 없애게 위해 StructuredBuffer 사용
StructuredBuffer<float4x4> boneTransforms : register(t9);

//cbuffer SkinnedConstants : register(b3)
//{
//    float4x4 boneTransforms[52]; // 관절 개수에 따라 조절
//};

#endif

struct VertexShaderInput
{
    float3 posModel : POSITION; //모델 좌표계의 위치 position
    float3 normalModel : NORMAL; // 모델 좌표계의 normal    
    float2 texcoord : TEXCOORD;
    float3 tangentModel : TANGENT;
    
#ifdef SKINNED
    float4 boneWeights0 : BLENDWEIGHT0;
    float4 boneWeights1 : BLENDWEIGHT1;
    uint4 boneIndices0 : BLENDINDICES0;
    uint4 boneIndices1 : BLENDINDICES1;
#endif
};

struct PixelShaderInput
{
    float4 posProj : SV_POSITION; // Screen position
    float3 posWorld : POSITION0; // World position (조명 계산에 사용)
    float3 normalWorld : NORMAL0;
    float2 texcoord : TEXCOORD0;
    float3 tangentWorld : TANGENT0;
    float3 posModel : POSITION1; // Volume casting 시작점
};

#endif // __COMMON_HLSLI__