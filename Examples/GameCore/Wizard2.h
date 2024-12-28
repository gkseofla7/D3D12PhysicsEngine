#pragma once
#include "Actor2.h"

namespace dengine {
class Wizard : public Actor
{
public:
	Wizard(shared_ptr<DModel> inModel);
	void Initialize(shared_ptr<DModel> inModel) override;
	void Tick(float dt);

	virtual void ReactProjectileHitted() override;
private:
	// TODO. Actor로 옮길 예정
	// 초기화
	void InitAnimPath();
	void LoadAnimAsync(string inState);
	virtual void InitBoundingKey() override;

	// 인푼 관련 함수
	void Attack();
	void WalkStart();
	void WalkEnd();
	void RotateLeft(bool InOn);
	void RotateRight(bool InOn);
	void Jump();

	void ShotFireBall();

	bool bLeft = false;
	bool bRight = false;
};
}


