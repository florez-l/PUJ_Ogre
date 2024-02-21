// =========================================================================
// This file is part of the OGRE project.
// It is subject to the license terms in the LICENSE file found in the
// top-level directory of this distribution and at
// https://www.ogre3d.org/licensing.
// SPDX-License-Identifier: MIT
// =========================================================================
#ifndef __PUJ_Ogre__OgreBullet__h__
#define __PUJ_Ogre__OgreBullet__h__

#include <PUJ_Ogre/Export.h>

#include <Ogre.h>
#include <btBulletDynamicsCommon.h>

namespace PUJ_Ogre
{
  namespace Bullet
  {
    /**
       class
    */

    /** \addtogroup Optional
     *  @{
     */
    /** \defgroup Bullet Bullet
     * [Bullet-Physics](https://pybullet.org) to %Ogre connection
     * @{
     */
    enum ColliderType
    {
      CT_BOX,
      CT_SPHERE,
      CT_CYLINDER,
      CT_CAPSULE,
      CT_TRIMESH,
      CT_HULL
    };

    inline btQuaternion convert( const Ogre::Quaternion& q )
    { return( btQuaternion( q.x, q.y, q.z, q.w ) ); }
    inline btVector3 convert( const Ogre::Vector3& v )
    { return( btVector3( v.x, v.y, v.z ) ); }

    inline Ogre::Quaternion convert( const btQuaternion& q )
    { return( Ogre::Quaternion( q.w( ), q.x( ), q.y( ), q.z( ) ) ); }
    inline Ogre::Vector3 convert( const btVector3& v )
    { return( Ogre::Vector3( v.x( ), v.y( ), v.z( ) ) ); }

    /** A MotionState is Bullet's way of informing you about updates to an
     * object. Pass this MotionState to a btRigidBody to have your SceneNode
     * updated automaticaly.
     */
    class PUJ_Ogre_EXPORT RigidBodyState
      : public btMotionState
    {
      Ogre::Node* mNode;

    public:
      RigidBodyState( Ogre::Node* node )
        : mNode( node )
        {
        }

      virtual void getWorldTransform( btTransform& ret ) const override
        {
          ret = btTransform(
            convert( mNode->getOrientation( ) ),
            convert( mNode->getPosition( ) )
            );
        }

      virtual void setWorldTransform( const btTransform& in ) override
        {
          btQuaternion rot = in.getRotation( );
          btVector3 pos = in.getOrigin( );
          mNode->setOrientation( rot.w( ), rot.x( ), rot.y( ), rot.z( ) );
          mNode->setPosition( pos.x( ), pos.y( ), pos.z( ) );
        }
    };

    /// create sphere collider using ogre provided data
    PUJ_Ogre_EXPORT btSphereShape* createSphereCollider( const Ogre::MovableObject* mo );

    /// create box collider using ogre provided data
    PUJ_Ogre_EXPORT btBoxShape* createBoxCollider( const Ogre::MovableObject* mo );

    /// create capsule collider using ogre provided data
    PUJ_Ogre_EXPORT btCapsuleShape* createCapsuleCollider( const Ogre::MovableObject* mo );

    /// create capsule collider using ogre provided data
    PUJ_Ogre_EXPORT btCylinderShape* createCylinderCollider( const Ogre::MovableObject* mo );

    /**
     * TODO
     */
    struct PUJ_Ogre_EXPORT CollisionListener
    {
      virtual ~CollisionListener( ) = default;
        virtual void contact(
          const Ogre::MovableObject* other, const btManifoldPoint& manifoldPoint
          ) = 0;
    };

    /**
     * TODO
     */
    struct PUJ_Ogre_EXPORT RayResultCallback
    {
      virtual ~RayResultCallback( ) {}
      virtual void addSingleResult(
        const Ogre::MovableObject* other, float distance
        ) = 0;
    };

    /// simplified wrapper with automatic memory management
    class PUJ_Ogre_EXPORT CollisionWorld
    {
    protected:
      std::unique_ptr< btCollisionConfiguration > mCollisionConfig;
      std::unique_ptr< btCollisionDispatcher > mDispatcher;
      std::unique_ptr< btBroadphaseInterface > mBroadphase;

      btCollisionWorld* mBtWorld;

    public:
      CollisionWorld( btCollisionWorld* btWorld )
        : mBtWorld( btWorld )
        {
        }
      virtual ~CollisionWorld( );

      btCollisionObject* addCollisionObject(
        Ogre::Entity* ent, ColliderType ct, int group = 1, int mask = -1
        );

      void rayTest(
        const Ogre::Ray& ray, RayResultCallback* callback, float maxDist = 1000
        );
    };

    /// simplified wrapper with automatic memory management
    class PUJ_Ogre_EXPORT DynamicsWorld
      : public CollisionWorld
    {
    protected:
      std::unique_ptr< btConstraintSolver > mSolver;

    public:
      explicit DynamicsWorld( const Ogre::Vector3& gravity );
      DynamicsWorld( btDynamicsWorld* btWorld )
        : CollisionWorld( btWorld )
        {
        }

      btRigidBody* addRigidBody(
        float mass,
        Ogre::Entity* ent,
        ColliderType ct,
        CollisionListener* listener = nullptr,
        int group = 1, int mask = -1
        );

      btDynamicsWorld* getBtWorld( ) const
        {
          return( ( btDynamicsWorld* )mBtWorld );
        }
    };

    /**
     * TODO
     */
    class PUJ_Ogre_EXPORT DebugDrawer
      : public btIDebugDraw
    {
    protected:
      Ogre::SceneNode* mNode;
      btCollisionWorld* mWorld;

      Ogre::ManualObject mLines;
      int mDebugMode;

    public:
      DebugDrawer( Ogre::SceneNode* node, btCollisionWorld* world )
        : mNode( node ),
          mWorld( world ),
          mLines( "" ),
          mDebugMode( DBG_DrawWireframe )
        {
          mLines.setCastShadows( false );
          mNode->attachObject( &mLines );
          mWorld->setDebugDrawer( this );
        }

      void update( )
        {
          mWorld->debugDrawWorld( );
          if ( !mLines.getSections( ).empty( ) ) // begin was called
            mLines.end( );
        }

      void drawLine(
        const btVector3& from, const btVector3& to, const btVector3& color
        ) override;

      void drawContactPoint(
        const btVector3& PointOnB, const btVector3& normalOnB,
        btScalar distance, int lifeTime,
        const btVector3& color
        ) override
        {
          drawLine( PointOnB, PointOnB + normalOnB * distance * 20, color );
        }

      void reportErrorWarning( const char* warningString ) override
        {
          Ogre::LogManager::getSingleton( ).logWarning( warningString );
        }

      void draw3dText( const btVector3& location, const char* textString ) override
        {
        }

      void setDebugMode( int mode ) override
        {
          mDebugMode = mode;
          if ( mDebugMode == DBG_NoDebug )
            clear( );
        }

      void clear( )
        {
          mLines.clear( );
        }

      int getDebugMode( ) const override { return mDebugMode; }
    };
  } // end namespace
} // end namespace

#endif // __PUJ_Ogre__OgreBullet__h__

// eof - $RCSfile$
