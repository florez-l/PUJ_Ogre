// =========================================================================
// @author Leonardo Florez-Valencia (florez-l@javeriana.edu.co)
// =========================================================================

#include "Application.h"

#include <filesystem>

#include <Ogre.h>
#include <OgreCameraMan.h>

#include <vtkCellData.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkPolyDataNormals.h>
#include <vtkTriangleFilter.h>

#include <vtkPolyDataReader.h>
#include <vtkSTLReader.h>


#include <vtkNew.h>

/* TODO
   #include <exception>
   #include <iostream>

   #include <Ogre.h>
   #include <OgreApplicationContext.h>
   #include <OgreInput.h>
   #include <OgreRTShaderSystem.h>

*/

/**
 */
// -------------------------------------------------------------------------
Application::
Application(
  const std::string& exec_fname,
  const std::string& input_fname,
  const std::string& output_name,
  bool normals_from_points
  )
  : TContext( "PUJ_Ogre_ConvertVTKToMesh" ),
    TListener( )
{
  this->m_OutputMeshName = output_name;
  this->m_NormalsFromPoints = normals_from_points;

  // Paths
  auto e = std::filesystem::canonical( std::filesystem::path( exec_fname ) );
  auto m = std::filesystem::canonical( std::filesystem::path( input_fname ) );

  // Resources file
  this->m_ResourcesFile = ( e.parent_path( ) / ".resources.cfg" ).string( );
  std::ofstream r( this->m_ResourcesFile.c_str( ) );
  r
    << "[General]" << std::endl
    << "FileSystem=" << m.parent_path( ).string( )
    << std::endl;
  r.close( );

  // Materials file
  this->m_MaterialName = m.stem( ).string( );
  auto mat_file = m.parent_path( ) / ".material.material";
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
  n << "         emissive 1 1 1 1" << std::endl;
  n << "      }" << std::endl;
  n << "   }" << std::endl;
  n << "}" << std::endl;
  n.close( );

  this->_readInput< vtkPolyDataReader >( input_fname );
  // this->_readInput< vtkSTLReader >( input_fname );

  // Load VTK model
  vtkNew< vtkPolyDataReader > reader;
  vtkNew< vtkTriangleFilter > triangles;
  vtkNew< vtkPolyDataNormals > normals;

  reader->SetFileName( input_fname.c_str( ) );
  triangles->SetInputConnection( reader->GetOutputPort( ) );
  normals->SetInputConnection( triangles->GetOutputPort( ) );
  if( this->m_NormalsFromPoints )
  {
    normals->ComputeCellNormalsOff( );
    normals->ComputePointNormalsOn( );
  }
  else
  {
    normals->ComputeCellNormalsOn( );
    normals->ComputePointNormalsOff( );
  } // end if
  normals->Update( );
  this->m_PolyData = vtkSmartPointer< vtkPolyData >::New( );
  this->m_PolyData->DeepCopy( normals->GetOutput( ) );
}

// -------------------------------------------------------------------------
Application::
~Application( )
{
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
setup( )
{
  // Basic objects
  this->TContext::setup( );
  this->addInputListener( this );
  auto root = this->getRoot( );
  auto scnMgr = root->createSceneManager( );

  // Shader
  auto shadergen = Ogre::RTShader::ShaderGenerator::getSingletonPtr( );
  shadergen->addSceneManager( scnMgr );

  // Mesh
  double b[ 6 ];
  this->m_PolyData->GetBounds( b );
  this->_loadMesh( scnMgr );
  Ogre::Vector3 minB( b[ 0 ], b[ 2 ], b[ 4 ] );
  Ogre::Vector3 maxB( b[ 1 ], b[ 3 ], b[ 5 ] );
  Ogre::Vector3 cntB = ( maxB + minB ) * 0.5;

  // Camera
  Ogre::SceneNode* camNode =
    scnMgr->getRootSceneNode( )->createChildSceneNode( );
  Ogre::Camera* cam = scnMgr->createCamera( "MainCam" );
  camNode->setPosition( maxB * 2.0 );
  camNode->lookAt( cntB, Ogre::Node::TransformSpace::TS_WORLD );
  cam->setNearClipDistance( 1e-2 );
  cam->setFarClipDistance( 1e+3 );
  camNode->attachObject( cam );
  Ogre::Viewport* vp = this->getRenderWindow( )->addViewport( cam );
  vp->setBackgroundColour( Ogre::ColourValue( 0.8, 0.8, 0.1 ) );
  cam->setAspectRatio(
    Ogre::Real( vp->getActualWidth( ) ) / Ogre::Real( vp->getActualHeight( ) )
    );
  this->m_CamMan = new OgreBites::CameraMan( camNode );
  this->addInputListener( this->m_CamMan );

  // Main light
  scnMgr->setAmbientLight( Ogre::ColourValue( 0.1, 0.2, 0.7 ) );
  scnMgr->setShadowTechnique(
    Ogre::ShadowTechnique::SHADOWTYPE_STENCIL_ADDITIVE
    );

  //! [spotlight]
  Ogre::Light* spotLight = scnMgr->createLight("SpotLight");
  spotLight->setDiffuseColour(0, 0, 1.0);
  spotLight->setSpecularColour(0, 0, 1.0);
  spotLight->setType(Ogre::Light::LT_SPOTLIGHT);
  Ogre::SceneNode* spotLightNode = scnMgr->getRootSceneNode()->createChildSceneNode();
  spotLightNode->attachObject(spotLight);
  spotLightNode->setDirection(-1, -1, 0);
  spotLightNode->setPosition( maxB );
  spotLight->setSpotlightRange(Ogre::Degree(35), Ogre::Degree(50));

  //! [directlight]
  Ogre::Light* directionalLight = scnMgr->createLight("DirectionalLight");
  directionalLight->setType(Ogre::Light::LT_DIRECTIONAL);
  directionalLight->setDiffuseColour(Ogre::ColourValue(0.4, 0, 0));
  directionalLight->setSpecularColour(Ogre::ColourValue(0.4, 0, 0));
  Ogre::SceneNode* directionalLightNode = scnMgr->getRootSceneNode()->createChildSceneNode();
  directionalLightNode->attachObject(directionalLight);
  directionalLightNode->setDirection(Ogre::Vector3(0, -1, 1));

  //! [pointlight]
  Ogre::Light* pointLight = scnMgr->createLight("PointLight");
  pointLight->setType(Ogre::Light::LT_POINT);
  pointLight->setDiffuseColour(0.3, 0.3, 0.3);
  pointLight->setSpecularColour(0.3, 0.3, 0.3);
  Ogre::SceneNode* pointLightNode = scnMgr->getRootSceneNode()->createChildSceneNode();
  pointLightNode->attachObject(pointLight);
  pointLightNode->setPosition( maxB );

  /* TODO

     //! [lightingsset]

     //! [ninja]
     Entity* ninjaEntity = scnMgr->createEntity("ninja.mesh");
     ninjaEntity->setCastShadows(true);
     scnMgr->getRootSceneNode()->createChildSceneNode()->attachObject(ninjaEntity);
     //! [ninja]

     //! [plane]

     //! [spotlight]
     Ogre::Light* spotLight = scnMgr->createLight("SpotLight");
     //! [spotlight]

     //! [spotlightcolor]
     spotLight->setDiffuseColour(0, 0, 1.0);
     spotLight->setSpecularColour(0, 0, 1.0);
     //! [spotlightcolor]

     //! [spotlighttype]
     spotLight->setType(Ogre::Light::LT_SPOTLIGHT);
     //! [spotlighttype]

     //! [spotlightposrot]
     Ogre::SceneNode* spotLightNode = scnMgr->getRootSceneNode()->createChildSceneNode();
     spotLightNode->attachObject(spotLight);
     spotLightNode->setDirection(-1, -1, 0);
     spotLightNode->setPosition(Ogre::Vector3(200, 200, 0));
     //! [spotlightposrot]

     //! [spotlightrange]
     spotLight->setSpotlightRange(Ogre::Degree(35), Ogre::Degree(50));
     //! [spotlightrange]

     //! [directlight]
     Ogre::Light* directionalLight = scnMgr->createLight("DirectionalLight");
     directionalLight->setType(Ogre::Light::LT_DIRECTIONAL);
     //! [directlight]

     //! [directlightcolor]
     directionalLight->setDiffuseColour(Ogre::ColourValue(0.4, 0, 0));
     directionalLight->setSpecularColour(Ogre::ColourValue(0.4, 0, 0));
     //! [directlightcolor]

     //! [directlightdir]
     Ogre::SceneNode* directionalLightNode = scnMgr->getRootSceneNode()->createChildSceneNode();
     directionalLightNode->attachObject(directionalLight);
     directionalLightNode->setDirection(Ogre::Vector3(0, -1, 1));
     //! [directlightdir]

     //! [pointlight]
     Ogre::Light* pointLight = scnMgr->createLight("PointLight");
     pointLight->setType(Ogre::Light::LT_POINT);
     //! [pointlight]

     //! [pointlightcolor]
     pointLight->setDiffuseColour(0.3, 0.3, 0.3);
     pointLight->setSpecularColour(0.3, 0.3, 0.3);
     //! [pointlightcolor]

     //! [pointlightpos]
     Ogre::SceneNode* pointLightNode = scnMgr->getRootSceneNode()->createChildSceneNode();
     pointLightNode->attachObject(pointLight);
     pointLightNode->setPosition(Ogre::Vector3(0, 150, 250));
  */
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
template< class _R >
void Application::
_readInput( const std::string& input_fname )
{
  // Load VTK model
  vtkNew< _R > reader;
  vtkNew< vtkTriangleFilter > triangles;
  vtkNew< vtkPolyDataNormals > normals;

  reader->SetFileName( input_fname.c_str( ) );
  triangles->SetInputConnection( reader->GetOutputPort( ) );
  normals->SetInputConnection( triangles->GetOutputPort( ) );
  if( this->m_NormalsFromPoints )
  {
    normals->ComputeCellNormalsOff( );
    normals->ComputePointNormalsOn( );
  }
  else
  {
    normals->ComputeCellNormalsOn( );
    normals->ComputePointNormalsOff( );
  } // end if
  normals->Update( );
  this->m_PolyData = vtkSmartPointer< vtkPolyData >::New( );
  this->m_PolyData->DeepCopy( normals->GetOutput( ) );
}

// -------------------------------------------------------------------------
void Application::
_loadMesh( Ogre::SceneManager* mgr )
{
  vtkPoints* points = this->m_PolyData->GetPoints( );
  vtkCellArray* polys = this->m_PolyData->GetPolys( );

  auto man = mgr->createManualObject( this->m_OutputMeshName );
  man->begin( this->m_MaterialName, Ogre::RenderOperation::OT_TRIANGLE_LIST );
  if( this->m_NormalsFromPoints )
  {
    vtkDataArray* normals = this->m_PolyData->GetPointData( )->GetNormals( );
    unsigned long long j = 0;
    vtkNew< vtkIdList > cell_points;
    for(
      unsigned long long i = 0; i < polys->GetNumberOfCells( ); ++i, j += 3
      )
    {
      cell_points->Initialize( );
      polys->GetCellAtId( i, cell_points );

      double* p = points->GetPoint( cell_points->GetId( 0 ) );
      double* n = normals->GetTuple( cell_points->GetId( 0 ) );
      man->position( p[ 0 ], p[ 1 ], p[ 2 ] );
      man->normal( n[ 0 ], n[ 1 ], n[ 2 ] );
      man->textureCoord( 1, 1 );

      p = points->GetPoint( cell_points->GetId( 1 ) );
      n = normals->GetTuple( cell_points->GetId( 1 ) );
      man->position( p[ 0 ], p[ 1 ], p[ 2 ] );
      man->normal( n[ 0 ], n[ 1 ], n[ 2 ] );
      man->textureCoord( 1, 1 );

      p = points->GetPoint( cell_points->GetId( 2 ) );
      n = normals->GetTuple( cell_points->GetId( 2 ) );
      man->position( p[ 0 ], p[ 1 ], p[ 2 ] );
      man->normal( n[ 0 ], n[ 1 ], n[ 2 ] );
      man->textureCoord( 1, 1 );

      man->triangle( j, j + 1, j + 2 );
    } // end for
  }
  else
  {
    vtkDataArray* normals = this->m_PolyData->GetCellData( )->GetNormals( );
    unsigned long long j = 0;
    vtkNew< vtkIdList > cell_points;
    for(
      unsigned long long i = 0; i < polys->GetNumberOfCells( ); ++i, j += 3
      )
    {
      cell_points->Initialize( );
      polys->GetCellAtId( i, cell_points );
      double* n = normals->GetTuple( i );

      double* p = points->GetPoint( cell_points->GetId( 0 ) );
      man->position( p[ 0 ], p[ 1 ], p[ 2 ] );
      man->normal( n[ 0 ], n[ 1 ], n[ 2 ] );
      man->textureCoord( 1, 1 );

      p = points->GetPoint( cell_points->GetId( 1 ) );
      man->position( p[ 0 ], p[ 1 ], p[ 2 ] );
      man->normal( n[ 0 ], n[ 1 ], n[ 2 ] );
      man->textureCoord( 1, 1 );

      p = points->GetPoint( cell_points->GetId( 2 ) );
      man->position( p[ 0 ], p[ 1 ], p[ 2 ] );
      man->normal( n[ 0 ], n[ 1 ], n[ 2 ] );
      man->textureCoord( 1, 1 );

      man->triangle( j, j + 1, j + 2 );
    } // end for
  } // end if
  man->end( );

  /* TODO
     man->convertToMesh( this->m_OutputMeshName );
     Ogre::Entity* ninjaEntity = mgr->createEntity( this->m_OutputMeshName );
     ninjaEntity->setCastShadows(true);
     mgr->getRootSceneNode()->createChildSceneNode()->attachObject(ninjaEntity);
  */
  mgr->getRootSceneNode( )->createChildSceneNode( )->attachObject( man );



  /* TODO

->convertToMesh( this->m_OutputMeshName ) );

     Ogre::MeshSerializer serializer;
     serializer.exportMesh(
     man->convertToMesh( output_name ),
     output_name + std::string( ".mesh" )
     );
  */



  /* TODO
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

     Ogre::MeshSerializer serializer;
     serializer.exportMesh(
     man->convertToMesh( output_name ),
     output_name + std::string( ".mesh" )
     );
  */
}

/* TODO

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

Ogre::MeshSerializer serializer;
serializer.exportMesh(
man->convertToMesh( output_name ),
output_name + std::string( ".mesh" )
);
}
*/

// eof - $RCSfile$
