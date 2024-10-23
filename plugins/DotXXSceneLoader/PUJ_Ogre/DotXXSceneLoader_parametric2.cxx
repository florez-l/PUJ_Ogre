// =========================================================================
// @author Leonardo Florez-Valencia (florez-l@javeriana.edu.co)
// =========================================================================

#include <PUJ_Ogre/DotXXSceneLoader.h>
#include <PUJ_Ogre/ParametricFunction.h>
#include <vtkFloatArray.h>
#include <vtkParametricFunctionSource.h>
#include <vtkPointData.h>
#include <vtkPolyDataNormals.h>
#include <vtkNew.h>

#include <Ogre.h>
#include <pugixml.hpp>

// -------------------------------------------------------------------------
void PUJ_Ogre::DotXXSceneLoader::
_parametric2(
  pugi::xml_node& XMLNode,
  const Ogre::String& name,
  const Ogre::String& material
  )
{
  Ogre::String urange = Self::_attrib( XMLNode, "urange", "0 0 open" );
  Ogre::String vrange = Self::_attrib( XMLNode, "vrange", "0 0 open" );
  Ogre::String sampling = Self::_attrib( XMLNode, "sampling", "0 0" );
  Ogre::String x = Self::_attrib( XMLNode.child( "x" ), "function", "0" );
  Ogre::String y = Self::_attrib( XMLNode.child( "y" ), "function", "0" );
  Ogre::String z = Self::_attrib( XMLNode.child( "z" ), "function", "0" );

  std::stringstream params;
  params << urange << " " << vrange;

  unsigned int usamples, vsamples;
  std::istringstream( sampling ) >> usamples >> vsamples;

  // Create model
  vtkNew< PUJ_Ogre::ParametricFunction > model;
  model->SetParameters( params.str( ).c_str( ) );
  model->SetXExpression( x.c_str( ) );
  model->SetYExpression( y.c_str( ) );
  model->SetZExpression( z.c_str( ) );

  // A dummy evaluation to update ranges and openness
  double uvw[ 3 ] = { 0 };
  double Pt[ 3 ] = { 0 };
  double Duvw[ 9 ] = { 0 };
  model->Evaluate( uvw, Pt, Duvw );

  // Sample the model
  vtkNew< vtkParametricFunctionSource > src;
  src->SetParametricFunction( model );
  src->SetUResolution( usamples );
  src->SetVResolution( vsamples );
  src->SetWResolution( 1 );

  vtkNew< vtkPolyDataNormals > normalFilter;
  normalFilter->SetInputConnection( src->GetOutputPort( ) );
  normalFilter->ComputePointNormalsOn( );
  normalFilter->ComputeCellNormalsOff( );
  normalFilter->Update( );

  // Create manual object
  auto points = normalFilter->GetOutput( )->GetPoints( );
  auto normals
    =
    vtkArrayDownCast< vtkFloatArray >(
      normalFilter->GetOutput( )->GetPointData( )->GetNormals( )
      );
  auto polys = normalFilter->GetOutput( )->GetPolys( );

  Ogre::ManualObject* man
    =
    this->m_SceneMgr->createManualObject( name + "_entity" );
  man->begin( material, Ogre::RenderOperation::OT_TRIANGLE_LIST );
  man->estimateVertexCount( points->GetNumberOfPoints( ) );
  man->estimateIndexCount( polys->GetNumberOfCells( ) * 3 );

  for( unsigned long long i = 0; i < points->GetNumberOfPoints( ); ++i )
  {
    double* p = points->GetPoint( i );
    double* n = normals->GetTuple( i );

    /* TODO
       std::cout
       << i
       << " ---> "
       << p[ 0 ] << " " << p[ 1 ] << " " << p[ 2 ]
       << " : "
       << n[ 0 ] << " " << n[ 1 ] << " " << n[ 2 ]
       << std::endl;
    */

    man->position( p[ 0 ], p[ 1 ], p[ 2 ] );
    man->normal( n[ 0 ], n[ 1 ], n[ 2 ] );
    man->textureCoord( 0, 0 ); // TODO
  } // end for
  
  vtkNew< vtkIdList > ids;
  polys->InitTraversal( );
  for( unsigned long long i = 0; i < polys->GetNumberOfCells( ); ++i )
  {
    polys->GetNextCell( ids );
    if( ids->GetNumberOfIds( ) == 3 )
      man->triangle( ids->GetId( 0 ), ids->GetId( 1 ), ids->GetId( 2 ) );
    else if( ids->GetNumberOfIds( ) == 4 )
      man->quad(
        ids->GetId( 0 ), ids->GetId( 1 ), ids->GetId( 2 ), ids->GetId( 3 )
        );
  } // end for

  man->end( );
}

// eof - $RCSfile$
