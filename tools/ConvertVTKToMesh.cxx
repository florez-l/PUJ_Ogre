// =========================================================================
// @author Leonardo Florez-Valencia (florez-l@javeriana.edu.co)
// =========================================================================

#include <iostream>

#include <vtkCellData.h>
#include <vtkPolyData.h>
#include <vtkPolyDataNormals.h>
#include <vtkPolyDataReader.h>
#include <vtkTriangleFilter.h>
#include <vtkNew.h>

#include <OgreManualObject.h>

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
  vtkPoints* points = pdata->GetPoints( );
  vtkCellArray* polys = pdata->GetPolys( );
  vtkDataArray* normals = pdata->GetCellData( )->GetNormals( );

  vtkNew< vtkIdList > cell_points;
  for( unsigned long long i = 0; i < polys->GetNumberOfCells( ); ++i )
  {
    cell_points->Initialize( );
    polys->GetCellAtId( i, cell_points );
    double* normal = normals->GetTuple( i );

    std::cout
      << cell_points->GetId( 0 ) << " "
      << cell_points->GetId( 1 ) << " "
      << cell_points->GetId( 2 ) << std::endl;


  } // end for
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

  if( normals_from_points )
    SaveMeshWithPointNormals( pdata, output_name );
  else
    SaveMeshWithFaceNormals( pdata, output_name );
  return( EXIT_SUCCESS );
}

// eof - $RCSfile$
