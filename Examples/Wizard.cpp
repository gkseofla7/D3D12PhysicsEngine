#include "Wizard.h"
namespace hlab {

Wizard::Wizard(ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext>& context,
	const string& basePath, const string& filename)
{
	Initialize(device, context, basePath, filename);
}
void Wizard::Initialize(ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext>& context,
	const string& basePath, const string& filename)
{
	SkeletalMeshActor::Initialize(device, context, basePath, filename);
	// 애니메이션 등록
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