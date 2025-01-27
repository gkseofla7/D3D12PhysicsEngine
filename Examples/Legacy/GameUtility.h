#pragma once
#include <directxtk/SimpleMath.h>
#include "bullet/btBulletDynamicsCommon.h"
namespace hlab {
	using DirectX::SimpleMath::Vector3;
	Vector3 TransfromVector(const btVector3& InVector)
	{
		return Vector3(InVector.x(), InVector.y(), InVector.z());
	}
}