// =========================================================================
// @author Leonardo Florez-Valencia (florez-l@javeriana.edu.co)
// =========================================================================

/* TODO
material Template/Red
{
   technique
   {
        pass solidPass
        {
            // sets your object's colour, texture etc.
         lighting on

         ambient 0.3 0.1 0.1 1
         diffuse 0.8 0.05 0.05 1
         emissive 1 0 0 1
            // ... leave what you have here

            polygon_mode solid // sets to render the object as a solid

        }

        pass wireframePass
        {
         lighting on
         ambient 0 0 1 1
         diffuse 0 0 1 1
         emissive 0 0 1 1
            polygon_mode wireframe // sets to render the object as a wireframe
        }

   }
}
*/

#include <cstdio>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <unistd.h>
#include <PUJ_Ogre/OBJReader.h>

#include <Ogre.h>
#include <OgreApplicationContext.h>
#include <OgreCameraMan.h>

#include <PUJ_Ogre/OBJReader.h>

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
    const std::string& fname,
    const std::string& app_name,
    const std::string& resources = "resources.cfg"
    )
    : TContext( app_name ),
      TListener( )
    {
      this->m_Resources =
        std::filesystem::canonical( std::filesystem::path( resources ) )
        .string( );
      this->m_FileName = fname;
    }
  virtual ~Application( ) override = default;

  // Configuration
  virtual void locateResources( ) override
    {
    }
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

      // register our scene with the RTSS
      auto* shadergen = Ogre::RTShader::ShaderGenerator::getSingletonPtr( );
      shadergen->addSceneManager( this->m_SceneMgr );
    }
  virtual void _configureCamera( const Ogre::AxisAlignedBox& bbox )
    {
      auto* root = this->getRoot( );
      auto* root_node = this->m_SceneMgr->getRootSceneNode( );

      // Configure camera
      auto cam = this->m_SceneMgr->createCamera( "MainCamera" );
      cam->setNearClipDistance( 0.005 );
      cam->setAutoAspectRatio( true );
      auto camnode = root_node->createChildSceneNode( );
      camnode->setPosition( bbox.getMaximum( ) );
      camnode->lookAt( bbox.getCenter( ), Ogre::Node::TS_WORLD );
      camnode->attachObject( cam );

      cam->setAutoAspectRatio( true );

      this->m_CamMan = new OgreBites::CameraMan( cam->getParentSceneNode( ) );
      this->m_CamMan->setStyle( OgreBites::CS_ORBIT /*FREELOOK*/ );
      this->m_CamMan->setTopSpeed( 3 );
      this->m_CamMan->setFixedYaw( true );
      this->addInputListener( this->m_CamMan );
      auto vp = this->getRenderWindow( )->addViewport( cam );
      vp->setBackgroundColour( Ogre::ColourValue( 0.9, 0.75, 0.5 ) );
    }
  virtual void _loadScene( )
    {
      auto* root = this->getRoot( );
      auto* root_node = this->m_SceneMgr->getRootSceneNode( );

      // Load mesh
      Ogre::AxisAlignedBox bbox;
      this->_loadMeshFromUnconventionalFile( bbox, this->m_FileName );
      bbox.scale( Ogre::Vector3( 2, 2, 2 ) );

      // Configure lights
      this->m_SceneMgr->setAmbientLight( Ogre::ColourValue( 1, 1, 1 ) );

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

      // Configure camera
      this->_configureCamera( bbox );
    }

  virtual std::vector< Ogre::ManualObject* > _loadMeshFromUnconventionalFile(
    Ogre::AxisAlignedBox& bbox,
    const std::string& fname
    )
    {
      PUJ_Ogre::OBJReader reader;
      auto c = Ogre::ResourceGroupManager::getSingleton( )
        .openResource( fname );
      std::istringstream input( c->getAsString( ) );
      reader.read( input, true );
      const auto& buffer = reader.buffer( );

      bbox = Ogre::AxisAlignedBox(
        std::numeric_limits< Ogre::Real >::max( ),
        std::numeric_limits< Ogre::Real >::max( ),
        std::numeric_limits< Ogre::Real >::max( ),
        std::numeric_limits< Ogre::Real >::lowest( ),
        std::numeric_limits< Ogre::Real >::lowest( ),
        std::numeric_limits< Ogre::Real >::lowest( )
        );

      std::vector< Ogre::ManualObject* > objects;
      for( const auto& o: buffer )
      {
        for( const auto& g: o.second )
        {
          std::stringstream n;
          n << o.first << "_" << g.first << std::endl;
          const auto& geom = std::get< 0 >( g.second );
          const auto& tria = std::get< 1 >( g.second );
          const auto& quad = std::get< 2 >( g.second );

          Ogre::ManualObject* man =
            this->m_SceneMgr->createManualObject( n.str( ) );
          man->begin(
            "Template/Red", Ogre::RenderOperation::OT_TRIANGLE_LIST
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

          this->m_SceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(man);

          objects.push_back( man );

          Ogre::AxisAlignedBox bb = man->getBoundingBox( );
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

        } // end for
      } // end for
      return( objects );
    }

protected:
  std::string           m_Resources;
  Ogre::SceneManager*   m_SceneMgr;
  OgreBites::CameraMan* m_CamMan;

  std::string m_FileName { "" };
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
  Application app( argv[ 1 ], "OBJViewer", filename );
  app.go( );

  // Close application
  close( fd );
  unlink( filename );
  delete filename;
  return( EXIT_SUCCESS );
}

// eof - $RCSfile$
