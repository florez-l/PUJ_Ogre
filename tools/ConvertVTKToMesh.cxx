// =========================================================================
// @author Leonardo Florez-Valencia (florez-l@javeriana.edu.co)
// =========================================================================

#include <iostream>
#include <memory>

#include <vtkCellData.h>
#include <vtkPolyData.h>
#include <vtkPolyDataNormals.h>
#include <vtkPolyDataReader.h>
#include <vtkTriangleFilter.h>
#include <vtkNew.h>

#include <Ogre.h>
#include <OgreManualObject.h>
#include <OgreMeshSerializer.h>

void SaveMeshWithPointNormals(
  vtkPolyData* pdata,
  const std::string& output_name
  )
{
}

void SaveMeshWithFaceNormals(
  vtkPolyData* pdata,
  const std::string& output_name
  )
{
  std::shared_ptr< Ogre::ManualObject > man =
    std::make_shared< Ogre::ManualObject >( output_name );
  man->begin( output_name, Ogre::RenderOperation::OT_TRIANGLE_LIST );

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

  Ogre::MeshSerializer serializer;
  serializer.exportMesh(
    man->convertToMesh( output_name ),
    output_name + std::string( ".mesh" )
    );
}

int main( int argc, char** argv )
{
  if( argc < 3 )
  {
    std::cerr
      << "Usage: " << argv[ 0 ]
      << " input output_name [normals_from_points:0/1]"
      << std::endl;
    return( EXIT_FAILURE );
  } // end if
  std::string input_fname = argv[ 1 ];
  std::string output_name = argv[ 2 ];
  bool normals_from_points = false;
  if( argc >= 4 )
    normals_from_points = ( argv[ 3 ][ 0 ] == '1' );

  vtkNew< vtkPolyDataReader > reader;
  reader->SetFileName( argv[ 1 ] );

  vtkNew< vtkTriangleFilter > triangles;
  triangles->SetInputConnection( reader->GetOutputPort( ) );

  vtkNew< vtkPolyDataNormals > normals;
  normals->SetInputConnection( triangles->GetOutputPort( ) );
  if( normals_from_points )
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
  vtkPolyData* pdata = normals->GetOutput( );

  Ogre::Root root( "", "", "" );
  Ogre::MaterialManager::getSingleton().initialise();
  Ogre::MeshSerializer meshSerializer;
  /* TODO
     Ogre::MeshResourceCreator resCreator;
     meshSerializer.setListener(&resCreator);
  */
  Ogre::SkeletonSerializer skeletonSerializer;
  Ogre::DefaultHardwareBufferManager bufferManager; // needed because we don't have a rendersystem
  // don't pad during upgrade
  Ogre::MeshManager::getSingleton().setBoundsPaddingFactor(0.0f);


  Ogre::ConfigFile cf = Ogre::ConfigFile( );
  cf.loadDirect( "resources.cfg" );
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
  } // end try

  try
  {
    if( normals_from_points )
      SaveMeshWithPointNormals( pdata, output_name );
    else
      SaveMeshWithFaceNormals( pdata, output_name );
  }
  catch( ... )
  {
  } // end try
  return( EXIT_SUCCESS );
}

// eof - $RCSfile$
