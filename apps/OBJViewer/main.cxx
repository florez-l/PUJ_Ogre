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

#include <PUJ_Ogre/BaseApplication.h>

/**
 */
class Application
  : public PUJ_Ogre::BaseApplication
{
public:
  using Superclass  = PUJ_Ogre::BaseApplication;

public:
  Application(
    const std::string& fname,
    const std::string& app_name,
    const std::string& resources = "resources.cfg"
    )
    : Superclass( app_name, resources ),
      m_FileName( fname )
    {
    }
  virtual ~Application( ) override = default;

protected:
  virtual void _loadScene( ) override
    {
      auto* root = this->getRoot( );
      auto* root_node = this->m_SceneMgr->getRootSceneNode( );

      // Load mesh
      Ogre::AxisAlignedBox bbox;
      this->_loadMeshFromUnconventionalFile( bbox, this->m_FileName );
      // bbox.scale( Ogre::Vector3( 2, 2, 2 ) );

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

protected:
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
