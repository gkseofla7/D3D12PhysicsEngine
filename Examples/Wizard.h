#pragma once
#include "SkeletalMeshActor.h"
namespace hlab {
class Wizard : public SkeletalMeshActor
{
	virtual void InitBoundingKey() override;
	static void ShotFireball(shared_ptr<Actor> InActiveActor);

private:

};
}


