#pragma once

#include <directxtk/SimpleMath.h>
#include <string>
#include <vector>

#include "../D3D12Core/Vertex.h"

namespace dengine {

using std::string;
using std::vector;

struct MeshData {
    vector<Vertex> vertices;
    vector<SkinnedVertex> skinnedVertices;
    vector<uint32_t> indices;
    string albedoTextureFilename;
    string emissiveTextureFilename;
    string normalTextureFilename;
    string heightTextureFilename;
    string aoTextureFilename; // Ambient Occlusion
    string metallicTextureFilename;
    string roughnessTextureFilename;
    string opacityTextureFilename;
};

} // namespace hlab