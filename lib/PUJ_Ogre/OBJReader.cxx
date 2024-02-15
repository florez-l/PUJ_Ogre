// =========================================================================
// @author Leonardo Florez-Valencia (florez-l@javeriana.edu.co)
// =========================================================================

#include <PUJ_Ogre/OBJReader.h>

#include <istream>
#include <sstream>

#include <boost/tokenizer.hpp>

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Polygon_mesh_processing/compute_normal.h>
#include <CGAL/Surface_mesh.h>

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
const typename PUJ_Ogre::OBJReader< _TReal, _TNatural >::
TBuffer& PUJ_Ogre::OBJReader< _TReal, _TNatural >::
buffer( ) const
{
  return( this->m_Buffer );
}

// -------------------------------------------------------------------------
template< class _TReal, class _TNatural >
bool PUJ_Ogre::OBJReader< _TReal, _TNatural >::
read( const std::string& fname, bool phong_shading )
{
  this->m_FileName = fname;

  // Open input stream: load all file into a memory buffer
  /* TODO
   */
  // return( this->read( input, phong_shading ) );
  return( true );
}

// -------------------------------------------------------------------------
template< class _TReal, class _TNatural >
bool PUJ_Ogre::OBJReader< _TReal, _TNatural >::
read( std::istream& input, bool phong_shading )
{
  using TTokSep = boost::char_separator< char >;
  using TTok = boost::tokenizer< TTokSep >;

  this->m_FileName = "..no_filename..";
  this->m_Points.clear( );
  this->m_Normals.clear( );
  this->m_Textures.clear( );
  this->m_Objects.clear( );

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
        if( *t != "" && *t != "\r" )
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
  if( this->m_Normals.size( ) > 0 )
  {
    this->m_Buffer.clear( );

    for( const auto& o: this->m_Objects )
    {
      auto ob = this->m_Buffer.insert(
        std::make_pair(
          o.first,
          std::map< std::string, std::tuple< TGeom, TIndices, TIndices > >( )
          )
        ).first;
      for( const auto& g: o.second )
      {
        auto og = ob->second.insert(
          std::make_pair(
            g.first,
            std::make_tuple( TGeom( ), TIndices( ), TIndices( ) )
            )
          ).first;
        TNatural npnts = 0;
        for( const auto& f: g.second )
        {
          for( const auto& i: f )
          {
            auto p = ( i[ 0 ] - 1 ) * 4;
            auto n = ( i[ 2 ] - 1 ) * 3;

            std::get< 0 >( og->second ).push_back( this->m_Points[ p ] );
            std::get< 0 >( og->second ).push_back( this->m_Points[ p + 1 ] );
            std::get< 0 >( og->second ).push_back( this->m_Points[ p + 2 ] );
            std::get< 0 >( og->second ).push_back( this->m_Normals[ n ] );
            std::get< 0 >( og->second ).push_back( this->m_Normals[ n + 1 ] );
            std::get< 0 >( og->second ).push_back( this->m_Normals[ n + 2 ] );
            std::get< 0 >( og->second ).push_back( TReal( 0 ) );
            std::get< 0 >( og->second ).push_back( TReal( 0 ) );

            if( f.size( ) == 3 )
              std::get< 1 >( og->second ).push_back( npnts++ );
            else if( f.size( ) == 4 )
              std::get< 2 >( og->second ).push_back( npnts++ );
          } // end for
        } // end for
      } // end for
    } // end for

    /* TODO
       if( phong_shading )
       {
       _TNM normals = mesh.template add_property_map< _TI, _TV >(
       "v:normals", CGAL::NULL_VECTOR
       ).first;
       CGAL::Polygon_mesh_processing
       ::compute_vertex_normals( mesh, normals );

       auto& points = mesh.points( );
       auto p = points.begin( );
       auto n = normals.begin( );
       for( ; p != points.end( ); ++p, ++n )
       {
       std::get< 0 >( og->second ).push_back( TReal( ( *p )[ 0 ] ) );
       std::get< 0 >( og->second ).push_back( TReal( ( *p )[ 1 ] ) );
       std::get< 0 >( og->second ).push_back( TReal( ( *p )[ 2 ] ) );
       std::get< 0 >( og->second ).push_back( TReal( ( *n )[ 0 ] ) );
       std::get< 0 >( og->second ).push_back( TReal( ( *n )[ 1 ] ) );
       std::get< 0 >( og->second ).push_back( TReal( ( *n )[ 2 ] ) );
       std::get< 0 >( og->second ).push_back( TReal( 0 ) );
       std::get< 0 >( og->second ).push_back( TReal( 0 ) );
       } // end for

       CGAL::Vertex_around_face_iterator< _TM > vb, ve;
       auto faces = mesh.faces( );
       for( const auto& f: mesh.faces( ) )
       {
       boost::tie( vb, ve ) =
       CGAL::vertices_around_face( mesh.halfedge( f ), mesh );
       unsigned int s = std::distance( vb, ve );
       for( ; vb != ve; ++vb )
       {
       if( s == 3 )
       std::get< 1 >( og->second ).push_back( TNatural( *vb ) );
       else if( s == 4 )
       std::get< 2 >( og->second ).push_back( TNatural( *vb ) );
       } // end for
       } // end for

       std::get< 0 >( og->second ).shrink_to_fit( );
       std::get< 1 >( og->second ).shrink_to_fit( );
       std::get< 2 >( og->second ).shrink_to_fit( );
       }
    */
  }
  else
    this->_build_buffer_without_normals( phong_shading );
}

// -------------------------------------------------------------------------
template< class _TReal, class _TNatural >
void PUJ_Ogre::OBJReader< _TReal, _TNatural >::
_build_buffer_without_normals( bool phong_shading )
{
  using _TK = CGAL::Exact_predicates_inexact_constructions_kernel;
  using _TP = _TK::Point_3;
  using _TV = _TK::Vector_3;
  using _TM = CGAL::Surface_mesh< _TP >;
  using _TI = _TM::Vertex_index;
  using _TF = _TM::Face_index;
  using _TNM = _TM::template Property_map< _TI, _TV >;

  this->m_Buffer.clear( );

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
    auto ob = this->m_Buffer.insert(
      std::make_pair(
        o.first,
        std::map< std::string, std::tuple< TGeom, TIndices, TIndices > >( )
        )
      ).first;
    for( const auto& g: o.second )
    {
      auto og = ob->second.insert(
        std::make_pair(
          g.first,
          std::make_tuple( TGeom( ), TIndices( ), TIndices( ) )
          )
        ).first;

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

        auto& points = mesh.points( );
        auto p = points.begin( );
        auto n = normals.begin( );
        for( ; p != points.end( ); ++p, ++n )
        {
          std::get< 0 >( og->second ).push_back( TReal( ( *p )[ 0 ] ) );
          std::get< 0 >( og->second ).push_back( TReal( ( *p )[ 1 ] ) );
          std::get< 0 >( og->second ).push_back( TReal( ( *p )[ 2 ] ) );
          std::get< 0 >( og->second ).push_back( TReal( ( *n )[ 0 ] ) );
          std::get< 0 >( og->second ).push_back( TReal( ( *n )[ 1 ] ) );
          std::get< 0 >( og->second ).push_back( TReal( ( *n )[ 2 ] ) );
          std::get< 0 >( og->second ).push_back( TReal( 0 ) );
          std::get< 0 >( og->second ).push_back( TReal( 0 ) );
        } // end for

        CGAL::Vertex_around_face_iterator< _TM > vb, ve;
        auto faces = mesh.faces( );
        for( const auto& f: mesh.faces( ) )
        {
          boost::tie( vb, ve ) =
            CGAL::vertices_around_face( mesh.halfedge( f ), mesh );
          unsigned int s = std::distance( vb, ve );
          for( ; vb != ve; ++vb )
          {
            if( s == 3 )
              std::get< 1 >( og->second ).push_back( TNatural( *vb ) );
            else if( s == 4 )
              std::get< 2 >( og->second ).push_back( TNatural( *vb ) );
          } // end for
        } // end for

        std::get< 0 >( og->second ).shrink_to_fit( );
        std::get< 1 >( og->second ).shrink_to_fit( );
        std::get< 2 >( og->second ).shrink_to_fit( );
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

// -------------------------------------------------------------------------
#include <PUJ_Ogre/Export.h>

namespace PUJ_Ogre
{
  template class PUJ_Ogre_EXPORT OBJReader< float, unsigned char >;
  template class PUJ_Ogre_EXPORT OBJReader< float, unsigned short >;
  template class PUJ_Ogre_EXPORT OBJReader< float, unsigned int >;
  template class PUJ_Ogre_EXPORT OBJReader< float, unsigned long >;
  template class PUJ_Ogre_EXPORT OBJReader< float, unsigned long long >;

  template class PUJ_Ogre_EXPORT OBJReader< double, unsigned char >;
  template class PUJ_Ogre_EXPORT OBJReader< double, unsigned short >;
  template class PUJ_Ogre_EXPORT OBJReader< double, unsigned int >;
  template class PUJ_Ogre_EXPORT OBJReader< double, unsigned long >;
  template class PUJ_Ogre_EXPORT OBJReader< double, unsigned long long >;

  template class PUJ_Ogre_EXPORT OBJReader< long double, unsigned char >;
  template class PUJ_Ogre_EXPORT OBJReader< long double, unsigned short >;
  template class PUJ_Ogre_EXPORT OBJReader< long double, unsigned int >;
  template class PUJ_Ogre_EXPORT OBJReader< long double, unsigned long >;
  template class PUJ_Ogre_EXPORT OBJReader< long double, unsigned long long >;
} // end namespace

// eof - $RCSfile$
