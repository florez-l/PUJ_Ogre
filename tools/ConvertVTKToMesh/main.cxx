// =========================================================================
// @author Leonardo Florez-Valencia (florez-l@javeriana.edu.co)
// =========================================================================

#include "Application.h"

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>

std::string ExePath( )
{
  char buffer[ MAX_PATH ];
  GetModuleFileName( NULL, buffer, MAX_PATH );
  std::string::size_type pos = std::string( buffer ).find_last_of( "\\/" );
  if ( pos == std::string::npos )
    return( "" );
  else
    return( std::string( buffer ).substr( 0, pos ) );
}

INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT )
#  else
int main( int argc, char** argv )
#endif
{
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
  Application app( ExePath( ) );
#else
  bool point_normals = false;
  if( argc >= 3 )
    point_normals = argv[ 2 ][ 0 ] == '1';
  Application app( argv[ 0 ], argv[ 1 ], point_normals );
#endif // OGRE_PLATFORM == OGRE_PLATFORM_WIN32
  app.go( );
  return( EXIT_SUCCESS );
}

// eof - $RCSfile$

