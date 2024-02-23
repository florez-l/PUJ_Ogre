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
    );
  virtual ~Application( ) override = default;

  virtual bool keyPressed( const OgreBites::KeyboardEvent& evt ) override;
  virtual bool frameStarted( const Ogre::FrameEvent& evt ) override;

protected:

  virtual void _configureCamera( const Ogre::AxisAlignedBox& bbox ) override;
  virtual void _loadScene( ) override;
  virtual void _connect_Ogre_Bullet( ) override;

protected:
  Ogre::AxisAlignedBox m_BBox;
  Ogre::SceneNode* m_Field;
  std::pair< Ogre::SceneNode*, btRigidBody* > m_Tejo;

  bool m_Simulating { false };
};

int main( int argc, char** argv )
{
  Application app( "Tejo!" );
  app.go( );

  return( EXIT_SUCCESS );
}

// -------------------------------------------------------------------------
Application::
Application( const std::string& app_name, const std::string& resources )
  : Superclass( app_name, resources, false )
{
}

// -------------------------------------------------------------------------
bool Application::
keyPressed( const OgreBites::KeyboardEvent& evt )
{
  if( evt.keysym.sym == 'g' )
    this->m_Simulating = !( this->m_Simulating );
  return( this->Superclass::keyPressed( evt ) );
}

// -------------------------------------------------------------------------
bool Application::
frameStarted( const Ogre::FrameEvent& evt )
{
  if( !( this->m_Simulating ) )
  {
    Ogre::SceneNode* cam = this->m_CamMan->getCamera( );
    Ogre::Vector3 dir = cam->getLocalAxes( ).GetColumn( 2 );
    Ogre::Vector3 pos = cam->getPosition( ) - ( 2 * dir );
    this->m_Tejo.first->setPosition( pos );
    auto tr = this->m_Tejo.second->getWorldTransform( );
    tr.setOrigin( btVector3( pos[ 0 ], pos[ 1 ], pos[ 2 ] ) );
    this->m_Tejo.second->setWorldTransform( tr );
    this->m_Tejo.second->setLinearVelocity(
      btVector3( dir[ 0 ], dir[ 1 ], dir[ 2 ] ) * ( -15 )
      );
  }
  else
    this->m_BulletWorld->getBtWorld( )
      ->stepSimulation( evt.timeSinceLastFrame, 1 );

  return( this->Superclass::frameStarted( evt ) );
}

// -------------------------------------------------------------------------
void Application::
_configureCamera( const Ogre::AxisAlignedBox& bbox )
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

// -------------------------------------------------------------------------
void Application::
_loadScene( )
{
  auto* root = this->getRoot( );
  auto* root_node = this->m_SceneMgr->getRootSceneNode( );

  // Load tejo
  auto tejo = this->_loadOBJ( this->m_BBox, "sphere.obj" )[ 0 ];
  auto tejo_mesh = tejo->convertToMesh( "tejo_mesh" );
  auto tejo_entity =
    this->m_SceneMgr->createEntity( "tejo_entity", "tejo_mesh" );
  this->m_Tejo.first = root_node->createChildSceneNode( );
  this->m_Tejo.first->attachObject( tejo_entity );

  // Load field
  auto field = this->_loadOBJ( this->m_BBox, "cancha.obj" )[ 0 ];
  auto field_mesh = field->convertToMesh( "field_mesh" );
  auto field_entity =
    this->m_SceneMgr->createEntity( "field_entity", "field_mesh" );
  this->m_Field = root_node->createChildSceneNode( );
  this->m_Field->attachObject( field_entity );

  // Position all objects
  this->m_Tejo.first->setPosition( this->m_BBox.getCenter( ) );

  // Configure camera
  this->_configureCamera( this->m_BBox );

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

// -------------------------------------------------------------------------
void Application::
_connect_Ogre_Bullet( )
{
  auto tejo =
    dynamic_cast< Ogre::Entity* >(
      this->m_Tejo.first->getAttachedObject( 0 )
      );
  this->m_Tejo.second = this->m_BulletWorld
    ->addRigidBody( 1, tejo, PUJ_Ogre::Bullet::CT_SPHERE );

  auto field =
    dynamic_cast< Ogre::Entity* >( this->m_Field->getAttachedObject( 0 ) );
  this->m_BulletWorld
    ->addCollisionObject( field, PUJ_Ogre::Bullet::CT_TRIMESH );
}

// eof - $RCSfile$
