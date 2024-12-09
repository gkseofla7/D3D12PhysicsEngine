#pragma once
#include "EnginePch.h"
#include "../GeometryGenerator.h"
#include "DModel2.h"
#include "StructuredBuffer.h"
namespace hlab {
// DModel에서는 Mesh Loading
// DSkinnedMeshModel에서는 Animation Loading
class DSkinnedMeshModel2 : public DModel2 {
public:
    DSkinnedMeshModel2(const string& basePath,
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
    shared_ptr<StructuredBuffer2> m_boneTransforms;
private:
    Matrix m_accumulatedRootTransform;
    Matrix m_accumulatedRootTransformToLocal;
};

} // namespace hlab