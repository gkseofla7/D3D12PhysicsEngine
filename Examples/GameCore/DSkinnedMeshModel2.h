#pragma once
#include "../D3D12Core/EnginePch.h"
#include "../GameCore/GeometryGenerator2.h"
#include "DModel2.h"
#include "../D3D12Core/StructuredBuffer2.h"
namespace dengine {
// DModel에서는 Mesh Loading
// DSkinnedMeshModel에서는 Animation Loading
class DSkinnedMeshModel : public DModel {
public:
    DSkinnedMeshModel(const string& basePath,
        const string& filename);
    virtual void Tick(float dt) override;
    virtual void Render() override;

    Matrix& GetAccumulatedRootTransform() { return m_accumulatedRootTransform; }
    Matrix& GetAccumulatedRootTransformToLocal() { return m_accumulatedRootTransformToLocal; }

    void SetAccumulatedRootTransform(const Matrix& InAccumulatedRootTransform) { m_accumulatedRootTransform = InAccumulatedRootTransform; }
    void SetAccumulatedRootTransformToLocal(const Matrix& InAccumulatedRootTransformToLocal) { m_accumulatedRootTransformToLocal = InAccumulatedRootTransformToLocal; }

    void IntegrateRootTransformToWorldTransform();
protected:
    virtual void UploadBuffers() override;
public:
    shared_ptr<StructuredBuffer<Matrix>> m_boneTransforms;
private:
    Matrix m_accumulatedRootTransform;
    Matrix m_accumulatedRootTransformToLocal;

};

} // namespace dengine