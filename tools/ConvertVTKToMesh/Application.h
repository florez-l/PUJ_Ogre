// =========================================================================
// @author Leonardo Florez-Valencia (florez-l@javeriana.edu.co)
// =========================================================================
#ifndef __Application__h__
#define __Application__h__

#include <OgreApplicationContext.h>

// -- Some forward declarations
class vtkPolyData;
namespace Ogre      { class Entity; }
namespace OgreBites { class CameraMan; }

/**
 */
class Application
  : public OgreBites::ApplicationContext,
    public OgreBites::InputListener
{
public:
  using TContext  = OgreBites::ApplicationContext;
  using TListener = OgreBites::InputListener;

public:
  Application(
    const std::string& exec_path, const std::string& model_fname,
    bool point_normals
    );
  virtual ~Application( ) override = default;

  // Configuration
  virtual void loadResources( ) override;
  virtual void setup( ) override;

  // Iterative methods
  virtual bool keyPressed( const OgreBites::KeyboardEvent& evt ) override;

  // Main method
  virtual void go( );

protected:
  virtual void _initSceneManager( );
  virtual void _loadScene( );

  void _meshWithPointNormals( vtkPolyData* pdata );
  void _meshWithFaceNormals( vtkPolyData* pdata );

protected:
  std::string m_ResourcesFile;
  std::string m_ModelFileName;
  bool m_PointNormals;

  Ogre::SceneManager*   m_SceneMgr;
  OgreBites::CameraMan* m_CamMan;
};

#endif // __Application__h__

// eof - $RCSfile$
