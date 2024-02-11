#include "Wizard.h"
#include "AnimHelper.h"
namespace hlab {

Wizard::Wizard(ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext>& context,
	shared_ptr<DModel> InModel)
    :SkeletalMeshActor(device, context, InModel)
{
	Initialize(device, context, InModel);
}
void Wizard::Initialize(ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext>& context,
	shared_ptr<DModel> InModel)
{
    m_model->m_modelId = 1;
	InitBoundingKey();
	// TODO. 애니메이션 등록
}
 
void Wizard::Update(ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext>& context, float dt)
{
    static int frameCount = 0;
    static int state = 0;
	SkeletalMeshActor::Update(device, context, dt);


    // TODO:
    //if (state == 0) {

    //}
    //// 델리게이트로 끝났을 경우 쏴주는게,,,ㅋㅋ or 그냥 마무리 됐다고 알려주는
    //else if (state == 1) {
    //    //if (frameCount ==
    //        //m_character->m_aniData.clips[1].keys[0].size()) {
    //    //    frameCount = 0;
    //    //    state = 0;

    //    //}
    //    if (frameCount == 115) {
    //        Vector3 handPos = (m_character->m_worldRow).Translation();
    //        Vector4 offset = Vector4::Transform(
    //            Vector4(0.0f, 0.0f, -0.1f, 0.0f),
    //            m_character->m_worldRow *
    //            m_character->m_aniData.accumulatedRootTransform);
    //        handPos += Vector3(offset.x, offset.y, offset.z);

    //        Vector4 dir(0.0f, 0.0f, -1.0f, 0.0f);
    //        dir = Vector4::Transform(
    //            dir, m_character->m_worldRow *
    //            m_character->m_aniData.accumulatedRootTransform);
    //        dir.Normalize();
    //        dir *= 1.5f / m_simToRenderScale;
    //        CreateDynamic(PxTransform(PxVec3(handPos.x, handPos.y, handPos.z) /
    //            m_simToRenderScale),
    //            PxSphereGeometry(5), PxVec3(dir.x, dir.y, dir.z));
    //    }
    //}


    //m_character->UpdateAnimation(m_context, state, frameCount);

    frameCount += 1;
}
void Wizard::InitBoundingKey()
{
	std::function<void()> ShotFireballFunc = [this]() { this->ShotFireball();};
	m_keyBinding.insert({ VK_SPACE, ShotFireballFunc });
}

void Wizard::ShotFireball()
{
	if (GetActorState() == ActorState::SpecialState)
	{
		return;
	}
	cout << "Fireball" << endl;
}
}