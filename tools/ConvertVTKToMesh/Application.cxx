// =========================================================================
// @author Leonardo Florez-Valencia (florez-l@javeriana.edu.co)
// =========================================================================

#include "Application.h"

#include <filesystem>
#include <regex>

#include <Ogre.h>
#include <OgreCameraMan.h>

#include <vtkCellData.h>
#include <vtkPolyData.h>
#include <vtkPolyDataNormals.h>
#include <vtkPolyDataReader.h>
#include <vtkTriangleFilter.h>
#include <vtkNew.h>

// -------------------------------------------------------------------------
Application::
Application(
  const std::string& exec_path,
  const std::string& model_fname,
  bool point_normals
  )
  : TContext( "PUJ_Ogre_ConvertVTKToMesh" ),
    TListener( ),
    m_PointNormals( point_normals )
{
  // Model file
  auto m =
    std::filesystem::canonical( std::filesystem::path( model_fname ) );
  this->m_ModelFileName = m.string( );

  // Resources file
  auto p =
    std::filesystem::canonical( std::filesystem::path( exec_path ) ).
    parent_path( );
  p /= "resources.cfg";
  this->m_ResourcesFile = p.string( );
  std::ofstream r( this->m_ResourcesFile.c_str( ) );
  r
    << "[General]" << std::endl
    << "FileSystem=" << m.parent_path( ).string( )
    << std::endl;
  r.close( );

  // Materials file
  this->m_MaterialName = m.stem( ).string( );
  auto mat_file = m.parent_path( ) / "material.material";
  std::ofstream n( mat_file.string( ).c_str( ) );
  n << "material " << this->m_MaterialName << std::endl;
  n << "{" << std::endl;
  n << "   technique" << std::endl;
  n << "   {" << std::endl;
  n << "      pass" << std::endl;
  n << "      {" << std::endl;
  n << "         lighting on" << std::endl;
  n << std::endl;
  n << "         ambient 1 1 1 1" << std::endl;
  n << "         diffuse 1 1 1 1" << std::endl;
  n << "         emissive 0 0 0 1" << std::endl;
  n << "      }" << std::endl;
  n << "   }" << std::endl;
  n << "}" << std::endl;
  n.close( );
}

// -------------------------------------------------------------------------
void Application::
loadResources( )
{
  this->enableShaderCache( );
  Ogre::ConfigFile cf = Ogre::ConfigFile( );
  cf.loadDirect( this->m_ResourcesFile );

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

// -------------------------------------------------------------------------
void Application::
setup( )
{
  this->_initSceneManager( );
  this->_loadScene( );
  this->setWindowGrab( true );
}

// -------------------------------------------------------------------------
bool Application::
keyPressed( const OgreBites::KeyboardEvent& evt )
{
  if( evt.keysym.sym == OgreBites::SDLK_ESCAPE )
    this->getRoot( )->queueEndRendering( );
  return( true );
}


// -------------------------------------------------------------------------
void Application::
go( )
{
  this->initApp( );
  this->getRoot( )->startRendering( );
  this->closeApp( );
}

// -------------------------------------------------------------------------
void Application::
_initSceneManager( )
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

// -------------------------------------------------------------------------
void Application::
_loadScene( )
{
  // Load VTK model
  vtkNew< vtkPolyDataReader > reader;
  reader->SetFileName( this->m_ModelFileName.c_str( ) );

  vtkNew< vtkTriangleFilter > triangles;
  triangles->SetInputConnection( reader->GetOutputPort( ) );

  vtkNew< vtkPolyDataNormals > normals;
  normals->SetInputConnection( triangles->GetOutputPort( ) );
  if( this->m_PointNormals )
  {
    normals->ComputeCellNormalsOff( );
    normals->ComputePointNormalsOn( );
    normals->Update( );
    this->_meshWithPointNormals( normals->GetOutput( ) );
  }
  else
  {
    normals->ComputeCellNormalsOn( );
    normals->ComputePointNormalsOff( );
    normals->Update( );
    this->_meshWithFaceNormals( normals->GetOutput( ) );
  } // end if

  // Camera
  double bounds[ 6 ];
  normals->GetOutput( )->GetBounds( bounds );

  std::cout
    << bounds[ 0 ] << " "
    << bounds[ 1 ] << " "
    << bounds[ 2 ] << " "
    << bounds[ 3 ] << " "
    << bounds[ 4 ] << " "
    << bounds[ 5 ] << std::endl;
  std::exit( 1 );

  Ogre::Camera* cam = this->m_SceneMgr->createCamera( "Camera" );
  cam->setNearClipDistance( 1e-3 );
  cam->setFarClipDistance( 1e+3 );
  cam->setAutoAspectRatio( true );
  this->getRenderWindow( )->addViewport( cam );

  Ogre::SceneNode* node =
    this->m_SceneMgr->getRootSceneNode( )->createChildSceneNode( );
  node->attachObject( cam );

  this->m_CamMan = new OgreBites::CameraMan( node );
  this->addInputListener( this->m_CamMan );

  node->setPosition( bounds[ 0 ], bounds[ 2 ], bounds[ 4 ] );
  node->lookAt(
    Ogre::Vector3(
      bounds[ 1 ], bounds[ 3 ], bounds[ 5 ]
      ),
    Ogre::Node::TS_PARENT
    );

  // Lights
  Ogre::Light* light = this->m_SceneMgr->createLight( "MainLight" );
  node =
    this->m_SceneMgr->getRootSceneNode( )->createChildSceneNode( );
  node->setPosition(
    ( bounds[ 0 ] + bounds[ 1 ] ) * 0.5,
    bounds[ 3 ] * 2.5,
    ( bounds[ 4 ] + bounds[ 5 ] ) * 0.5
    );
  node->attachObject( light );

}

// -------------------------------------------------------------------------
void Application::
_meshWithPointNormals( vtkPolyData* pdata )
{
}

// -------------------------------------------------------------------------
void Application::
_meshWithFaceNormals( vtkPolyData* pdata )
{
  Ogre::ManualObject* man = this->m_SceneMgr->createManualObject( "man_obj" );
  man->begin( this->m_MaterialName, Ogre::RenderOperation::OT_TRIANGLE_LIST );

  vtkPoints* points = pdata->GetPoints( );
  vtkCellArray* polys = pdata->GetPolys( );
  vtkDataArray* normals = pdata->GetCellData( )->GetNormals( );

  unsigned long long j = 0;
  vtkNew< vtkIdList > cell_points;
  for( unsigned long long i = 0; i < polys->GetNumberOfCells( ); ++i, j += 3 )
  {
    cell_points->Initialize( );
    polys->GetCellAtId( i, cell_points );
    double* n = normals->GetTuple( i );
    double* a = points->GetPoint( cell_points->GetId( 0 ) );
    double* b = points->GetPoint( cell_points->GetId( 1 ) );
    double* c = points->GetPoint( cell_points->GetId( 2 ) );

    man->position( a[ 0 ], a[ 1 ], a[ 2 ] );
    man->normal( n[ 0 ], n[ 1 ], n[ 2 ] );
    man->textureCoord( 0, 0 );

    man->position( b[ 0 ], b[ 1 ], b[ 2 ] );
    man->normal( n[ 0 ], n[ 1 ], n[ 2 ] );
    man->textureCoord( 0, 0 );

    man->position( c[ 0 ], c[ 1 ], c[ 2 ] );
    man->normal( n[ 0 ], n[ 1 ], n[ 2 ] );
    man->textureCoord( 0, 0 );

    man->triangle( j, j + 1, j + 2 );
  } // end for
  man->end( );

  this->m_SceneMgr->getRootSceneNode( )->
    createChildSceneNode( )->attachObject( man );

  /* TODO
     Ogre::MeshSerializer serializer;
     serializer.exportMesh(
     man->convertToMesh( output_name ),
     output_name + std::string( ".mesh" )
     );
  */
}

// eof - $RCSfile$
