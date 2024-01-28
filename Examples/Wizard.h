#pragma once
#include "SkeletalMeshActor.h"
namespace hlab {
class Wizard : public SkeletalMeshActor
{
public:
	Wizard(ComPtr<ID3D11Device>& device,ComPtr<ID3D11DeviceContext>& context);
private:
	virtual void InitBoundingKey() override;
	static void ShotFireball(shared_ptr<Actor> InActiveActor);

private:

};
}


