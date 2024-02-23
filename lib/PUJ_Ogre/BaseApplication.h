// =========================================================================
// @author Leonardo Florez-Valencia (florez-l@javeriana.edu.co)
// =========================================================================
#ifndef __PUJ_Ogre__BaseApplication__h__
#define __PUJ_Ogre__BaseApplication__h__

#include <PUJ_Ogre/Export.h>
#include <string>
#include <OgreApplicationContext.h>

namespace OgreBites { class CameraMan; }

namespace PUJ_Ogre
{
  /**
   */
  class PUJ_Ogre_EXPORT BaseApplication
    : public OgreBites::ApplicationContext,
      public OgreBites::InputListener
  {
  public:
    using TContext  = OgreBites::ApplicationContext;
    using TListener = OgreBites::InputListener;

  public:
    BaseApplication(
      const std::string& app_name,
      const std::string& resources = "resources.cfg",
      bool real_resources_file = true
      );
    virtual ~BaseApplication( ) override = default;

    // Configuration
    virtual void locateResources( ) override;
    virtual void loadResources( ) override;
    virtual void setup( ) override;

    // Iterative methods
    virtual bool keyPressed( const OgreBites::KeyboardEvent& evt ) override;

    // Main method
    virtual void go( );

  protected:
    virtual void _initSceneManager( );
    virtual void _configureCamera( const Ogre::AxisAlignedBox& bbox );
    virtual void _loadScene( ) = 0;
    
    virtual std::vector< Ogre::ManualObject* > _loadOBJ(
      Ogre::AxisAlignedBox& bbox,
      const std::string& fname
      );

  protected:
    std::string           m_Resources;
    bool                  m_RealResourcesFile;
    Ogre::SceneManager*   m_SceneMgr;
    OgreBites::CameraMan* m_CamMan;
  };
} // end namespace

#endif // __PUJ_Ogre__BaseApplication__h__

// eof - $RCSfile$
