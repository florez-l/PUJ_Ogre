// =========================================================================
// @author Leonardo Florez-Valencia (florez-l@javeriana.edu.co)
// =========================================================================
#ifndef __PUJ_Ogre__BaseApplicationWithBullet__h__
#define __PUJ_Ogre__BaseApplicationWithBullet__h__

#include <PUJ_Ogre/BaseApplication.h>
#include <btBulletDynamicsCommon.h>

namespace PUJ_Ogre
{
  /**
   */
  class PUJ_Ogre_EXPORT BaseApplicationWithBullet
    : public PUJ_Ogre::BaseApplication
  {
  public:
    using Superclass  = PUJ_Ogre::BaseApplication;

  public:
    BaseApplicationWithBullet(
      const std::string& app_name,
      const std::string& resources = "resources.cfg"
      );
    virtual ~BaseApplicationWithBullet( ) override;

    // Configuration
    virtual void setup( ) override;

  protected:
    virtual void _connect_Ogre_Bullet( ) = 0;

    /* TODO
       virtual void locateResources( ) override;
       virtual void loadResources( ) override;

       // Iterative methods
       virtual bool keyPressed( const OgreBites::KeyboardEvent& evt ) override;

       // Main method
       virtual void go( );

       protected:
       virtual void _initSceneManager( );
       virtual void _configureCamera( const Ogre::AxisAlignedBox& bbox );
       virtual void _loadScene( ) = 0;

       virtual std::vector< Ogre::ManualObject* > _loadMeshFromUnconventionalFile(
       Ogre::AxisAlignedBox& bbox,
       const std::string& fname
       );
    */

  protected:
    btDefaultCollisionConfiguration* m_BulletCollisionConfiguration;
    btCollisionDispatcher* m_BulletDispatcher;
    btBroadphaseInterface* m_BulletOverlappingPairCache;
    btSequentialImpulseConstraintSolver* m_BulletSolver;
    btDiscreteDynamicsWorld* m_BulletDynamicsWorld;
    btAlignedObjectArray< btCollisionShape* > m_BulletCollisionShapes;

    /* TODO
       std::string           m_Resources;
       Ogre::SceneManager*   m_SceneMgr;
       OgreBites::CameraMan* m_CamMan;
    */
  };
} // end namespace

#endif // __PUJ_Ogre__BaseApplicationWithBullet__h__

// eof - $RCSfile$
