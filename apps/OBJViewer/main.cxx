// =========================================================================
// @author Leonardo Florez-Valencia (florez-l@javeriana.edu.co)
// =========================================================================

#include <cstdio>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <unistd.h>
#include <PUJ_Ogre/OBJReader.h>

#include <Ogre.h>
#include <OgreApplicationContext.h>
#include <OgreCameraMan.h>

/**
 */
class Application
  : public OgreBites::ApplicationContext,
    public OgreBites::InputListener
{
public:
  using TContext  = OgreBites::ApplicationContext;
  using TListener = OgreBites::InputListener;

public:
  Application(
    const std::string& app_name,
    const std::string& resources = "resources.cfg"
    )
    : TContext( app_name ),
      TListener( )
    {
      this->m_Resources =
        std::filesystem::canonical( std::filesystem::path( resources ) )
        .string( );
    }
  virtual ~Application( ) override = default;

  // Configuration
  virtual void loadResources( ) override
    {
      this->enableShaderCache( );
      Ogre::ConfigFile cf = Ogre::ConfigFile( );
      cf.loadDirect( this->m_Resources );

      auto res_mgr = Ogre::ResourceGroupManager::getSingletonPtr( );
      auto settings = cf.getSettingsBySection( );
      for( auto sIt = settings.begin( ); sIt != settings.end( ); ++sIt )
        for( auto fIt = sIt->second.begin( ); fIt != sIt->second.end( ); ++fIt )
          res_mgr->addResourceLocation( fIt->second, fIt->first, sIt->first );

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
  virtual void setup( ) override
    {
      this->_initSceneManager( );
      this->_loadScene( );
      this->_configureCamera( );
      this->setWindowGrab( true );
    }

  // Iterative methods
  /* TODO
     virtual bool frameStarted( const Ogre::FrameEvent& evt ) override;
     virtual bool frameRenderingQueued( const Ogre::FrameEvent& evt ) override;
  */
  virtual bool keyPressed( const OgreBites::KeyboardEvent& evt ) override
    {
      if( evt.keysym.sym == OgreBites::SDLK_ESCAPE )
        this->getRoot( )->queueEndRendering( );
      return( true );
    }

  // Main method
  virtual void go( )
    {
      this->initApp( );
      this->getRoot( )->startRendering( );
      this->closeApp( );
    }

protected:
  virtual void _initSceneManager( )
    {
      this->TContext::setup( );
      this->addInputListener( this );

      auto* root = this->getRoot( );
      this->m_SceneMgr = root->createSceneManager( "DefaultSceneManager" );
      this->m_SceneMgr->setAmbientLight( Ogre::ColourValue( 0.9, 0.9, 0.9 ) );

      // register our scene with the RTSS
      auto* shadergen = Ogre::RTShader::ShaderGenerator::getSingletonPtr( );
      shadergen->addSceneManager( this->m_SceneMgr );
    }
  virtual void _configureCamera( )
    {
      auto* cam = this->m_SceneMgr->getCameras( ).begin( )->second;
      cam->setAutoAspectRatio( true );

      this->m_CamMan = new OgreBites::CameraMan( cam->getParentSceneNode( ) );
      this->m_CamMan->setStyle( OgreBites::CS_FREELOOK );
      this->m_CamMan->setTopSpeed( 3 );
      this->m_CamMan->setFixedYaw( true );
      this->addInputListener( this->m_CamMan );
      this->getRenderWindow( )->addViewport( cam );
    }
  virtual void _loadScene( )
    {
    }

protected:
  std::string           m_Resources;
  Ogre::SceneManager*   m_SceneMgr;
  OgreBites::CameraMan* m_CamMan;
};


int main( int argc, char** argv )
{
  // Create temporary resources file
  std::stringstream str;
  str
    << std::filesystem::path( argv[ 0 ] ).filename( ).stem( ).string( )
    << "_resources_XXXXXX";
  std::string fname =
    ( std::filesystem::temp_directory_path( ) / str.str( ) ).string( );
  char* filename = new char[ fname.size( ) + 1 ];
  std::strcpy( filename, fname.c_str( ) );
  int fd = mkstemp( filename );
  if( fd == -1 )
  {
    std::cerr
      << "Error: could not create temporary resources file."
      << std::endl;
    return( EXIT_FAILURE );
  } // end if

  // Save resources
  std::stringstream data;
  data
    << "[General]" << std::endl
    << "FileSystem="
    << std::filesystem::path( argv[ 1 ] ).parent_path( ).string( );
  write( fd, data.str( ).c_str( ), data.str( ).size( ) );

  // Execute application
  Application app( "OBJViewer", filename );
  app.go( );

  // Close application
  close( fd );
  unlink( filename );
  delete filename;
  return( EXIT_SUCCESS );
}

// eof - $RCSfile$
