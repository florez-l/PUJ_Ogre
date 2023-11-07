// =========================================================================
// @author Leonardo Florez-Valencia (florez-l@javeriana.edu.co)
// =========================================================================
#ifndef __Application__h__
#define __Application__h__

#include <OgreApplicationContext.h>
#include <vtkSmartPointer.h>

// -- Some forward declarations
class vtkPolyData;
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
    const std::string& exec_fname,
    const std::string& input_fname,
    const std::string& output_name,
    bool normals_from_points
    );
  virtual ~Application( ) override;

  virtual void go( );

  virtual void loadResources( ) override;
  virtual void setup( ) override;
  virtual bool keyPressed( const OgreBites::KeyboardEvent& evt ) override;

protected:

  template< class _R >
  void _readInput( const std::string& input_fname );

  void _loadMesh( Ogre::SceneManager* mgr );

protected:
  std::string m_OutputMeshName;
  std::string m_ResourcesFile;
  std::string m_MaterialName;
  bool m_NormalsFromPoints;

  vtkSmartPointer< vtkPolyData > m_PolyData;

  OgreBites::CameraMan* m_CamMan;
};

#endif // __Application__h__

  // eof - $RCSfile$
