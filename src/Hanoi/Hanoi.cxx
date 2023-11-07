// -------------------------------------------------------------------------
// @author Leonardo Florez-Valencia (florez-l@javeriana.edu.co)
// -------------------------------------------------------------------------

#include <iostream>
#include <PUJ_Ogre/BaseApplication.h>
#include <PUJ_Ogre/CameraMan.h>

#include <climits>
#include <filesystem>
#include <sstream>
#include <unistd.h>

#include <Ogre.h>

/**
 */
class Hanoi
  : public PUJ_Ogre::BaseApplication
{
public:
  Hanoi( const std::string& work_dir );
  virtual ~Hanoi( ) override;

protected:
  virtual void _loadScene( ) override;
};

// -------------------------------------------------------------------------
int main( int argc, char** argv )
{
  std::filesystem::path p( argv[ 0 ] );
  Hanoi app( p.parent_path( ).string( ) );
  app.go( );
  return( EXIT_SUCCESS );
}

// -------------------------------------------------------------------------
Hanoi::
Hanoi( const std::string& work_dir )
  : PUJ_Ogre::BaseApplication( "Hanoi v1.0", "" )
{
  static char name_template[] = "tmp_Hanoi_XXXXXXXXX";
  char fname[ PATH_MAX ];

  std::stringstream contents;
  contents << "[General]" << std::endl;
  contents << "Zip=" << work_dir << "/Hanoi_resources.zip" << std::endl;

  std::strcpy( fname, name_template );
  int fd = mkstemp( fname );
  write( fd, contents.str( ).c_str( ), contents.str( ).size( ) );
  close( fd );
  this->m_ResourcesFile = fname;
}

// -------------------------------------------------------------------------
Hanoi::
~Hanoi( )
{
  unlink( this->m_ResourcesFile.c_str( ) );
}

// -------------------------------------------------------------------------
void Hanoi::
_loadScene( )
{
  auto* root = this->getRoot( );
  auto* root_node = this->m_SceneMgr->getRootSceneNode( );

  // Load room geometry
  auto room = this->_load_using_vtk( "Hanoi_resources/room.obj", "room" );
  auto bb = room->getBoundingBox( );
  auto cog = bb.getMaximum( ) + bb.getMinimum( );
  cog *= 0.5;

  // Configure illumination
  this->m_SceneMgr->setAmbientLight( Ogre::ColourValue( 1, 1, 1 ) );

  auto light = this->m_SceneMgr->createLight( "MainLight" );
  auto lightnode = root_node->createChildSceneNode( );
  lightnode->setPosition( cog );
  lightnode->attachObject( light );

  // Configure camera
  auto cam = this->m_SceneMgr->createCamera( "MainCamera" );
  cam->setNearClipDistance( 0.005 );
  cam->setAutoAspectRatio( true );
  auto camnode = root_node->createChildSceneNode( );
  camnode->setPosition( cog );
  camnode->attachObject( cam );

  this->m_CamMan = new PUJ_Ogre::CameraMan( camnode, bb );
  this->addInputListener( this->m_CamMan );

  auto vp = this->getRenderWindow( )->addViewport( cam );
  vp->setBackgroundColour( Ogre::ColourValue( 0, 0, 0 ) );

  // Meshes
  auto room_mesh = room->convertToMesh( "room", "General" );
  auto room_ent = this->m_SceneMgr->createEntity( "room" );
  this->m_SceneMgr->getRootSceneNode( )->attachObject( room_ent );

  auto table = this->_load_using_vtk( "Hanoi_resources/table.stl", "room" );
  auto table_mesh = table->convertToMesh( "table", "General" );
  auto table_ent = this->m_SceneMgr->createEntity( "table" );
  this->m_SceneMgr->getRootSceneNode( )->attachObject( table_ent );
}

// eof - $RCSfile$
