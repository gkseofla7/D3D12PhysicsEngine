#pragma once
#include "SkeletalMeshActor.h"
namespace hlab {
class Wizard : public SkeletalMeshActor
{
public:
	Wizard(ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext>& context,
		shared_ptr<DModel> InModel);
	void Initialize(ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext>& context,
		shared_ptr<DModel> InModel) override;
	void Update(ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext>& context, float dt);
private:
	virtual void InitBoundingKey() override;
	void ShotFireball();

private:

};
}


