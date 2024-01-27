#pragma once
#include "bullet/btBulletDynamicsCommon.h"
namespace hlab
{
class DaerimsEngineBase
{
public:
	static void CreateEmptyDynamicsWorld(btDefaultCollisionConfiguration* OutCollisionCofiguration,
		btCollisionDispatcher* OutCollisionDispatcher, btBroadphaseInterface* OutBroadphaseInterface,
		btConstraintSolver* OutSolver, btDiscreteDynamicsWorld* OutDynamicsWorld);
	static btBoxShape* CreateBoxShape(const btVector3& halfExtents);
	static btRigidBody* CreateRigidBody(btDiscreteDynamicsWorld* InDynamicsWorld, float mass, const btTransform& startTransform, btCollisionShape* shape, const btScalar m_angularDamping, const btVector4& color);
};
}
