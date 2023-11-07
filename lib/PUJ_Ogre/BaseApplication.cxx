// -------------------------------------------------------------------------
// @author Leonardo Florez-Valencia (florez-l@javeriana.edu.co)
// -------------------------------------------------------------------------

#include <PUJ_Ogre/BaseApplication.h>

#include <Ogre.h>
#include <PUJ_Ogre/CameraMan.h>

// -------------------------------------------------------------------------
PUJ_Ogre::BaseApplication::
BaseApplication( const std::string& title, const std::string& res_file )
  : OgreBites::ApplicationContext( title ),
    OgreBites::InputListener( )
{
  this->m_ResourcesFile = res_file;
}

// -------------------------------------------------------------------------
PUJ_Ogre::BaseApplication::
~BaseApplication( )
{
  if( this->m_CamMan != nullptr )
    delete this->m_CamMan;
}

// -------------------------------------------------------------------------
void PUJ_Ogre::BaseApplication::
locateResources( )
{
}

// -------------------------------------------------------------------------
void PUJ_Ogre::BaseApplication::
loadResources( )
{
  this->enableShaderCache( );
  Ogre::ConfigFile cf = Ogre::ConfigFile( );
  cf.loadDirect( this->m_ResourcesFile );

  auto res_mgr = Ogre::ResourceGroupManager::getSingletonPtr( );
  auto settings = cf.getSettingsBySection( );
  for( auto sIt = settings.begin( ); sIt != settings.end( ); ++sIt )
    for( auto fIt = sIt->second.begin( ); fIt != sIt->second.end( ); ++fIt )
      res_mgr->addResourceLocation( fIt->second, fIt->first, sIt->first );
  try
  {
    res_mgr->initialiseAllResourceGroups( );
    res_mgr->loadResourceGroup( "General" );
  }
  catch( ... )
  {
    // Do nothing
  }
}

// -------------------------------------------------------------------------
void PUJ_Ogre::BaseApplication::
setup( )
{
  this->_initSceneManager( );
  this->setWindowGrab( true );
  this->_loadScene( );
}

// -------------------------------------------------------------------------
bool PUJ_Ogre::BaseApplication::
keyPressed( const OgreBites::KeyboardEvent& evt )
{
  if( evt.keysym.sym == OgreBites::SDLK_ESCAPE )
    this->getRoot( )->queueEndRendering( );
  return( true );
}

// -------------------------------------------------------------------------
void PUJ_Ogre::BaseApplication::
go( )
{
  this->initApp( );
  this->getRoot( )->startRendering( );
  this->closeApp( );
}

// -------------------------------------------------------------------------
void PUJ_Ogre::BaseApplication::
_initSceneManager( )
{
  this->OgreBites::ApplicationContext::setup( );
  this->addInputListener( this );

  auto* root = this->getRoot( );
  this->m_SceneMgr = root->createSceneManager( "DefaultSceneManager" );

  auto* shadergen = Ogre::RTShader::ShaderGenerator::getSingletonPtr( );
  shadergen->addSceneManager( this->m_SceneMgr );
}

// eof - $RCSfile$
