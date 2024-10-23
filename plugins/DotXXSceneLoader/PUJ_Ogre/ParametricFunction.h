// =========================================================================
// @author Leonardo Florez-Valencia (florez-l@javeriana.edu.co)
// =========================================================================
#ifndef PUJ_Ogre_plugins_ParametricFunction__h__
#define PUJ_Ogre_plugins_ParametricFunction__h__

#include <PUJ_Ogre/Plugin_PUJ_Ogre_DotXXSceneLoader_Export.h>

#include <vtkParametricFunction.h>
 
namespace PUJ_Ogre
{
  // Forward declaration
  class ParametricFunction_Expression;

  /**
   */
  class Plugin_PUJ_Ogre_DotXXSceneLoader_EXPORT ParametricFunction
    : public vtkParametricFunction
  {
  public:
    vtkTypeMacro( ParametricFunction, vtkParametricFunction );

    vtkGetStringMacro( Parameters );
    vtkGetStringMacro( XExpression );
    vtkGetStringMacro( YExpression );
    vtkGetStringMacro( ZExpression );

    vtkSetStringMacro( Parameters );
    vtkSetStringMacro( XExpression );
    vtkSetStringMacro( YExpression );
    vtkSetStringMacro( ZExpression );

  public:
    virtual void PrintSelf( std::ostream& os, vtkIndent indent ) override;
 
    static ParametricFunction* New( );
    virtual int GetDimension( ) override;
    virtual void Evaluate(
      double uvw[ 3 ], double Pt[ 3 ], double Duvw[ 9 ]
      ) override;
    virtual double EvaluateScalar(
      double uvw[ 3 ], double Pt[ 3 ], double Duvw[ 9 ]
      ) override;

  protected:
    ParametricFunction( );
    virtual ~ParametricFunction( ) override;

  private:
    ParametricFunction( const ParametricFunction& ) = delete;
    void operator=( const ParametricFunction& ) = delete;

  protected:
    char* Parameters  = 0;
    char* XExpression = 0;
    char* YExpression = 0;
    char* ZExpression = 0;

    ParametricFunction_Expression* XExpr { nullptr };
    ParametricFunction_Expression* YExpr { nullptr };
    ParametricFunction_Expression* ZExpr { nullptr };
  };
} // end namespace

#endif // PUJ_Ogre_plugins_ParametricFunction__h__

// eof - $RCSfile$
