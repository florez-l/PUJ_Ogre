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
void PUJ_Ogre::OBJReader< _TReal, _TNatural >::
set_filename( const std::string& fname )
{
  this->m_FileName = fname;
}

// -------------------------------------------------------------------------
template< class _TReal, class _TNatural >
bool PUJ_Ogre::OBJReader< _TReal, _TNatural >::
read( )
{
  using TTokSep = boost::char_separator< char >;
  using TTok = boost::tokenizer< TTokSep >;

  this->m_Points.clear( );
  this->m_Normals.clear( );
  this->m_Textures.clear( );
  this->m_Objects.clear( );

  // Open input stream: load all file into a memory buffer
  std::ifstream ifs( this->m_FileName.c_str( ) );
  if( !ifs )
  {
    ifs.close( );
    return( false );
  } // end if
  ifs.seekg( 0, std::ios::end );
  std::size_t size = ifs.tellg( );
  ifs.seekg( 0, std::ios::beg );
  std::string buffer( size, 0 );
  ifs.read( &buffer[ 0 ], size );
  ifs.close( );
  std::istringstream input( buffer );

  // Read stream
  std::string current_object = this->m_FileName;
  std::string current_group = this->m_FileName;

  std::string line;
  while( std::getline( input, line ) )
  {
    // Tokenize line
    TTok toks { line, TTokSep{ " " } };
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
          TTok face { *t, TTokSep{ "/" } };
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
  return( true );
}

#endif // __PUJ_Ogre__OBJReader__hxx__

// eof - $RCSfile$
