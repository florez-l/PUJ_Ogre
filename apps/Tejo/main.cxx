// =========================================================================
// @author Leonardo Florez-Valencia (florez-l@javeriana.edu.co)
// =========================================================================

#include <iostream>
#include <PUJ_Ogre/BaseApplicationWithBullet.h>

#include <Ogre.h>
#include <OgreCameraMan.h>

/**
 */
class Application
  : public PUJ_Ogre::BaseApplicationWithBullet
{
public:
  using Superclass  = PUJ_Ogre::BaseApplicationWithBullet;

public:
  Application(
    const std::string& app_name,
    const std::string& resources = "PUJ_Ogre_Tejo_resources.zip"
    )
    : Superclass( app_name, resources )
    {
    }
  virtual ~Application( ) override = default;

  virtual void loadResources( ) override
    {
      auto res_mgr = Ogre::ResourceGroupManager::getSingletonPtr( );
      res_mgr->addResourceLocation( this->m_Resources, "Zip", "General" );
      try
      {
        res_mgr->initialiseAllResourceGroups( );
        res_mgr->loadResourceGroup( "General" );
      }
      catch( ... )
      {
        // Do nothing
      }
    }

  virtual bool keyPressed( const OgreBites::KeyboardEvent& evt ) override
    {
      if( evt.keysym.sym == 'g' )
      {
        this->m_Simulating = true;
      }
      return( this->Superclass::keyPressed( evt ) );
    }

  virtual bool frameStarted( const Ogre::FrameEvent& evt ) override
    {
      if( this->m_Simulating )
      {
        auto dw = this->m_BulletWorld->getBtWorld( );

        dw->stepSimulation( evt.timeSinceLastFrame, 2 );

        //print positions of all objects
        /* TODO
           for (int j = dw->getNumCollisionObjects() - 1; j >= 0; j--)
           {
           btCollisionObject* obj = dw->getCollisionObjectArray()[j];
           btRigidBody* body = btRigidBody::upcast(obj);
           btTransform trans;
           if (body && body->getMotionState())
           {
           body->getMotionState()->getWorldTransform(trans);
           }
           else
           {
           trans = obj->getWorldTransform();
           }
           printf("world pos object %d = %f,%f,%f\n", j, float(trans.getOrigin().getX()), float(trans.getOrigin().getY()), float(trans.getOrigin().getZ()));
           }
         */
      } // end if
      return( this->Superclass::frameStarted( evt ) );
    }

protected:

  virtual void _configureCamera( const Ogre::AxisAlignedBox& bbox ) override
    {
      auto* root = this->getRoot( );
      auto* root_node = this->m_SceneMgr->getRootSceneNode( );

      // Configure camera
      auto cam = this->m_SceneMgr->createCamera( "Camera" );
      cam->setNearClipDistance( 0.005 );
      cam->setAutoAspectRatio( true );
      auto vp = this->getRenderWindow( )->addViewport( cam );
      vp->setBackgroundColour( Ogre::ColourValue( 0, 0, 0 ) );

      auto camnode = root_node->createChildSceneNode( );
      camnode->attachObject( cam );

      this->m_CamMan = new OgreBites::CameraMan( camnode );
      this->m_CamMan->setStyle( OgreBites::CS_FREELOOK );
      this->m_CamMan->setTopSpeed( 1.2 );
      this->m_CamMan->setFixedYaw( true );
      this->addInputListener( this->m_CamMan );

      camnode->setPosition( bbox.getCenter( ) );
      camnode->lookAt( Ogre::Vector3( 0, 0, -1 ), Ogre::Node::TS_WORLD );
    }

  virtual void _loadScene( ) override
    {
      auto* root = this->getRoot( );
      auto* root_node = this->m_SceneMgr->getRootSceneNode( );

      // Load tejo
      auto tejo =
        this->_loadMeshFromUnconventionalFile( this->m_BBox, "sphere.obj" )[ 0 ];
      auto tejo_mesh = tejo->convertToMesh( "tejo_mesh" );
      auto tejo_entity = this->m_SceneMgr->createEntity( "tejo_entity", "tejo_mesh" );
      this->m_Tejo = root_node->createChildSceneNode( );
      this->m_Tejo->attachObject( tejo_entity );

      // Load field
      auto field =
        this->_loadMeshFromUnconventionalFile( this->m_BBox, "cancha.obj" )[ 0 ];
      auto field_mesh = field->convertToMesh( "field_mesh" );
      auto field_entity = this->m_SceneMgr->createEntity( "field_entity", "field_mesh" );
      this->m_Field = root_node->createChildSceneNode( );
      this->m_Field->attachObject( field_entity );

      // Position all objects
      this->m_Tejo->setPosition( this->m_BBox.getCenter( ) );

      // Configure camera
      this->_configureCamera( this->m_BBox );

      /* TODO
         for( auto o: tejo )
         root_node->createChildSceneNode( )->attachObject( o );
      */

      // Load mesh
      /* TODO
         Ogre::AxisAlignedBox bbox, bbox2;
         auto objects = this->_loadMeshFromUnconventionalFile(
         bbox, this->m_FileName
         );
         for( auto o: objects )
         root_node->createChildSceneNode( )->attachObject( o );

         Ogre::Real sc = 2;
         Ogre::Matrix4 tr(
         sc, 0, 0, bbox.getCenter( )[ 0 ] * ( 1 - sc ),
         0, sc, 0, bbox.getCenter( )[ 1 ] * ( 1 - sc ),
         0, 0, sc, bbox.getCenter( )[ 2 ] * ( 1 - sc ),
         0, 0, 0, 1
         );
         bbox2 = bbox;
         bbox2.transform( tr );
      */

      // Configure lights
      this->m_SceneMgr->setAmbientLight( Ogre::ColourValue( 0.1, 0.1, 0 ) );

      /* TODO
         Ogre::Light* l = this->m_SceneMgr->createLight( "main_light" );
         l->setDiffuseColour( 0.1, 1, 1 );
         l->setSpecularColour( 0.1, 1, 1 );
         l->setType( Ogre::Light::LT_POINT );

         Ogre::SceneNode* ln = root_node->createChildSceneNode( );
         ln->attachObject( l );
         ln->setPosition( Ogre::Vector3( 2.25, 1.05, 9.75 ) );
         std::cout << bbox.getCenter( ) << std::endl;

         auto corners = bbox.getAllCorners( );
         for( unsigned int i = 0; i < 8; ++i )
         {
         std::stringstream n;
         n << "light_" << i;
         Ogre::Light* l = this->m_SceneMgr->createLight( n.str( ) );
         l->setDiffuseColour( 1, 1, 1 );
         l->setSpecularColour( 1, 1, 1 );
         l->setType( Ogre::Light::LT_POINT );

         Ogre::SceneNode* ln = root_node->createChildSceneNode( );
         ln->attachObject( l );
         ln->setPosition( corners[ i ] );
         } // end for

      */
    }

  virtual void _connect_Ogre_Bullet( ) override
    {
      auto field =
        dynamic_cast< Ogre::Entity* >(
          this->m_Field->getAttachedObject( 0 )
          );
      this->m_BulletWorld->addCollisionObject( field, PUJ_Ogre::Bullet::CT_TRIMESH );
      // this->m_BulletWorld->addRigidBody( 0, field, PUJ_Ogre::Bullet::CT_TRIMESH );

      auto tejo =
        dynamic_cast< Ogre::Entity* >(
          this->m_Tejo->getAttachedObject( 0 )
          );
      this->m_BulletWorld->addRigidBody( 1, tejo, PUJ_Ogre::Bullet::CT_SPHERE );

      /* TODO
         btRigidBody* addRigidBody(
         float mass,
         Ogre::Entity* ent,
         ColliderType ct,
         CollisionListener* listener = nullptr,
         int group = 1, int mask = -1
         );

         btCollisionObject* addCollisionObject(
         Ogre::Entity* ent, ColliderType ct, int group = 1, int mask = -1
         );
      */
    }

protected:
  Ogre::AxisAlignedBox m_BBox;
  Ogre::SceneNode* m_Field;
  Ogre::SceneNode* m_Tejo;

  bool m_Simulating { false };
};

int main( int argc, char** argv )
{
  Application app( "Tejo!" );
  app.go( );

  return( EXIT_SUCCESS );
}

// eof - $RCSfile$
