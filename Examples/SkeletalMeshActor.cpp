#include"SkeletalMeshActor.h"
#include "SkinnedMeshModel.h"

namespace hlab {
	void SkeletalMeshActor::UpdateAnimation(ComPtr<ID3D11DeviceContext> m_context, float dt, bool* m_keyPressed)
	{
        //전역함수로 빼주면 된다.
        static int frameCount = 0;
        static int state = 0;
        const float m_simToRenderScale = 0.01f;
        // TODO:
        if (state == 0) {
            if (m_keyPressed[VK_SPACE]) {
                state = 1;
                frameCount = 0;
            }
        }
        else if (state == 1) {
            if (frameCount ==
                m_skinnedMeshModel->m_aniData.clips[1].keys[0].size()) {
                frameCount = 0;
                state = 0;

            }
            if (frameCount == 115) {
                Vector3 handPos = (m_skinnedMeshModel->m_worldRow).Translation();
                Vector4 offset = Vector4::Transform(
                    Vector4(0.0f, 0.0f, -0.1f, 0.0f),
                    m_worldRow*
                    m_skinnedMeshModel->m_worldRow *
                    m_skinnedMeshModel->m_aniData.accumulatedRootTransform);
                handPos += Vector3(offset.x, offset.y, offset.z);

                Vector4 dir(0.0f, 0.0f, -1.0f, 0.0f);
                dir = Vector4::Transform(
                    dir, m_worldRow *
                    m_skinnedMeshModel->m_aniData.accumulatedRootTransform);
                dir.Normalize();
                dir *= 1.5f / m_simToRenderScale;
                //btSphereShape* SphereShape = new btSphereShape(5.0);
                //CreateDynamic(btTransform(btQuaternion(), btVector3(handPos.x, handPos.y, handPos.z) /
                //    m_simToRenderScale),
                //    SphereShape, btVector3(dir.x, dir.y, dir.z));
            }
        }


        m_skinnedMeshModel->UpdateAnimation(m_context, state, frameCount);

        frameCount += 1;
	}

}