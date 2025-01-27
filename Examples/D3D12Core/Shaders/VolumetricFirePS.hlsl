#include "common.hlsli"
#include "Quaternion.hlsli"

cbuffer BillboardContsts : register(b3)
{
    float widthWorld;
    float3 dirWorld;
};

struct BillboardPixelShaderInput
{
    float4 posProj : SV_POSITION;
    float4 posWorld : POSITION0;
    float4 center : POSITION1;
    float2 texcoord : TEXCOORD;
    uint primID : SV_PrimitiveID;
};

// BRADY'S VOLUMETRIC FIRE
// https://www.shadertoy.com/view/WllXzB

float noise(float3 P)
{
    float3 Pi = floor(P);
    float3 Pf = P - Pi;
    float3 Pf_min1 = Pf - 1.0;
    Pi.xyz = Pi.xyz - floor(Pi.xyz * (1.0 / 69.0)) * 69.0;
    float3 temp;
    temp.xyz = 69.0 - 1.5;
    float3 Pi_inc1 = step(Pi, temp) * (Pi + 1.0);
    float4 Pt = float4(Pi.xy, Pi_inc1.xy) + float2(50.0, 161.0).xyxy;
    Pt *= Pt;
    Pt = Pt.xzxz * Pt.yyww;
    float2 hash_mod = float2(1.0 / (635.298681 + float2(Pi.z, Pi_inc1.z) * 48.500388));
    float4 hash_lowz = frac(Pt * hash_mod.xxxx);
    float4 hash_highz = frac(Pt * hash_mod.yyyy);
    float3 blend = Pf * Pf * Pf * (Pf * (Pf * 6.0 - 15.0) + 10.0);
    float4 res0 = lerp(hash_lowz, hash_highz, blend.z);
    float4 blend2 = float4(blend.xy, float2(1.0 - blend.xy));
    return dot(res0, blend2.zxzx * blend2.wwyy);
}

float fnoise(float3 p, float time)
{
    float f = 0.0;
    //p = p - float3(0.0, 0.0, 1.0) * time;
    p = p + dirWorld * time; // 진행방향 (반대로) 꼬리 흐름 만들기
    p = p * 3.0;
    f += 0.50000 * noise(p);
    p = 2.0 * p;
    f += 0.25000 * noise(p);
    p = 2.0 * p;
    f += 0.12500 * noise(p);
    p = 2.0 * p;
    f += 0.06250 * noise(p);
    p = 2.0 * p;
    f += 0.03125 * noise(p);
    return f;
}

float3x3 AngleAxis3x3(float angle, float3 axis)
{
    float c, s;
    sincos(angle, s, c);

    float t = 1 - c;
    float x = axis.x;
    float y = axis.y;
    float z = axis.z;

    return float3x3(t * x * x + c, t * x * y - s * z, t * x * z + s * y,
                    t * x * y + s * z, t * y * y + c, t * y * z - s * x,
                    t * x * z - s * y, t * y * z + s * x, t * z * z + c);
}

float modelFunc(in float3 p)
{
    //p.x *= .7;

    //p.z *= 0.3;
    //float4 rotQuat = from_to_rotation(normalize(-dirWorld), float3(0, 0, 1) );
    //p = mul(float4(p, 0.0), quaternion_to_matrix(rotQuat));

    float sphere = length(p) - 0.8; // 중심이 원점인 구
    float res = sphere + fnoise(p * 1.5, globalTime * 5.) * .4;
    //float res = sphere;
    
    //float res = sphere + fnoise(p * 1.5, 0 * 3.) * .4;
    return res * 0.8;
}

float raymarch(in float3 ro, in float3 rd)
{
    float dist = 0.;
    for (int i = 0; i < 30; i++)
    {
        float m = modelFunc(ro + rd * dist);
        dist += m;
        
        if (m < .01) // 구 안으로 들어갔다면 거리 반환
            return dist;
        else if (dist > 10.)
            break;
    }
    return -1.;
}

float3 hueShift(float3 color, float hueAdjust)
{

    const float3 kRGBToYPrime = float3(0.299, 0.587, 0.114);
    const float3 kRGBToI = float3(0.596, -0.275, -0.321);
    const float3 kRGBToQ = float3(0.212, -0.523, 0.311);

    const float3 kYIQToR = float3(1.0, 0.956, 0.621);
    const float3 kYIQToG = float3(1.0, -0.272, -0.647);
    const float3 kYIQToB = float3(1.0, -1.107, 1.704);

    float YPrime = dot(color, kRGBToYPrime);
    float I = dot(color, kRGBToI);
    float Q = dot(color, kRGBToQ);
    float hue = atan2(Q, I);
    float chroma = sqrt(I * I + Q * Q);

    hue += hueAdjust;

    Q = chroma * sin(hue);
    I = chroma * cos(hue);

    float3 yIQ = float3(YPrime, I, Q);

    return float3(dot(yIQ, kYIQToR), dot(yIQ, kYIQToG), dot(yIQ, kYIQToB));

}

float3 saturation(float3 rgb, float adjustment)
{
    const float3 W = float3(0.2125, 0.7154, 0.0721);
    float3 intensity;
    intensity.xyz = dot(rgb, W);
    return lerp(intensity, rgb, adjustment);
}


float3 volume(in float3 p, in float3 rd, in float3 ld)
{
    float3 op = p;
    float trans = 1.0;
    float td = 0.0;
    float dif = 0.0;
    float emit = 0.0;
    float steps = 30.; // increase to smooth
    
    // march
    for (float i = 0.; i < steps; i++)
    {
        float m = modelFunc(p);
        p += rd * .03;
        
        float dens = 1. - smoothstep(0., .35, -m);
        td += dens;
        trans *= dens;
        
        if (td > 1.0 && dif <= 0.)
        {
            td = 1.;
            dif = clamp(1. - modelFunc(p - .1 * ld), 0., 1.);
            emit = pow(smoothstep(-.3, 0., modelFunc(p)), 4.);
        }
    }
    
    trans = (1. - pow(abs(td / steps), 4.5));
    trans = smoothstep(0., 1., trans);
    float emitmod = (emit * trans) * .8 + .2;
    
    // light
    float3 lin = float3(.3, .2, .9);
    lin = hueShift(lin, 4.2 + -trans * .6 + dif * .5);
    lin *= emitmod;
    
    // bright/sat/contrast
    lin = saturation(lin, pow(trans, .5) * .4);
    lin *= 5.;
    lin -= float3(0.4, 0.4, 0.4);
    
    return lerp(float3(0, 0, 0), lin, pow(trans, 1.25));
}

struct PixelShaderOutput
{
    float4 pixelColor : SV_Target0;
};

// 쉐이더 토이에서 직접 바꿔가는 과정도 설명
/*mat3 rotationMatrix(vec3 axis, float angle)
{
    axis = normalize(axis);
    float s = sin(angle);
    float c = cos(angle);
    float oc = 1.0 - c;
    
    return mat3(oc * axis.x * axis.x + c, oc * axis.x * axis.y - axis.z * s, oc * axis.z * axis.x + axis.y * s,
                oc * axis.x * axis.y + axis.z * s, oc * axis.y * axis.y + c, oc * axis.y * axis.z - axis.x * s,
                oc * axis.z * axis.x - axis.y * s, oc * axis.y * axis.z + axis.x * s, oc * axis.z * axis.z + c);
}

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 p = (fragCoord - .5 * iResolution.xy) / iResolution.y;
    p.xy *= 4.0;
    
    mat3 rot = rotationMatrix(vec3(0.0, 1.0, 0.0), iMouse.x / 100.0);
    
    //vec3 ro = normalize(vec3(cos(iMouse.x/100.), 0.0, -sin(iMouse.x/100.))) * 2.35;
    vec3 ro = vec3(0.0, 0.0, 1.35) * rot;
    vec3 rd = normalize(vec3(p.xy, 0.0) * rot - ro); // xy 평면도 같이 회전해야 함
    
    float dist = raymarch(ro, rd);
    vec3 ld = vec3(-1., 1., 0.);
    vec3 col = dist > 0. ? volume(ro + rd * dist, rd, ld, p) : background(p);
    
    fragColor = vec4(col, 1.0);
}*/

PixelShaderOutput main(BillboardPixelShaderInput input)
{
    float2 p = input.texcoord - 0.5;
    p.xy = p.xy * 2.0f;

    //float3 ro = normalize(mul(float4(0, 0, 1, 0), invView).xyz) * 1.35;
    //float3 rd = normalize(mul(float4(p.x, p.y, 0, 0), invView).xyz - ro);
    float3 posLocal = (input.posWorld.xyz - input.center.xyz) * 10.0; // 쉐이더토이 원본 효과와 스케일 맞춰주기
    float3 eyeLocal = (eyeWorld - input.center.xyz) * 10.0;
    float3 rd = normalize(posLocal - eyeLocal);
    float3 ro = posLocal - rd * 1.35;
    
    float4 rotQuat1 = from_to_rotation(float3(0, 0, 1), dirWorld);
    float4 rotQuat2 = from_to_rotation(dirWorld, float3(0, 0, 1));
    rd = mul(float4(rd, 0.0), quaternion_to_matrix(rotQuat1)).xyz;
    ro = mul(float4(ro, 0.0), quaternion_to_matrix(rotQuat1)).xyz;
    rd.z *= 0.5;
    ro.z *= 0.5;
    rd = mul(float4(rd, 0.0), quaternion_to_matrix(rotQuat2)).xyz;
    ro = mul(float4(ro, 0.0), quaternion_to_matrix(rotQuat2)).xyz;

    /*
    *     //p.z *= 0.3;
    //float4 rotQuat = from_to_rotation(normalize(-dirWorld), float3(0, 0, 1) );
    //p = mul(float4(p, 0.0), quaternion_to_matrix(rotQuat));
    */

    float dist = raymarch(ro, rd);
    float3 ld = float3(-1., 1., 0.);
    float3 col = dist > 0. ? volume(ro + rd * dist, rd, ld) : float3(0, 0, 0);
    
    PixelShaderOutput output;
    output.pixelColor = float4(col, dot(float3(1, 1, 1), col) * 1.3);
    //output.pixelColor.a = 1.0; // 테스트용

    return output;
}