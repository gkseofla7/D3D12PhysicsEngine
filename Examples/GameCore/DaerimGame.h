#pragma once
#include "D3D12Core/Engine.h"
#include "PhysicsCore.h"
namespace dengine
{
class Wizard;
class DaerimGame : public Engine
{
public:
	DaerimGame();
	~DaerimGame()
	{
	}
	virtual bool InitScene() override;
	void InitPhysics(bool interactive);
	virtual void Update(float dt) override;

	//void UpdateGUI() override;
	
	//void Render() override;

	void StepSimulation(float deltaTime);
	void CreateStack(const btTransform t, int numStacks, int numSlices,
		btScalar halfExtent);
	//btRigidBody* CreateDynamic(const btTransform& t,
	//	btCollisionShape* shape,
	//	const btVector3& velocity);
public:
	//shared_ptr<BillboardModel> m_fireball;
	//shared_ptr<SkeletalMeshActor> m_character;


	btScalar stackZ = 10.0f;

	float m_simToRenderScale = 0.01f; // 시뮬레이션 물체가 너무 작으면 불안정
};
}