#pragma once

#include <directxtk/SimpleMath.h>
#include <vector>

namespace hlab {

using DirectX::SimpleMath::Vector2;
using DirectX::SimpleMath::Vector3;
using DirectX::SimpleMath::Vector4;

struct Vertex {
    Vector3 position;
    Vector3 normalModel;
    Vector2 texcoord;
    Vector3 tangentModel;
    // Vector3 biTangentModel; // biTangent는 쉐이더에서 계산
};

struct SkinnedVertex {
    Vector3 position;
    Vector3 normalModel;
    Vector2 texcoord;
    Vector3 tangentModel;

    float blendWeights[8] = {0.0f, 0.0f, 0.0f, 0.0f,
                             0.0f, 0.0f, 0.0f, 0.0f};  // BLENDWEIGHT0 and 1
    uint8_t boneIndices[8] = {0, 0, 0, 0, 0, 0, 0, 0}; // BLENDINDICES0 and 1

    // boneWeights가 최대 8개라고 가정 (Luna 교재에서는 4개)
    // bone의 수가 256개 이하라고 가정 uint8_t
};

struct GrassVertex {
    Vector3 posModel;
    Vector3 normalModel;
    Vector2 texcoord;

    // 주의: Instance World는 별도의 버퍼로 보냄
};

// GrassVS, grassIL과 일관성이 있어야 합니다.
struct GrassInstance {
    Matrix instanceWorld; // <- Instance 단위의 Model to World 변환
    float windStrength;
};

} // namespace hlab