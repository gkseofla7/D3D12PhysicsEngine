#include "DaerimsEngineBase.h"
#include "Object.h"

#include "bullet/btBulletDynamicsCommon.h"
#include "bullet/btBulletCollisionCommon.h"
#include "bullet/BulletDynamics/Dynamics/btRigidBody.h"
#include "bullet/BulletDynamics/btBulletDynamicsCommon.h"
#include "bullet/BulletCollision/btBulletCollisionCommon.h"
#include "bullet/BulletCollision/CollisionDispatch/btCollisionWorldImporter.h"

namespace hlab{


void DaerimsEngineBase::InitPhysEngine()
{
	DaerimsEngineBase::CreateEmptyDynamicsWorld(m_collisionConfiguration, m_dispatcher, m_broadphase, m_solver, m_dynamicsWorld);
	if (m_dynamicsWorld->getDebugDrawer())
		m_dynamicsWorld->getDebugDrawer()->setDebugMode(btIDebugDraw::DBG_DrawWireframe + btIDebugDraw::DBG_DrawContactPoints);

}
void DaerimsEngineBase::CreateEmptyDynamicsWorld(btDefaultCollisionConfiguration*& OutCollisionCofiguration,
		btCollisionDispatcher* &OutCollisionDispatcher, btBroadphaseInterface*& OutBroadphaseInterface,
		btConstraintSolver* &OutSolver, btDiscreteDynamicsWorld*& OutDynamicsWorld)
{
	///collision configuration contains default setup for memory, collision setup
	OutCollisionCofiguration = new btDefaultCollisionConfiguration();
	//m_collisionConfiguration->setConvexConvexMultipointIterations();

	///use the default collision dispatcher. For parallel processing you can use a diffent dispatcher (see Extras/BulletMultiThreaded)
	OutCollisionDispatcher = new btCollisionDispatcher(OutCollisionCofiguration);

	OutBroadphaseInterface = new btDbvtBroadphase();

	///the default constraint solver. For parallel processing you can use a different solver (see Extras/BulletMultiThreaded)
	btSequentialImpulseConstraintSolver* sol = new btSequentialImpulseConstraintSolver;
	OutSolver = sol;

	OutDynamicsWorld = new btDiscreteDynamicsWorld(OutCollisionDispatcher, OutBroadphaseInterface, OutSolver, OutCollisionCofiguration);

	OutDynamicsWorld->setGravity(btVector3(0, -10, 0));
}

btBoxShape* DaerimsEngineBase::CreateBoxShape(const btVector3& halfExtents)
{
	btBoxShape* box = new btBoxShape(halfExtents);
	return box;
}

btRigidBody* DaerimsEngineBase::CreateRigidBody(float mass,
	const btTransform startTransform, btCollisionShape* shape, const btScalar m_angularDamping, const btVector4& color)
{
	btAssert((!shape || shape->getShapeType() != INVALID_SHAPE_PROXYTYPE));

	//rigidbody is dynamic if and only if mass is non zero, otherwise static
	bool isDynamic = (mass != 0.f);

	btVector3 localInertia(0, 0, 0);
	if (isDynamic)
		shape->calculateLocalInertia(mass, localInertia);

	//using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects

#define USE_MOTIONSTATE 1
#ifdef USE_MOTIONSTATE
	btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);

	btRigidBody::btRigidBodyConstructionInfo cInfo(mass, myMotionState, shape, localInertia);
	cInfo.m_angularDamping = m_angularDamping;
	btRigidBody* body = new btRigidBody(cInfo);
	//body->setContactProcessingThreshold(m_defaultContactProcessingThreshold);

#else
	btRigidBody* body = new btRigidBody(mass, 0, shape, localInertia);
	body->setWorldTransform(startTransform);
#endif  //

	body->setUserIndex(-1);
	m_dynamicsWorld->addRigidBody(body);
	return body;
}

void DaerimsEngineBase::RemoveRigidBody(btRigidBody* InRigidBody)
{
	m_dynamicsWorld->removeRigidBody(InRigidBody);
}

void DaerimsEngineBase::StepSimulation(float InDeltaTime)
{
	if (m_dynamicsWorld)
	{
		m_dynamicsWorld->stepSimulation(InDeltaTime);
	}
}

void DaerimsEngineBase::RegisterPhysMap(btCollisionObject* InRigidBody, std::weak_ptr<Object> InObject)
{
	m_physMap[InRigidBody] = InObject;

	std::shared_ptr<Object> objectLock = InObject.lock();
	if (objectLock.get() == nullptr)
	{
		return;
	}
	m_collisionMap[objectLock->GetObjectId()] = InRigidBody;
}
void DaerimsEngineBase::RemovePhysMap(btCollisionObject* InRigidBody)
{
	if (m_physMap.find(InRigidBody) != m_physMap.end())
	{
		std::shared_ptr<Object> objectLock = m_physMap[InRigidBody].lock();
		if (objectLock.get() != nullptr)
		{
			if (m_collisionMap.find(objectLock->GetObjectId()) != m_collisionMap.end())
			{
				m_collisionMap.erase(m_collisionMap.find(objectLock->GetObjectId()));
			}
		}
		m_physMap.erase(m_physMap.find(InRigidBody));
	}
}

std::weak_ptr<Object> DaerimsEngineBase::GetPhysObject(btCollisionObject* InRigidBody)
{
	if (m_physMap.find(InRigidBody) == m_physMap.end())
	{
		return std::weak_ptr<Object>();
	}
	return m_physMap[InRigidBody];
}

btCollisionObject* DaerimsEngineBase::GetCollisionObject(int InObjId)
{
	if (m_collisionMap.find(InObjId) == m_collisionMap.end())
	{
		return nullptr;
	}
	return m_collisionMap[InObjId];
}

}