// =========================================================================
// @author Leonardo Florez-Valencia (florez-l@javeriana.edu.co)
// =========================================================================

#include <PUJ_Ogre/BaseApplicationWithBullet.h>

// -------------------------------------------------------------------------
PUJ_Ogre::BaseApplicationWithBullet::
BaseApplicationWithBullet(
  const std::string& app_name, const std::string& resources
  )
  : Superclass( app_name, resources )
{
}

// -------------------------------------------------------------------------
PUJ_Ogre::BaseApplicationWithBullet::
~BaseApplicationWithBullet( )
{
  for(
    int i = this->m_BulletDynamicsWorld->getNumCollisionObjects( ) - 1;
    i >= 0;
    i--
    )
  {
    btCollisionObject* obj =
      this->m_BulletDynamicsWorld->getCollisionObjectArray( )[ i ];
    btRigidBody* body = btRigidBody::upcast( obj );
    if( body && body->getMotionState( ) )
      delete body->getMotionState( );
    this->m_BulletDynamicsWorld->removeCollisionObject( obj );
    delete obj;
  } // end for

  for( int j = 0; j < this->m_BulletCollisionShapes.size( ); j++ )
  {
    btCollisionShape* shape = this->m_BulletCollisionShapes[ j ];
    this->m_BulletCollisionShapes[ j ] = 0;
    delete shape;
  } // end for

  delete this->m_BulletDynamicsWorld;
  delete this->m_BulletSolver;
  delete this->m_BulletOverlappingPairCache;
  delete this->m_BulletDispatcher;
  delete this->m_BulletCollisionConfiguration;
  this->m_BulletCollisionShapes.clear( );
}

// -------------------------------------------------------------------------
void PUJ_Ogre::BaseApplicationWithBullet::
setup( )
{
  this->Superclass::setup( );

  this->m_BulletCollisionConfiguration =
    new btDefaultCollisionConfiguration( );
  this->m_BulletDispatcher =
    new btCollisionDispatcher( this->m_BulletCollisionConfiguration );
  this->m_BulletOverlappingPairCache = new btDbvtBroadphase( );
  this->m_BulletSolver = new btSequentialImpulseConstraintSolver( );
  this->m_BulletDynamicsWorld =
    new btDiscreteDynamicsWorld(
      this->m_BulletDispatcher,
      this->m_BulletOverlappingPairCache,
      this->m_BulletSolver,
      this->m_BulletCollisionConfiguration
      );
  this->m_BulletDynamicsWorld->setGravity( btVector3( 0, -9.8, 0 ) );


  /* TODO
     btAlignedObjectArray< btCollisionShape* > m_BulletCollisionShapes;
  */

  this->_connect_Ogre_Bullet( );
}

// eof - $RCSfile$
