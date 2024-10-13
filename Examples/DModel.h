#pragma once

#include "ConstantBuffers.h"
#include "D3D11Utils.h"
#include "GraphicsCommon.h"
#include "Mesh.h"
#include "MeshData.h"
#include "StructuredBuffer.h"

#include <directxtk/SimpleMath.h>

// 참고: DirectX-Graphics-Sampels
// https://github.com/microsoft/DirectX-Graphics-Samples/blob/master/MiniEngine/Model/Model.h

namespace hlab {

    using std::cout;
    using std::endl;
    using std::string;
    using std::vector;

    class DModel {
    public:
        DModel() {}
        DModel(ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext>& context,
            const string& basePath, const string& filename);
        DModel(ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext>& context,
            const string& meshKey);
        virtual void Initialize(ComPtr<ID3D11Device>& device,
            ComPtr<ID3D11DeviceContext>& context);
        virtual void Initialize(ComPtr<ID3D11Device>& device,
            ComPtr<ID3D11DeviceContext>& context,
            const string& basePath,
            const string& filename);
        void Initialize(ComPtr<ID3D11Device>& device,
            ComPtr<ID3D11DeviceContext>& context,
            const string& meshKey);
        virtual void InitMeshBuffers(ComPtr<ID3D11Device>& device,
            const MeshData& meshData,
            shared_ptr<Mesh>& newMesh);
        void Tick(float dt);
        void UpdateConstantBuffers(ComPtr<ID3D11DeviceContext>& context);
        virtual void UpdateAnimation(ComPtr<ID3D11Device>& device,
            ComPtr<ID3D11DeviceContext>& context,
            string clipId, int frame, int type = 0) {}
        virtual void UpdatePosition(const Vector3& InDelta);
        void SetWorldPosition(const Vector3& InPos);
        virtual void UpdateRotation(const Matrix& InDelta);
        virtual void UpdateVelocity(float dt) {}
        Vector3 GetWorldPosition() { return m_worldRow.Translation(); }
        
        virtual GraphicsPSO& GetPSO(const bool wired);
        virtual GraphicsPSO& GetDepthOnlyPSO();
        virtual GraphicsPSO& GetReflectPSO(const bool wired);

        virtual void Render(ComPtr<ID3D11DeviceContext>& context);
        virtual void UpdateAnimation(ComPtr<ID3D11DeviceContext>& context,
            string clipId, int frame, int type);
        virtual void RenderWireBoundingBox(ComPtr<ID3D11DeviceContext>& context);
        virtual void RenderWireBoundingSphere(ComPtr<ID3D11DeviceContext>& context);
        void UpdateWorldRow(const Matrix& worldRow);
       

        void SetScale(float InScale) { m_scale = InScale; }

        bool IsMeshInitialized() { return m_initializeMesh; }
    private:
        bool LoadMesh();
    public:
        Matrix m_worldRow = Matrix();   // Model(Object) To World 행렬
        Matrix m_worldITRow = Matrix(); // InverseTranspose

        bool m_drawNormals = false;
        bool m_isVisible = true;
        bool m_castShadow = true;
        bool m_isPickable = false; // 마우스로 선택/조작 가능 여부

        vector<Mesh>* m_meshes;

        ConstantBuffer<MeshConstants> m_meshConsts;
        ConstantBuffer<MaterialConstants> m_materialConsts;

        DirectX::BoundingBox m_boundingBox;
        DirectX::BoundingSphere m_boundingSphere;

        string m_name = "NoName";
        int m_modelId = -1;

        int m_maxFrame = 0;
    protected:
        bool m_initializeMesh = false;
    private:
        string m_basePath;
        string m_filename;

        string m_meshKey;

        shared_ptr<Mesh> m_boundingBoxMesh;
        shared_ptr<Mesh> m_boundingSphereMesh;

        float m_scale = 1.0f;
        bool m_initializeBoundingVolume = false;
    };

} // namespace hlab
