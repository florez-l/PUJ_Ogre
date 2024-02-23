// =========================================================================
// @author Leonardo Florez-Valencia (florez-l@javeriana.edu.co)
// =========================================================================

#include <PUJ_Ogre/BaseApplication.h>

#include <iostream>
#include <filesystem>

#include <Ogre.h>
#include <OgreCameraMan.h>

#include <PUJ_Ogre/OBJReader.h>

// -------------------------------------------------------------------------
PUJ_Ogre::BaseApplication::
BaseApplication(
  const std::string& app_name,
  const std::string& resources,
  bool real_resources_file
  )
  : TContext( app_name ),
    TListener( ),
    m_RealResourcesFile( real_resources_file )
{
  this->m_Resources =
    std::filesystem::canonical( std::filesystem::path( resources ) )
    .string( );
}

// -------------------------------------------------------------------------
void PUJ_Ogre::BaseApplication::
locateResources( )
{
}

// -------------------------------------------------------------------------
void PUJ_Ogre::BaseApplication::
loadResources( )
{
  auto res_mgr = Ogre::ResourceGroupManager::getSingletonPtr( );

  if( this->m_RealResourcesFile )
  {
    this->enableShaderCache( );
    Ogre::ConfigFile cf = Ogre::ConfigFile( );
    cf.loadDirect( this->m_Resources );

    auto settings = cf.getSettingsBySection( );
    for( auto s = settings.begin( ); s != settings.end( ); ++s )
      for( auto f = s->second.begin( ); f != s->second.end( ); ++f )
        res_mgr->addResourceLocation( f->second, f->first, s->first );
  }
  else
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

// -------------------------------------------------------------------------
void PUJ_Ogre::BaseApplication::
setup( )
{
  this->_initSceneManager( );
  this->_loadScene( );
  this->setWindowGrab( true );
}

// -------------------------------------------------------------------------
bool PUJ_Ogre::BaseApplication::
keyPressed( const OgreBites::KeyboardEvent& evt )
{
  if( evt.keysym.sym == OgreBites::SDLK_ESCAPE )
    this->getRoot( )->queueEndRendering( );
  return( true );
}

// -------------------------------------------------------------------------
void PUJ_Ogre::BaseApplication::
go( )
{
  this->initApp( );
  this->getRoot( )->startRendering( );
  this->closeApp( );
}

// -------------------------------------------------------------------------
void PUJ_Ogre::BaseApplication::
_initSceneManager( )
{
  this->TContext::setup( );
  this->addInputListener( this );

  auto* root = this->getRoot( );
  this->m_SceneMgr = root->createSceneManager( "DefaultSceneManager" );

  // register our scene with the RTSS
  auto* shadergen = Ogre::RTShader::ShaderGenerator::getSingletonPtr( );
  shadergen->addSceneManager( this->m_SceneMgr );
}

// -------------------------------------------------------------------------
void PUJ_Ogre::BaseApplication::
_configureCamera( const Ogre::AxisAlignedBox& bbox )
{
  auto* root = this->getRoot( );
  auto* root_node = this->m_SceneMgr->getRootSceneNode( );

  // Configure camera
  auto cam = this->m_SceneMgr->createCamera( "MainCamera" );
  cam->setNearClipDistance( 0.005 );
  cam->setAutoAspectRatio( true );

  auto camnode = root_node->createChildSceneNode( );
  // TODO: camnode->setPosition( bbox.getMaximum( ) );
  camnode->setPosition( Ogre::Vector3( 2.25, 1.05, 9.75 ) );

  // camnode->lookAt( bbox.getMaximum( ), Ogre::Node::TS_WORLD );
  camnode->lookAt( Ogre::Vector3( 0, 0, 0 ), Ogre::Node::TS_WORLD );
  camnode->attachObject( cam );

  this->m_CamMan = new OgreBites::CameraMan( camnode );
  this->m_CamMan->setStyle( OgreBites::CS_ORBIT /*FREELOOK*/ );
  this->m_CamMan->setTopSpeed( 3 );
  this->m_CamMan->setFixedYaw( true );
  this->addInputListener( this->m_CamMan );
  auto vp = this->getRenderWindow( )->addViewport( cam );
  vp->setBackgroundColour( Ogre::ColourValue( 0.9, 0.75, 0.5 ) );
}

// -------------------------------------------------------------------------
std::vector< Ogre::ManualObject* > PUJ_Ogre::BaseApplication::
_loadOBJ( Ogre::AxisAlignedBox& bbox, const std::string& fname )
{
  PUJ_Ogre::OBJReader reader;
  auto c = Ogre::ResourceGroupManager::getSingleton( ).openResource( fname );
  std::istringstream input( c->getAsString( ) );
  reader.read( input, true );
  const auto& buffer = reader.buffer( );

  bool start_bb = true;
  bbox = Ogre::AxisAlignedBox( );
  std::vector< Ogre::ManualObject* > objects;
  for( const auto& o: buffer )
  {
    for( const auto& g: o.second )
    {
      std::stringstream n;
      n << o.first << "_" << g.first;
      const auto& geom = std::get< 0 >( g.second );
      const auto& tria = std::get< 1 >( g.second );
      const auto& quad = std::get< 2 >( g.second );

      Ogre::ManualObject* man =
        this->m_SceneMgr->createManualObject( n.str( ) );

      n << "_material";
      man->begin(
        n.str( ), Ogre::RenderOperation::OT_TRIANGLE_LIST
        );

      for( unsigned long long i = 0; i < geom.size( ); i += 8 )
      {
        man->position( geom[ i ], geom[ i + 1 ], geom[ i + 2 ] );
        man->normal( geom[ i + 3 ], geom[ i + 4 ], geom[ i + 5 ] );
        man->textureCoord( geom[ i + 6 ], geom[ i + 7 ] );
      } // end for

      for( unsigned long long i = 0; i < tria.size( ); i += 3 )
        man->triangle( tria[ i ], tria[ i + 1 ], tria[ i + 2 ] );

      for( unsigned long long i = 0; i < quad.size( ); i += 4 )
        man->quad(
          quad[ i ], quad[ i + 1 ], quad[ i + 2 ], quad[ i + 3 ]
          );

      man->end( );

      objects.push_back( man );

      Ogre::AxisAlignedBox bb = man->getBoundingBox( );
      if( start_bb )
      {
        Ogre::Vector3 minV = bbox.getMinimum( );
        Ogre::Vector3 maxV = bbox.getMaximum( );
        for( unsigned int i = 0; i < 3; ++i )
        {
          minV[ i ] =
            ( bb.getMinimum( )[ i ] < minV[ i ] )?
            bb.getMinimum( )[ i ]: minV[ i ];
          maxV[ i ] =
            ( bb.getMaximum( )[ i ] > maxV[ i ] )?
            bb.getMaximum( )[ i ]: maxV[ i ];
        } // end for

        bbox.setMinimum( minV );
        bbox.setMaximum( maxV );
        start_bb = false;
      }
      else
        bbox = bb;

    } // end for
  } // end for
  return( objects );
}

// eof - $RCSfile$
