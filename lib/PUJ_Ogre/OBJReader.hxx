// =========================================================================
// @author Leonardo Florez-Valencia (florez-l@javeriana.edu.co)
// =========================================================================
#ifndef __PUJ_Ogre__OBJReader__hxx__
#define __PUJ_Ogre__OBJReader__hxx__

#include <algorithm>
#include <fstream>
#include <iterator>
#include <sstream>

#include <boost/tokenizer.hpp>

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Polygon_mesh_processing/compute_normal.h>
#include <CGAL/Surface_mesh.h>

#include <OgreResourceGroupManager.h>

// -------------------------------------------------------------------------
template< class _TReal, class _TNatural >
PUJ_Ogre::OBJReader< _TReal, _TNatural >::
OBJReader( )
{
}

// -------------------------------------------------------------------------
template< class _TReal, class _TNatural >
const std::string& PUJ_Ogre::OBJReader< _TReal, _TNatural >::
filename( ) const
{
  return( this->m_FileName );
}

// -------------------------------------------------------------------------
template< class _TReal, class _TNatural >
bool PUJ_Ogre::OBJReader< _TReal, _TNatural >::
read( const std::string& fname, bool phong_shading )
{
  using TTokSep = boost::char_separator< char >;
  using TTok = boost::tokenizer< TTokSep >;

  this->m_FileName = fname;
  this->m_Points.clear( );
  this->m_Normals.clear( );
  this->m_Textures.clear( );
  this->m_Objects.clear( );

  // Open input stream: load all file into a memory buffer
  auto c = Ogre::ResourceGroupManager::getSingleton( )
    .openResource( this->m_FileName );
  std::istringstream input( c->getAsString( ) );

  // Read stream
  std::string current_object = this->m_FileName;
  std::string current_group = this->m_FileName;

  std::string line;
  while( std::getline( input, line ) )
  {
    // Tokenize line
    TTok toks { line, TTokSep{ " \t\n" } };
    auto t = toks.begin( );
    std::string cmd = *( t++ );

    if( cmd == "v" )
    {
      for( unsigned int i = 0; i < 3; ++i )
      {
        this->m_Points.push_back( 0 );
        if( t != toks.end( ) )
          std::istringstream( *( t++ ) ) >> this->m_Points.back( );
      } // end for
      this->m_Points.push_back( 1 );
      if( t != toks.end( ) )
        std::istringstream( *( t++ ) ) >> this->m_Points.back( );
    }
    else if( cmd == "vn" )
    {
      for( unsigned int i = 0; i < 3; ++i )
      {
        this->m_Normals.push_back( 0 );
        if( t != toks.end( ) )
          std::istringstream( *( t++ ) ) >> this->m_Normals.back( );
      } // end for
    }
    else if( cmd == "vt" )
    {
      for( unsigned int i = 0; i < 3; ++i )
      {
        this->m_Textures.push_back( 0 );
        if( t != toks.end( ) )
          std::istringstream( *( t++ ) ) >> this->m_Textures.back( );
      } // end for
    }
    else if( cmd == "o" )
    {
      if( t != toks.end( ) )
      {
        std::stringstream n;
        n << *( t++ );
        for( ; t != toks.end( ); ++t )
          n << "_" << *t;
        current_object = n.str( );
      } // end if
    }
    else if( cmd == "g" )
    {
      if( t != toks.end( ) )
      {
        std::stringstream n;
        n << *( t++ );
        for( ; t != toks.end( ); ++t )
          n << "_" << *t;
        current_group = n.str( );
      } // end if
    }
    else if( cmd == "f" )
    {
      auto o = this->m_Objects[ current_object ]
        .insert( std::make_pair( current_group, TTopo( ) ) ).first;
      o->second.push_back( TFace( ) );
      for( ; t != toks.end( ); ++t )
      {
        if( *t != "" )
        {
          auto c = std::count( t->begin( ), t->end( ), '/' );
          TTok face { *t, TTokSep{ "/ \t\n" } };
          auto f = face.begin( );
          auto d = std::distance( f, face.end( ) );

          TIndex i;
          std::istringstream( *( f++ ) ) >> i[ 0 ];
          if( c == 2 && d == 3 )
          {
            std::istringstream( *( f++ ) ) >> i[ 1 ];
            std::istringstream( *( f++ ) ) >> i[ 2 ];
          }
          else if( c == 1 && d == 2 )
            std::istringstream( *( f++ ) ) >> i[ 1 ];
          else if( c == 2 && d == 2 )
            std::istringstream( *( f++ ) ) >> i[ 2 ];
          o->second.back( ).push_back( i );
        } // end if
      } // end for
    } // end if
  } // end while

  this->_build_buffer( phong_shading );

  return( true );
}

// -------------------------------------------------------------------------
template< class _TReal, class _TNatural >
void PUJ_Ogre::OBJReader< _TReal, _TNatural >::
_build_buffer( bool phong_shading )
{
  using _TK = CGAL::Exact_predicates_inexact_constructions_kernel;
  using _TP = _TK::Point_3;
  using _TV = _TK::Vector_3;
  using _TM = CGAL::Surface_mesh< _TP >;
  using _TI = _TM::Vertex_index;
  using _TF = _TM::Face_index;
  // TODO using _TPM = boost::property_map< _TM, CGAL::vertex_point_t >::type;
  using _TNM = _TM::template Property_map< _TI, _TV >;

  /* TODO
     typedef CGAL::Simple_cartesian<double> K;
     typedef CGAL::Surface_mesh<K::Point_3> Mesh;
     typedef Mesh::Vertex_index vertex_descriptor;
     typedef Mesh::Face_index face_descriptor;
  */

  std::vector< _TP > points;
  for( unsigned long long i = 0; i < this->m_Points.size( ); i += 4 )
  {
    points.push_back(
      _TP(
        this->m_Points[ i ],
        this->m_Points[ i + 1 ],
        this->m_Points[ i + 2 ]
        )
      );
  } // end for
  points.shrink_to_fit( );

  // Create meshes
  std::vector< _TI > verts( points.size( ) ), face_range;
  for( const auto& o: this->m_Objects )
  {
    for( const auto& g: o.second )
    {
      _TM mesh;
      std::fill( verts.begin( ), verts.end( ), _TI( ) );
      for( const auto& f: g.second )
      {
        face_range.clear( );
        for( const auto& i: f )
        {
          auto idx = i[ 0 ] - 1;
          if( verts[ idx ] == _TI( ) )
            verts[ idx ] = mesh.add_vertex( points[ idx ] );
          face_range.push_back( verts[ idx ] );
        } // end for
        mesh.add_face( face_range );
      } // end for

      if( phong_shading )
      {
        _TNM normals = mesh.template add_property_map< _TI, _TV >(
          "v:normals", CGAL::NULL_VECTOR
          ).first;
        CGAL::Polygon_mesh_processing
          ::compute_vertex_normals( mesh, normals );

        /* TODO
           unsigned long long leo = 0;
           for( const auto& n: normals )
           leo++;
           std::cout << leo << " " << mesh.number_of_vertices( ) << std::endl;
        */

        // boost::graph_traits< _TM >::vertex_descriptor;
            
        /* TODO
           CGAL::Polygon_mesh_processing::compute_vertex_normals( mesh, normals );
        */

        /* TODO
           void CGAL::Polygon_mesh_processing::compute_vertex_normals       (       const PolygonMesh &     pmesh,
           VertexNormalMap  vertex_normals,
           const NamedParameters &  np = parameters::default_values() 
           )        
        */

        /* TODO
           using TIndex   = std::array< TNatural, 3 >;
           using TFace    = std::vector< TIndex >;
           using TTopo    = std::vector< TFace >;
           using TGroups  = std::map< std::string, TTopo >;
           using TObjects = std::map< std::string, TGroups >;
        */
      }
      else
      {
        /* TODO
           void CGAL::Polygon_mesh_processing::compute_face_normals (       const PolygonMesh &     pmesh,
           Face_normal_map  face_normals,
           const NamedParameters &  np = parameters::default_values() 
           )        
        */
      } // end if
    } // end for
  } // end for
}

#endif // __PUJ_Ogre__OBJReader__hxx__

// eof - $RCSfile$
