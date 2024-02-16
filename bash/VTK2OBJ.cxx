// =========================================================================
// @author Leonardo Florez-Valencia (florez-l@javeriana.edu.co)
// =========================================================================

#include <vtkPolyDataReader.h>
#include <vtkOBJWriter.h>
#include <vtkNew.h>

int main( int argc, char** argv )
{
  vtkNew< vtkPolyDataReader > r;
  r->SetFileName( argv[ 1 ] );

  vtkNew< vtkOBJWriter > w;
  w->SetFileName( argv[ 2 ] );
  w->SetInputConnection( r->GetOutputPort( ) );
  w->Update( );

  return( EXIT_SUCCESS );
}

// eof - $RCSfile$
