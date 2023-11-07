// -------------------------------------------------------------------------
// @author Leonardo Florez-Valencia (florez-l@javeriana.edu.co)
// -------------------------------------------------------------------------

#include <PUJ_Ogre/BaseApplication.h>

#include <filesystem>
#include <unistd.h>

#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkPLYReader.h>
#include <vtkXMLPolyDataReader.h>
#include <vtkOBJReader.h>
#include <vtkSTLReader.h>
#include <vtkPolyDataReader.h>
#include <vtkSmartPointer.h>

// -------------------------------------------------------------------------
template< class _R, class _C >
void PUJ_Ogre::BaseApplication::
_read_using_vtk( vtkPolyData** pdata, const _C& c )
{
  static char name_template[] = "tmp_PUJ_Ogre_XXXXXXXXX";
  char fname[ PATH_MAX ];

  std::strcpy( fname, name_template );
  int fd = mkstemp( fname );
  write( fd, c->getAsString( ).c_str( ), c->getAsString( ).size( ) );
  close( fd );

  vtkNew< _R > reader;
  reader->SetFileName( fname );
  reader->Update( );

  *pdata = vtkPolyData::New( );
  ( *pdata )->DeepCopy( reader->GetOutput( ) );

  unlink( fname );
}

// -------------------------------------------------------------------------
Ogre::ManualObject* PUJ_Ogre::BaseApplication::
_load_using_vtk( const std::string& fname, const std::string& material )
{
  std::string ext = std::filesystem::path( fname ).extension( );
  std::transform(
    ext.begin( ), ext.end( ), ext.begin( ),
    []( unsigned char c ) { return( std::tolower( c ) ); }
    );
  auto contents =
    Ogre::ResourceGroupManager::getSingleton( ).openResource( fname );

  vtkPolyData* pdata = nullptr;
  if( ext == ".ply" )
    this->_read_using_vtk< vtkPLYReader >( &pdata, contents );
  else if( ext == ".vtp" )
    this->_read_using_vtk< vtkXMLPolyDataReader >( &pdata, contents );
  else if( ext == ".obj" )
    this->_read_using_vtk< vtkOBJReader >( &pdata, contents );
  else if( ext == ".stl" )
    this->_read_using_vtk< vtkSTLReader >( &pdata, contents );
  else if( ext == ".vtk" )
    this->_read_using_vtk< vtkPolyDataReader >( &pdata, contents );
  /* TODO
     else if( ext == ".g" )
     this->_read_using_vtk< vtkBYUReader >( &pdata, contents );
     reader->SetGeometryFileName( fname )
  */
  if( pdata != nullptr )
  {
    Ogre::ManualObject* man = this->m_SceneMgr->createManualObject( fname );
    man->begin( material, Ogre::RenderOperation::OT_TRIANGLE_LIST );

    double p[ 3 ];
    auto normals = pdata->GetPointData( )->GetNormals( );
    auto tcoords = pdata->GetPointData( )->GetTCoords( );
    for( unsigned int i = 0; i < pdata->GetNumberOfPoints( ); ++i )
    {
      pdata->GetPoint( i, p );
      man->position( p[ 0 ], p[ 1 ], p[ 2 ] );
      if( normals != nullptr )
      {
        double* n = normals->GetTuple3( i );
        man->normal( n[ 0 ], n[ 1 ], n[ 2 ] );
      } // end if
      if( tcoords != nullptr )
      {
        double* t = tcoords->GetTuple2( i );
        man->textureCoord( t[ 0 ], t[ 1 ] );
      } // end if
    } // end for

    auto polys = pdata->GetPolys( );
    if( polys != nullptr )
    {
      vtkNew< vtkIdList > idx;
      for( unsigned int i = 0; i < polys->GetNumberOfCells( ); ++i )
      {
        polys->GetCellAtId( i, idx );
        if( idx->GetNumberOfIds( ) == 3 )
          man->triangle( idx->GetId( 0 ), idx->GetId( 1 ), idx->GetId( 2 ) );
        else if( idx->GetNumberOfIds( ) == 4 )
          man->quad(
            idx->GetId( 0 ), idx->GetId( 1 ), idx->GetId( 2 ), idx->GetId( 3 )
            );
      } // end for
    } // end if

    man->end( );

    return( man );
  }
  else
    return( nullptr );
}

// eof - $RCSfile$
