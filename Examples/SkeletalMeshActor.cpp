#include"SkeletalMeshActor.h"
#include "SkinnedMeshModel.h"
#include "AnimHelper.h"

namespace hlab {
	//void SkeletalMeshActor::UpdateAnimation(ComPtr<ID3D11DeviceContext> m_context, float dt, bool* m_keyPressed)
	//{
 //       //�����Լ��� ���ָ� �ȴ�.
 //       static int frameCount = 0;
 //       static int state = 0;
 //       const float m_simToRenderScale = 0.01f;
 //       // TODO:
 //       if (state == 0) {
 //           if (m_keyPressed[VK_SPACE]) {
 //               state = 1;
 //               frameCount = 0;
 //           }
 //       }
 //       else if (state == 1) {
 //           if (frameCount ==
 //               //m_skinnedMeshModel->m_aniData.clips[1].keys[0].size()) {
 //               frameCount = 0;
 //               state = 0;

 //           }
 //           if (frameCount == 115) {
 //               Vector3 handPos = (m_skinnedMeshModel->m_worldRow).Translation();
 //               Vector4 offset = Vector4::Transform(
 //                   Vector4(0.0f, 0.0f, -0.1f, 0.0f),
 //                   m_worldRow*
 //                   m_skinnedMeshModel->m_worldRow *
 //                   m_skinnedMeshModel->m_aniData.accumulatedRootTransform);
 //               handPos += Vector3(offset.x, offset.y, offset.z);

 //               Vector4 dir(0.0f, 0.0f, -1.0f, 0.0f);
 //               dir = Vector4::Transform(
 //                   dir, m_worldRow *
 //                   m_skinnedMeshModel->m_aniData.accumulatedRootTransform);
 //               dir.Normalize();
 //               dir *= 1.5f / m_simToRenderScale;
 //               //btSphereShape* SphereShape = new btSphereShape(5.0);
 //               //CreateDynamic(btTransform(btQuaternion(), btVector3(handPos.x, handPos.y, handPos.z) /
 //               //    m_simToRenderScale),
 //               //    SphereShape, btVector3(dir.x, dir.y, dir.z));
 //           }
 //       }


 //      // m_skinnedMeshModel->UpdateAnimation(m_context, state, frameCount);

 //       frameCount += 1;
	//}
 //   void SkeletalMeshActor::InitAnimationData(ComPtr<ID3D11Device>& device,
 //       const AnimationData& aniData) {
 //       if (!aniData.clips.empty()) {

 //           // ���⼭�� AnimationClip�� SkinnedMesh��� �����ϰڽ��ϴ�.
 //           // �Ϲ������δ� ��� Animation�� SkinnedMesh Animation�� �ƴմϴ�.
 //           m_boneTransforms.m_cpu.resize(aniData.clips.front().keys.size());

 //           // ����: ��� keys() ������ �������� ���� ���� �ֽ��ϴ�.
 //           for (int i = 0; i < aniData.clips.front().keys.size(); i++)
 //               m_boneTransforms.m_cpu[i] = Matrix();
 //           m_boneTransforms.Initialize(device);
 //       }
 //   }

    SkeletalMeshActor::SkeletalMeshActor(ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext>& context,
        const string& basePath, const string& filename)
    {
        Initialize(device, context, basePath, filename);
    }
    void SkeletalMeshActor::Initialize(ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext>& context,
        const string& basePath, const string& filename)
    {
        //InitAnimationData(device, aniData);
        Actor::Initialize(device, context, basePath, filename);
    }

    void SkeletalMeshActor::UpdateAnimation(ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext>& context, int clipId,
        int frame, int type = 0) 
    {
        AnimHelper::UpdateAnimation(device,context, this, clipId, frame, type);
    }

    void SkeletalMeshActor::Render(ComPtr<ID3D11DeviceContext>& context)
    {
        // ConstBuffer ��� StructuredBuffer ���
        // context->VSSetConstantBuffers(3, 1, m_skinnedConsts.GetAddressOf());

        context->VSSetShaderResources(
            9, 1, m_boneTransforms.GetAddressOfSRV()); // �׻� slot index ����

        // Skinned VS/PS�� GetPSO()�� ���ؼ� �����Ǳ� ������
        // Model::Render(.)�� ���� ��� ����
        Actor::Render(context);
    };
}