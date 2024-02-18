#include"SkeletalMeshActor.h"
#include "SkinnedMeshModel.h"
#include "AnimHelper.h"

namespace hlab {
    SkeletalMeshActor::SkeletalMeshActor(ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext>& context,
        shared_ptr<DModel> InModel)
        :Actor(device, context, InModel)
    {
    }
    void SkeletalMeshActor::Initialize(ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext>& context,
        shared_ptr<DModel>  InModel)
    {
        //Actor::Initialize(device, context, InModel);
    }
    void SkeletalMeshActor::Update(ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext>& context, float dt)
    {
        UpdateAnimation(device, context, m_state, m_curFrame);
    }
    void SkeletalMeshActor::UpdateAnimation(ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext>& context, int clipId,
        int frame, int type) 
    {
        if (m_model)
        {
            m_model->UpdateAnimation(device, context, clipId, frame, type);
        }
    }

    void SkeletalMeshActor::Render(ComPtr<ID3D11DeviceContext>& context)
    {
        // ConstBuffer 대신 StructuredBuffer 사용
        // context->VSSetConstantBuffers(3, 1, m_skinnedConsts.GetAddressOf());

        //context->VSSetShaderResources(
        //    9, 1, m_boneTransforms.GetAddressOfSRV()); // 항상 slot index 주의

        // Skinned VS/PS는 GetPSO()를 통해서 지정되기 때문에
        // Model::Render(.)를 같이 사용 가능
        Actor::Render(context);
    };
}

//void SkeletalMeshActor::UpdateAnimation(ComPtr<ID3D11DeviceContext> m_context, float dt, bool* m_keyPressed)
//{
//       //전역함수로 빼주면 된다.
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

//           // 여기서는 AnimationClip이 SkinnedMesh라고 가정하겠습니다.
//           // 일반적으로는 모든 Animation이 SkinnedMesh Animation은 아닙니다.
//           m_boneTransforms.m_cpu.resize(aniData.clips.front().keys.size());

//           // 주의: 모든 keys() 개수가 동일하지 않을 수도 있습니다.
//           for (int i = 0; i < aniData.clips.front().keys.size(); i++)
//               m_boneTransforms.m_cpu[i] = Matrix();
//           m_boneTransforms.Initialize(device);
//       }
//   }