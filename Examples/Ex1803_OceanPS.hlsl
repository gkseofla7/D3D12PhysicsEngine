#include "common.hlsli"

// Very fast procedural ocean 
// https://www.shadertoy.com/view/MdXyzX

// afl_ext 2017-2023
// Now with 2023 refresh

// Use your mouse to move the camera around! Press the Left Mouse Button on the image to look around!

#define DRAG_MULT 0.28 // changes how much waves pull on the water
#define WATER_DEPTH 1.0 // how deep is the water
#define CAMERA_HEIGHT 1.5 // how high the camera should be
#define ITERATIONS_RAYMARCH 12 // waves iterations of raymarching
#define ITERATIONS_NORMAL 40 // waves iterations when calculating normals

// #define NormalizedMouse (iMouse.xy / iResolution.xy) // normalize mouse coords

// Calculates wave value and its derivative, 
// for the wave direction, position in space, wave frequency and time
float2 wavedx(float2 position, float2 direction, float frequency, float timeshift)
{
    float x = dot(direction, position) * frequency + timeshift;
    float wave = exp(sin(x) - 1.0);
    float dx = wave * cos(x);
    return float2(wave, -dx);
}

// Calculates waves by summing octaves of various waves with various parameters
float getwaves(float2 position, int iterations)
{
    float iter = 0.0; // this will help generating well distributed wave directions
    float frequency = 1.3; // frequency of the wave, this will change every iteration
    float timeMultiplier = 1.0; // time multiplier for the wave, this will change every iteration
    float weight = 0.5; // weight in final sum for the wave, this will change every iteration
    float sumOfValues = 0.0; // will store final sum of values
    float sumOfWeights = 0.0; // will store final sum of weights
    for (int i = 0; i < iterations; i++)
    {
        // generate some wave direction that looks kind of random
        float2 p = float2(sin(iter), cos(iter));
        
        // calculate wave data
        float2 res = wavedx(position, p, frequency, globalTime * timeMultiplier);

        // shift position around according to wave drag and derivative of the wave
        position += p * res.y * weight * DRAG_MULT;

        // add the results to sums
        sumOfValues += res.x * weight;
        sumOfWeights += weight;

        // modify next octave parameters
        weight *= 0.82;
        frequency *= 1.18;
        timeMultiplier *= 1.07;

        // add some kind of random value to make next wave look random too
        iter += 1232.399963;
    }
    
    // calculate and return
    return sumOfValues / sumOfWeights;
}

// Raymarches the ray from top water layer boundary to low water layer boundary
float raymarchwater(float3 camera, float3 start, float3 end, float depth)
{
    float3 pos = start;
    float3 dir = normalize(end - start);
    for (int i = 0; i < 32; i++)
    {
        // the height is from 0 to -depth
        float height = getwaves(pos.xz, ITERATIONS_RAYMARCH) * depth - depth;
        
        // if the waves height almost nearly matches the ray height, assume its a hit and return the hit distance
        if (height + 0.01 > pos.y)
        {
            return distance(pos, camera);
        }
        
        // iterate forwards according to the height mismatch
        pos += dir * (pos.y - height);
    }
    
    // if hit was not registered, just assume hit the top layer, 
    // this makes the raymarching faster and looks better at higher distances
    return distance(start, camera);
}

// Calculate normal at point by calculating the height at the pos and 2 additional points very close to pos
float3 normal(float2 pos, float e, float depth)
{
    float2 ex = float2(e, 0);
    float H = getwaves(pos.xy, ITERATIONS_NORMAL) * depth;
    float3 a = float3(pos.x, H, pos.y);
    return normalize(
    cross(
      a - float3(pos.x - e, getwaves(pos.xy - ex.xy, ITERATIONS_NORMAL) * depth, pos.y),
      a - float3(pos.x, getwaves(pos.xy + ex.yx, ITERATIONS_NORMAL) * depth, pos.y + e)
    )
  );
}

// Ray-Plane intersection checker
float intersectPlane(float3 origin, float3 direction, float3 pos, float3 normal)
{
    return clamp(dot(pos - origin, normal) / dot(direction, normal), -1.0, 9991999.0);
}

// Some very barebones but fast atmosphere approximation
float3 extra_cheap_atmosphere(float3 raydir, float3 sundir)
{
    sundir.y = max(sundir.y, -0.07);
    float special_trick = 1.0 / (raydir.y * 1.0 + 0.1);
    float special_trick2 = 1.0 / (sundir.y * 11.0 + 1.0);
    float raysundt = pow(abs(dot(sundir, raydir)), 2.0);
    float sundt = pow(abs(max(0.0, dot(sundir, raydir))), 8.0);
    float mymie = sundt * special_trick * 0.2;
    float3 suncolor = lerp(float3(1, 1, 1), max(float3(1, 1, 1), float3(1, 1, 1) - float3(5.5, 13.0, 22.4) / 22.4), special_trick2);
    float3 bluesky = float3(5.5, 13.0, 22.4) / 22.4 * suncolor;
    float3 bluesky2 = max(float3(1, 1, 1), bluesky - float3(5.5, 13.0, 22.4) * 0.002 * (special_trick + -6.0 * sundir.y * sundir.y));
    bluesky2 *= special_trick * (0.24 + raysundt * 0.24);
    return bluesky2 * (1.0 + 1.0 * pow(1.0 - raydir.y, 3.0)) + mymie * suncolor;
}

// Calculate where the sun should be, it will be moving around the sky
float3 getSunDirection()
{
    return normalize(float3(sin(globalTime * 0.1), 1.0, cos(globalTime * 0.1)));
}

// Get atmosphere color for given direction
float3 getAtmosphere(float3 dir)
{
    return extra_cheap_atmosphere(dir, getSunDirection()) * 0.5;
}

// Get sun color for given direction
float getSun(float3 dir)
{
    return pow(max(0.0, dot(dir, getSunDirection())), 720.0) * 210.0;
}

// Main
float4 main(PixelShaderInput input) : SV_Target0
{
    float3 ray = -normalize(eyeWorld - input.posWorld);

    // now ray.y must be negative, water must be hit
    // define water planes
    float3 waterPlaneHigh = float3(0.0, 0.0, 0.0);
    float3 waterPlaneLow = float3(0.0, -WATER_DEPTH, 0.0);

    // define ray origin, moving around
    //float3 origin = float3(globalTime, CAMERA_HEIGHT, globalTime);
   // float3 origin = float3(0, CAMERA_HEIGHT, 0);
    float3 origin = eyeWorld;

    // calculate intersections and reconstruct positions
    float highPlaneHit = intersectPlane(origin, ray, waterPlaneHigh, float3(0.0, 1.0, 0.0));
    float lowPlaneHit = intersectPlane(origin, ray, waterPlaneLow, float3(0.0, 1.0, 0.0));
    float3 highHitPos = origin + ray * highPlaneHit;
    float3 lowHitPos = origin + ray * lowPlaneHit;

    // raymatch water and reconstruct the hit pos
    float dist = raymarchwater(origin, highHitPos, lowHitPos, WATER_DEPTH);
    float3 waterHitPos = origin + ray * dist;

    // calculate normal at the hit position
    float3 N = normal(waterHitPos.xz, 0.0001, WATER_DEPTH);

    // smooth the normal with distance to avoid disturbing high frequency noise
    N = lerp(N, float3(0.0, 1.0, 0.0), 0.8 * min(1.0, sqrt(dist * 0.01) * 1.1));

    // calculate fresnel coefficient
    float fresnel = (0.04 + (1.0 - 0.04) * (pow(1.0 - max(0.0, dot(-N, ray)), 5.0)));

    // reflect the ray and make sure it bounces up
    float3 R = normalize(reflect(ray, N));
    R.y = abs(R.y);
  
    // calculate the reflection and approximate subsurface scattering
    float3 reflection = getAtmosphere(R) + getSun(R);
    float3 scattering = float3(0.0293, 0.0698, 0.1717) * (0.2 + (waterHitPos.y + WATER_DEPTH) / WATER_DEPTH);

    // return the combined result
    float3 C = fresnel * reflection + (1.0 - fresnel) * scattering;
    
    return float4(C, 0.8);
}