#include "Wizard.h"
namespace hlab {
Wizard::Wizard(ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext>& context)
{
	m_skinnedMeshModel
}
void Wizard::InitBoundingKey()
{
	std::function<void(shared_ptr<Actor>)> ShotFireballFunc = &Wizard::ShotFireball;
	m_keyBinding.insert({ VK_SPACE, ShotFireballFunc });
}

void Wizard::ShotFireball(shared_ptr<Actor> InActiveActor)
{
	if (Wizard* WizardActor = dynamic_cast<Wizard*>(InActiveActor.get()))
	{
		if (WizardActor->GetActorState() == ActorState::SpecialState)
		{
			return;
		}
		cout << "Fireball" << endl;
	}
	
}

}