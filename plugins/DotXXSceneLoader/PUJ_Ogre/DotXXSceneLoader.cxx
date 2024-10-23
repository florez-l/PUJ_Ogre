// =========================================================================
// @author Leonardo Florez-Valencia (florez-l@javeriana.edu.co)
// =========================================================================

#include <iostream>












#include <PUJ_Ogre/DotXXSceneLoader.h>

#include <Ogre.h>
#include <pugixml.hpp>

// -------------------------------------------------------------------------
PUJ_Ogre::DotXXSceneLoader::
DotXXSceneLoader( )
{
}

// -------------------------------------------------------------------------
PUJ_Ogre::DotXXSceneLoader::
~DotXXSceneLoader( )
{
}

// -------------------------------------------------------------------------
void PUJ_Ogre::DotXXSceneLoader::
load(
  Ogre::DataStreamPtr& stream,
  const Ogre::String& groupName,
  Ogre::SceneNode* rootNode
  )
{
  this->m_sGroupName = groupName;
  this->m_SceneMgr = rootNode->getCreator();

  pugi::xml_document XMLDoc;
  auto result
    =
    XMLDoc.load_buffer( stream->getAsString( ).c_str( ), stream->size( )
      );
  if( !result )
  {
    Ogre::LogManager::getSingleton( )
      .logError(
        "DotXXSceneLoader - " + Ogre::String( result.description( ) )
        );
    return;
  } // end if

  // Grab the scene node
  auto XMLRoot = XMLDoc.child( "scenexx" );

  // Validate the File
  if( !XMLRoot.attribute( "formatVersion" ) )
  {
    Ogre::LogManager::getSingleton( )
      .logError(
        "DotXXSceneLoader - Invalid .scene File. Missing <scene formatVersion='x.y' >"
        );
    return;
  } // end if

  // figure out where to attach any nodes we create
  this->m_AttachNode = rootNode;

  // Process the scene
  this->_process( XMLRoot );
}

// -------------------------------------------------------------------------
void PUJ_Ogre::DotXXSceneLoader::
exportScene(
  Ogre::SceneNode* rootNode,
  const Ogre::String& outFileName
  )
{
}

// -------------------------------------------------------------------------
const Ogre::ColourValue& PUJ_Ogre::DotXXSceneLoader::
getBackgroundColour( ) const
{
  return( this->m_BackgroundColour );
}

// -------------------------------------------------------------------------
void PUJ_Ogre::DotXXSceneLoader::
_process( pugi::xml_node& XMLRoot )
{
  // Process the scene parameters
  Ogre::String version = Self::_attrib( XMLRoot, "formatVersion", "unknown" );
  Ogre::String physics = Self::_attrib( XMLRoot, "physics", "off" );
  Ogre::String message = "[DotXXSceneLoader] Parsing dotScene++ file with version " + version + " (physics=" + physics + ").";
  if( XMLRoot.attribute("sceneManager" ) )
    message += ", scene manager " + Ogre::String( XMLRoot.attribute("sceneManager" ).value( ) );
  if( XMLRoot.attribute( "author" ) )
    message += ", author " + Ogre::String( XMLRoot.attribute( "author" ).value( ) );
  Ogre::LogManager::getSingleton( ).logMessage( message );

  // Process nodes
  if( auto e = XMLRoot.child( "nodes" ) )
    this->_nodes( e );

  // Process camera
  if( auto e = XMLRoot.child( "camera" ) )
    this->_camera( e );

  // Process colourAmbient
  if( auto e = XMLRoot.child( "ambientLight" ) )
    this->m_SceneMgr->setAmbientLight( Self::_colour( e, "colour" ) );
}

// -------------------------------------------------------------------------
void PUJ_Ogre::DotXXSceneLoader::
_nodes( pugi::xml_node& XMLNode )
{
  for( auto child: XMLNode.children( "node" ) )
  {
    Ogre::String name = Self::_attrib( child, "name");
    auto entity = child.child( "entity" );
    if( !entity )
    {
      Ogre::LogManager::getSingleton( )
        .logError(
          "DotXXSceneLoader - No entity defined for node \""
          +
          name
          +
          "\""
          );
    } // end if

    Ogre::String material = Self::_attrib( entity, "material", "default" );
    if( auto parametric2 = entity.child( "parametric2" ) )
      this->_parametric2( parametric2, name, material );
  } // end for
}

// -------------------------------------------------------------------------
void PUJ_Ogre::DotXXSceneLoader::
_camera( pugi::xml_node& XMLNode, Ogre::SceneNode* parent )
{
  Ogre::String name = Self::_attrib( XMLNode, "name" );
  Ogre::Real fov = Self::_real( XMLNode, "fov", 45 );
  Ogre::Real ar = Self::_real( XMLNode, "aspectRatio", 1.3333 );
  Ogre::String proj
    =
    Self::_attrib( XMLNode, "projectionType", "perspective" );

  // Create the camera
  Ogre::Camera* cam = this->m_SceneMgr->createCamera( name );

  // construct a scenenode is no parent
  if( parent == 0 )
    parent = this->m_AttachNode->createChildSceneNode( name );
  parent->attachObject( cam );

  // Set parameters
  cam->setFOVy( Ogre::Degree( fov ) );
  cam->setAspectRatio( ar );
  if( proj == "perspective" )
    cam->setProjectionType( Ogre::PT_PERSPECTIVE );
  else if( proj == "orthographic" )
    cam->setProjectionType( Ogre::PT_ORTHOGRAPHIC );

  // Process clipping
  if( auto e = XMLNode.child( "clipping" ) )
  {
    cam->setNearClipDistance( Self::_real( e, "near" ) );
    cam->setFarClipDistance( Self::_real( e, "far" ) );
  } // end if

  // Create camera node
  Ogre::Vector3 pos( 0, 0, 0 ), lookAt( 0, 0, 0 ), up( 0, 1, 0 );
  if( auto e = XMLNode.child( "position" ) )
    pos = Self::_vector( e, "point", pos );
  if( auto e = XMLNode.child( "lookAt" ) )
    lookAt = Self::_vector( e, "point", lookAt );
  if( auto e = XMLNode.child( "up" ) )
    up = Self::_vector( e, "point", up );

  lookAt.normalise( );
  up.normalise( );
  lookAt = -lookAt;
  parent->setPosition( pos );
  parent->setInitialState( );
  parent->setOrientation(
    Ogre::Quaternion( lookAt.crossProduct( up ), up, lookAt )
    );
}

// -------------------------------------------------------------------------
Ogre::String PUJ_Ogre::DotXXSceneLoader::
_attrib(
  const pugi::xml_node& XMLNode, const Ogre::String& attrib,
  const Ogre::String& defaultValue
  )
{
  if( auto anode = XMLNode.attribute( attrib.c_str( ) ) )
    return( anode.value( ) );
  else
    return( defaultValue );
}

// -------------------------------------------------------------------------
Ogre::Real PUJ_Ogre::DotXXSceneLoader::
_real(
  const pugi::xml_node& XMLNode, const Ogre::String& attrib,
  Ogre::Real defaultValue
  )
{
  if( auto anode = XMLNode.attribute( attrib.c_str( ) ) )
    return( Ogre::StringConverter::parseReal( anode.value( ) ) );
  else
    return( defaultValue );
}

// -------------------------------------------------------------------------
bool PUJ_Ogre::DotXXSceneLoader::
_bool(
  const pugi::xml_node& XMLNode, const Ogre::String& attrib,
  bool defaultValue
  )
{
  if( auto anode = XMLNode.attribute( attrib.c_str( ) ) )
    return( anode.as_bool( ) );
  else
    return( defaultValue );
}

// -------------------------------------------------------------------------
Ogre::ColourValue PUJ_Ogre::DotXXSceneLoader::
_colour(
  const pugi::xml_node& XMLNode, const Ogre::String& attrib,
  const Ogre::ColourValue& defaultValue
  )
{
  if( auto anode = XMLNode.attribute( attrib.c_str( ) ) )
  {
    float r, g, b, a;
    std::istringstream( anode.value( ) ) >> r >> g >> b >> a;
    return( Ogre::ColourValue( r, g, b, a ) );
  }
  else
    return( defaultValue );
}

// -------------------------------------------------------------------------
Ogre::Vector3 PUJ_Ogre::DotXXSceneLoader::
_vector(
  const pugi::xml_node& XMLNode, const Ogre::String& attrib,
  const Ogre::Vector3& defaultValue
  )
{
  if( auto anode = XMLNode.attribute( attrib.c_str( ) ) )
  {
    float x, y, z;
    std::istringstream( anode.value( ) ) >> x >> y >> z;
    return( Ogre::Vector3( x, y, z ) );
  }
  else
    return( defaultValue );
}


/* TODO
   protected:
   void writeNode( pugi::xml_node& parentXML, const SceneNode* node );
   void processScene( pugi::xml_node& XMLRoot );

       void processNodes( pugi::xml_node& XMLNode );
       void processExternals( pugi::xml_node& XMLNode );
       void processEnvironment( pugi::xml_node& XMLNode );
       void processTerrainGroup( pugi::xml_node& XMLNode );
       void processBlendmaps( pugi::xml_node& XMLNode );
       void processUserData( pugi::xml_node& XMLNode, Ogre::UserObjectBindings& userData );
       void processLight( pugi::xml_node& XMLNode, Ogre::SceneNode* parent = 0 );
       void processCamera( pugi::xml_node& XMLNode, Ogre::SceneNode* parent = 0 );

       void processNode( pugi::xml_node& XMLNode, Ogre::SceneNode* parent = 0 );
       void processLookTarget( pugi::xml_node& XMLNode, Ogre::SceneNode* parent );
       void processTrackTarget( pugi::xml_node& XMLNode, Ogre::SceneNode* parent );
       void processEntity( pugi::xml_node& XMLNode, Ogre::SceneNode* parent );
       void processParticleSystem( pugi::xml_node& XMLNode, Ogre::SceneNode* parent );
       void processBillboardSet( pugi::xml_node& XMLNode, Ogre::SceneNode* parent );
       void processPlane( pugi::xml_node& XMLNode, Ogre::SceneNode* parent );
       void processNodeAnimations( pugi::xml_node& XMLNode, Ogre::SceneNode* parent );
       void processNodeAnimation( pugi::xml_node& XMLNode, Ogre::SceneNode* parent );
       void processKeyframe( pugi::xml_node& XMLNode, Ogre::NodeAnimationTrack* pTrack );

       void processFog( pugi::xml_node& XMLNode );
       void processSkyBox( pugi::xml_node& XMLNode );
       void processSkyDome( pugi::xml_node& XMLNode );
       void processSkyPlane( pugi::xml_node& XMLNode );

       void processLightRange( pugi::xml_node& XMLNode, Ogre::Light* pLight );
       void processLightAttenuation( pugi::xml_node& XMLNode, Ogre::Light* pLight );
       void processLightSourceSize( pugi::xml_node& XMLNode, Ogre::Light* pLight );
  protected:
    Ogre::SceneManager* this->m_SceneMgr;
    Ogre::SceneNode* this->m_AttachNode;
    Ogre::String m_sGroupName;
    Ogre::ColourValue mBackgroundColour;
  };
*/

// -------------------------------------------------------------------------
const Ogre::String& PUJ_Ogre::DotXXScenePlugin::
getName( ) const
{
  static Ogre::String name = "DotXXScene Loader";
  return( name );
}

// -------------------------------------------------------------------------
void PUJ_Ogre::DotXXScenePlugin::
install( )
{
}

namespace PUJ_Ogre
{
  /**
   */
  struct DotXXSceneCodec
    : public Ogre::Codec
  {
    Ogre::String magicNumberToFileExt(
      const char* magicNumberPtr, size_t maxbytes
      ) const override
      {
        return( "" );
      }

    Ogre::String getType( ) const override
      {
        return( "scenexx" );
      }

    void decode(
      const Ogre::DataStreamPtr& stream,
      const Ogre::Any& output
      ) const override
      {
        Ogre::DataStreamPtr _stream( stream );
        PUJ_Ogre::DotXXSceneLoader loader;
        loader.load(
          _stream,
          Ogre::ResourceGroupManager::getSingleton( )
          .getWorldResourceGroupName( ),
          Ogre::any_cast< Ogre::SceneNode* >( output )
          );
      }

    void encodeToFile(
      const Ogre::Any& input,
      const Ogre::String& outFileName
      ) const override
      {
        PUJ_Ogre::DotXXSceneLoader loader;
        loader.exportScene(
          Ogre::any_cast< Ogre::SceneNode* >( input ), outFileName
          );
      }
  };
} // end namespace

// -------------------------------------------------------------------------
void PUJ_Ogre::DotXXScenePlugin::
initialise( )
{
  this->m_Codec = new PUJ_Ogre::DotXXSceneCodec( );
  Ogre::Codec::registerCodec( this->m_Codec );
}

// -------------------------------------------------------------------------
void PUJ_Ogre::DotXXScenePlugin::
shutdown( )
{
  Ogre::Codec::unregisterCodec( this->m_Codec );
  delete this->m_Codec;
}

// -------------------------------------------------------------------------
void PUJ_Ogre::DotXXScenePlugin::
uninstall( )
{
}

#ifndef OGRE_STATIC_LIB
extern "C" Plugin_PUJ_Ogre_DotXXSceneLoader_EXPORT void dllStartPlugin( );
extern "C" Plugin_PUJ_Ogre_DotXXSceneLoader_EXPORT void dllStopPlugin( );

static PUJ_Ogre::DotXXScenePlugin my_dotxxloader_plugin;

extern "C" Plugin_PUJ_Ogre_DotXXSceneLoader_EXPORT void dllStartPlugin( )
{
  Ogre::Root::getSingleton( ).installPlugin( &my_dotxxloader_plugin );
}
extern "C" Plugin_PUJ_Ogre_DotXXSceneLoader_EXPORT void dllStopPlugin( )
{
  Ogre::Root::getSingleton( ).uninstallPlugin( &my_dotxxloader_plugin );
}
#endif

// eof - $RCSfile$
