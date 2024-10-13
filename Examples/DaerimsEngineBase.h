#pragma once
#include "bullet/btBulletDynamicsCommon.h"

#include <unordered_map>
#include <memory>
namespace hlab
{
class Model;
class Object;
class DaerimsEngineBase
{
private:
	DaerimsEngineBase() {}
public:
	static DaerimsEngineBase& GetInstance()
	{
		static DaerimsEngineBase Engine;
		return Engine;
	}
	void InitPhysEngine();

	btBoxShape* CreateBoxShape(const btVector3& halfExtents);
	btRigidBody* CreateRigidBody(float mass, const btTransform startTransform, btCollisionShape* shape, const btScalar m_angularDamping, const btVector4& color);
	void RemoveRigidBody(btRigidBody* InRigidBody);
	void StepSimulation(float InDeltaTime);
	btDiscreteDynamicsWorld* GetDynamicWorld() { return m_dynamicsWorld; }

	void RegisterPhysMap(btCollisionObject* InRigidBody, std::weak_ptr<Object> InObject);
	void RemovePhysMap(btCollisionObject* InRigidBody);
	const std::unordered_map<void*, std::weak_ptr<Object>>& GetPhysMap() const { return m_physMap; }
	
	std::weak_ptr<Object> GetPhysObject(btCollisionObject* InRigidBody);
	btCollisionObject* GetCollisionObject(int InObjId);
private:
	void CreateEmptyDynamicsWorld(btDefaultCollisionConfiguration*& OutCollisionCofiguration,
		btCollisionDispatcher*& OutCollisionDispatcher, btBroadphaseInterface*& OutBroadphaseInterface,
		btConstraintSolver*& OutSolver, btDiscreteDynamicsWorld*& OutDynamicsWorld);
private:
	std::unordered_map<void*, std::weak_ptr<Object>> m_physMap;
	std::unordered_map<int, btCollisionObject*> m_collisionMap;

	btDefaultCollisionConfiguration* m_collisionConfiguration;
	btBroadphaseInterface* m_broadphase;
	btCollisionDispatcher* m_dispatcher;
	btConstraintSolver* m_solver;
	btDiscreteDynamicsWorld* m_dynamicsWorld;
};

}
