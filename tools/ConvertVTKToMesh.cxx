// =========================================================================
// @author Leonardo Florez-Valencia (florez-l@javeriana.edu.co)
// =========================================================================

#include <iostream>

#include <vtkPolyData.h>
#include <vtkPolyDataReader.h>
#include <vtkNew.h>

#include <OgreManualObject.h>

int main( int argc, char** argv )
{
  vtkNew< vtkPolyDataReader > reader;
  reader->SetFileName( argv[ 1 ] );
  reader->Update( );
  vtkPolyData* pdata = reader->GetOutput( );

  Ogre::ManualObject man( "name" );

  return( EXIT_SUCCESS );
}

// eof - $RCSfile$

