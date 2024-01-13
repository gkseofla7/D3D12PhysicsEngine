#pragma once

#include <glm/glm.hpp>
#include <glm/simd/common.h>

namespace hlab {

using glm::ivec3;
using glm::uvec3;
using glm::vec3;

// https://www.shadertoy.com/view/3dVXDc

class Noise {

  public:
    static vec3 hash33(vec3 p);

    static float remap(float x, float a, float b, float c, float d);

    // Gradient noise by iq (modified to be tileable)
    static float gradientNoise(vec3 x, float freq);

    // Tileable 3D worley noise
    static float worleyNoise(vec3 uv, float freq);

    // Fbm for Perlin noise based on iq's blog
    static float perlinfbm(vec3 p, float freq, int octaves);

    // Tileable Worley fbm inspired by Andrew Schneider's Real-Time Volumetric
    // Cloudscapes chapter in GPU Pro 7.
    static float worleyFbm(vec3 p, float freq);

  private:
};

} // namespace hlab