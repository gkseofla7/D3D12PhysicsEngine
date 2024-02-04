#pragma once
#include "SkeletalMeshActor.h"
namespace hlab {
class Wizard : public SkeletalMeshActor
{
public:
	Wizard(ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext>& context,
		const string& basePath, const string& filename);
	virtual void Initialize(ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext>& context,
		const string& basePath, const string& filename) override;
private:
	virtual void InitBoundingKey() override;
	void ShotFireball();

private:

};
}


