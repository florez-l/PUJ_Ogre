// =========================================================================
// @author Leonardo Florez-Valencia (florez-l@javeriana.edu.co)
// =========================================================================

#include <iostream>












#include <PUJ_Ogre/DotXXSceneLoader.h>
#include <PUJ_Ogre/exprtk.h>

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
  auto x = XMLNode.child( "x" );
  auto y = XMLNode.child( "y" );
  auto z = XMLNode.child( "z" );
  Ogre::String xf = Self::_attrib( x, "function", "0" );
  Ogre::String yf = Self::_attrib( y, "function", "0" );
  Ogre::String zf = Self::_attrib( z, "function", "0" );

  std::string Lu, Uu, Ou, Lv, Uv, Ov;
  unsigned long long Nu, Nv;
  std::istringstream( urange ) >> Lu >> Uu >> Ou;
  std::istringstream( vrange ) >> Lv >> Uv >> Ov;
  std::istringstream( sampling ) >> Nu >> Nv;

  typedef exprtk::symbol_table<float> symbol_table_t;
  typedef exprtk::expression<float>   expression_t;
  typedef exprtk::parser<float>       parser_t;

  float u, v;
  symbol_table_t symbol_table;
  symbol_table.add_variable( "u", u );
  symbol_table.add_variable( "v", v );
  symbol_table.add_constants( );

  expression_t xe, ye, ze, Lue, Uue, Lve, Uve;
  xe.register_symbol_table( symbol_table );
  ye.register_symbol_table( symbol_table );
  ze.register_symbol_table( symbol_table );
  Lue.register_symbol_table( symbol_table );
  Uue.register_symbol_table( symbol_table );
  Lve.register_symbol_table( symbol_table );
  Uve.register_symbol_table( symbol_table );

  parser_t xp, yp, zp, Lup, Uup, Lvp, Uvp;
  xp.compile( xf, xe );
  yp.compile( yf, ye );
  zp.compile( zf, ze );
  Lup.compile( Lu, Lue );
  Uup.compile( Uu, Uue );
  Lvp.compile( Lv, Lve );
  Uvp.compile( Uv, Uve );

  float Lu_ = Lue.value( );
  float Uu_ = Uue.value( );
  float Lv_ = Lve.value( );
  float Uv_ = Uve.value( );

  Ogre::ManualObject* man
    =
    this->m_SceneMgr->createManualObject( name + "_entity" );
  man->begin( material, Ogre::RenderOperation::OT_TRIANGLE_LIST );

  for( unsigned long long su = 0; su < Nu; ++su )
  {
    u = ( ( Uu_ - Lu_ ) * float( su ) / float( Nu ) ) + Lu_;
    for( unsigned long long sv = 0; sv < Nv; ++sv )
    {
      v = ( ( Uv_ - Lv_ ) * float( sv ) / float( Nv ) ) + Lv_;
      float px = xe.value( );
      float py = ye.value( );
      float pz = ze.value( );

      man->position( px, py, pz );
      man->normal( px, py, pz );
      man->textureCoord( u, v );
    } // end for
  } // end for

  unsigned long long idx = 0;
  for( unsigned long long su = 0; su < Nu; ++su )
  {
    for( unsigned long long sv = 0; sv < Nv; ++sv )
    {
      // TODO: man->triangle( i, j, k );
      idx++;
    } // end for
  } // end for

  man->end( );
}

// eof - $RCSfile$
