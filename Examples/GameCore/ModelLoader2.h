#pragma once

// vcpkg install assimp:x64-windows
// Preprocessor definitions에 NOMINMAX 추가
#include <assimp\Importer.hpp>
#include <assimp\postprocess.h>
#include <assimp\scene.h>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "AnimationClip.h"
#include "MeshData.h"
#include "D3D12Core/Vertex.h"

namespace dengine {

using std::map;
using std::string;
using std::vector;

class ModelLoader {
  public:
    void Load(string basePath, string filename, bool revertNormals);
    void LoadAnimation(string basePath, string filename);

    const aiNode *FindParent(const aiNode *node);

    void ProcessNode(aiNode *node, const aiScene *scene,
                     DirectX::SimpleMath::Matrix tr);
    void CheckLowerBody(int nodeIndex);
    MeshData ProcessMesh(aiMesh *mesh, const aiScene *scene);

    void ReadAnimation(const aiScene *scene);

    std::string ReadTextureFilename(const aiScene *scene, aiMaterial *material,
                                    aiTextureType type);

    void UpdateTangents();

    // 버텍스의 변형에 직접적으로 참여하는 뼈들의 목록을 만듭니다.
    void FindDeformingBones(const aiScene *scene);
    void UpdateBoneIDs(aiNode *node, int *counter) {
        static int id = 0;
        if (node) {
            if (m_aniData.boneNameToId.count(node->mName.C_Str())) {
                m_aniData.boneNameToId[node->mName.C_Str()] = *counter;
                *counter += 1;
            }
            for (UINT i = 0; i < node->mNumChildren; i++) {
                UpdateBoneIDs(node->mChildren[i], counter);
            }
        }
    }

  public:
    string m_basePath;
    vector<MeshData> m_meshes;

    AnimationData m_aniData;

    bool m_isGLTF = false; // gltf or fbx
    bool m_revertNormals = false;
};
} // namespace hlab