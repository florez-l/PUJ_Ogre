// =========================================================================
// @author Leonardo Florez-Valencia (florez-l@javeriana.edu.co)
// =========================================================================
#ifndef __PUJ_Ogre__OBJReader__h__
#define __PUJ_Ogre__OBJReader__h__

#include <array>
#include <map>
#include <string>
#include <tuple>
#include <vector>

namespace PUJ_Ogre
{
  /**
   */
  template< class _TReal = double, class _TNatural = unsigned int >
  class OBJReader
  {
  public:
    using TReal    = _TReal;
    using TNatural = _TNatural;
    using TIndex   = std::array< TNatural, 3 >;
    using TFace    = std::vector< TIndex >;
    using TTopo    = std::vector< TFace >;
    using TGroups  = std::map< std::string, TTopo >;
    using TObjects = std::map< std::string, TGroups >;
    using TGeom    = std::vector< TReal >;
    using TIndices = std::vector< TNatural >;

    using TBuffer = std::map< std::string, std::map< std::string, std::tuple< TGeom, TIndices, TIndices > > >;

  public:
    OBJReader( );
    virtual ~OBJReader( ) = default;

    const std::string& filename( ) const;
    const TBuffer& buffer( ) const;
    bool read( const std::string& fname, bool phong_shading = true );
    virtual bool read( std::istream& input, bool phong_shading = true );

  protected:
    virtual void _build_buffer( bool phong_shading );
    virtual void _build_buffer_without_normals( bool phong_shading );

  protected:
    std::string m_FileName { "" };
    TGeom m_Points;
    TGeom m_Normals;
    TGeom m_Textures;
    TObjects m_Objects;

    TBuffer m_Buffer;
  };
} // end namespace

#endif // __PUJ_Ogre__OBJReader__h__

// eof - $RCSfile$
