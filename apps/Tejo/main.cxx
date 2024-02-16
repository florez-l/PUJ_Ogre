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

#include <iostream>

/* TODO
   #include <filesystem>
   #include <cstdio>
   #include <cstring>
   #include <sstream>
   #include <unistd.h>
*/

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

      /* TODO
         ResourceGroupManager::getSingleton().addResourceLocation("asset/table.zip", "Zip", "Scene");
      */
      res_mgr->addResourceLocation( this->m_Resources, "Zip", "General" );

      /* TODO
         auto settings = cf.getSettingsBySection( );
         for( auto sIt = settings.begin( ); sIt != settings.end( ); ++sIt )
         for( auto fIt = sIt->second.begin( ); fIt != sIt->second.end( ); ++fIt )
         res_mgr->addResourceLocation( fIt->second, fIt->first, sIt->first );
      */
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

protected:
  virtual void _loadScene( ) override
    {
      auto* root = this->getRoot( );
      auto* root_node = this->m_SceneMgr->getRootSceneNode( );

      // Load field
      Ogre::AxisAlignedBox bbox;
      auto objs = this->_loadMeshFromUnconventionalFile( bbox, "cancha.obj" );
      for( auto o: objs )
        root_node->createChildSceneNode( )->attachObject( o );

      // TODO: center -> Vector3(2.25, 2, 9.75)

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

      Ogre::Light* l = this->m_SceneMgr->createLight( "main_light" );
      l->setDiffuseColour( 0.1, 1, 1 );
      l->setSpecularColour( 0.1, 1, 1 );
      l->setType( Ogre::Light::LT_POINT );

      Ogre::SceneNode* ln = root_node->createChildSceneNode( );
      ln->attachObject( l );
      ln->setPosition( Ogre::Vector3( 2.25, 1.05, 9.75 ) );
      std::cout << bbox.getCenter( ) << std::endl;

      /* TODO
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

      // Configure camera
      this->_configureCamera( bbox );
    }
};


int main( int argc, char** argv )
{
  // Create temporary resources file
  // TODO: 
  /* TODO
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
  */

  // Execute application
  Application app( "Tejo!" );
  app.go( );

  // Close application
  /* TODO
     close( fd );
     unlink( filename );
     delete filename;
  */
  return( EXIT_SUCCESS );
}

// eof - $RCSfile$
