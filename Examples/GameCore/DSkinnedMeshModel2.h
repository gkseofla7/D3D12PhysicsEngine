#pragma once
#include "../D3D12Core/EnginePch.h"
#include "../Legacy/GeometryGenerator.h"
#include "DModel2.h"
#include "../D3D12Core/StructuredBuffer2.h"
namespace dengine {
// DModel������ Mesh Loading
// DSkinnedMeshModel������ Animation Loading
class DSkinnedMeshModel : public DModel {
public:
    DSkinnedMeshModel(const string& basePath,
        const string& filename);
    virtual void InitMeshBuffers(const MeshData& meshData, shared_ptr<DMesh>& newMesh) override;

    virtual void Render() override;

    Matrix& GetAccumulatedRootTransform() { return m_accumulatedRootTransform; }
    Matrix& GetAccumulatedRootTransformToLocal() { return m_accumulatedRootTransformToLocal; }

    void SetAccumulatedRootTransform(const Matrix& InAccumulatedRootTransform) { m_accumulatedRootTransform = InAccumulatedRootTransform; }
    void SetAccumulatedRootTransformToLocal(const Matrix& InAccumulatedRootTransformToLocal) { m_accumulatedRootTransformToLocal = InAccumulatedRootTransformToLocal; }

    void IntegrateRootTransformToWorldTransform();
public:
    // ConstantBuffer<SkinnedConsts> m_skinnedConsts;
    shared_ptr<StructuredBuffer<Matrix>> m_boneTransforms;
private:
    Matrix m_accumulatedRootTransform;
    Matrix m_accumulatedRootTransformToLocal;
};

} // namespace dengine