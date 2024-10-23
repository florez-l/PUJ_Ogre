// =========================================================================
// @author Leonardo Florez-Valencia (florez-l@javeriana.edu.co)
// =========================================================================

#include <PUJ_Ogre/ParametricFunction.h>
#include <sstream>
#include <PUJ_Ogre/exprtk.h>
#include <vtkObjectFactory.h>
 
namespace PUJ_Ogre
{
  /**
   */
  class ParametricFunction_Expression
  {
  public:
    ParametricFunction_Expression( const char* expression )
      : Expression( expression )
      {
        this->Table.add_variable( "u", this->U );
        this->Table.add_variable( "v", this->V );
        this->Table.add_constants( );
        this->Expr.register_symbol_table( this->Table );
        this->Parser.compile( this->Expression, this->Expr );
      }

    virtual ~ParametricFunction_Expression( )
      {
      }

    double eval( const double& u, const double& v ) const
      {
        this->U = u;
        this->V = v;
        return( this->Expr.value( ) );
      }

  protected:
    std::string Expression;
    
    mutable double U;
    mutable double V;
    exprtk::symbol_table< double > Table;
    exprtk::expression< double >   Expr;
    exprtk::parser< double >       Parser;
  };

  vtkStandardNewMacro( ParametricFunction );
} // end namespace


// -------------------------------------------------------------------------
void PUJ_Ogre::ParametricFunction::
 PrintSelf( std::ostream& os, vtkIndent indent )
{
  this->Superclass::PrintSelf(os, indent);

  os
    << indent << "Parameter   : " << this->Parameters << std::endl
    << indent << "X : \"" << this->XExpression << "\"" << std::endl
    << indent << "Y : \"" << this->YExpression << "\"" << std::endl
    << indent << "Y : \"" << this->ZExpression << "\"" << std::endl;
}
 
// -------------------------------------------------------------------------
int PUJ_Ogre::ParametricFunction::
GetDimension( )
{
  return( 2 );
}

// -------------------------------------------------------------------------
void PUJ_Ogre::ParametricFunction::
Evaluate( double uvw[ 3 ], double Pt[ 3 ], double Duvw[ 9 ] )
{
  if( this->XExpr == nullptr )
  {
    this->XExpr = new ParametricFunction_Expression( this->XExpression );
    this->YExpr = new ParametricFunction_Expression( this->YExpression );
    this->ZExpr = new ParametricFunction_Expression( this->ZExpression );

    std::string minU, maxU, minV, maxV, openU, openV;
    std::istringstream( this->Parameters )
      >> minU >> maxU >> openU
      >> minV >> maxV >> openV;

    ParametricFunction_Expression minUE( minU.c_str( ) );
    ParametricFunction_Expression maxUE( maxU.c_str( ) );
    ParametricFunction_Expression minVE( minV.c_str( ) );
    ParametricFunction_Expression maxVE( maxV.c_str( ) );

    this->MinimumU = minUE.eval( 0, 0 );
    this->MaximumU = maxUE.eval( 0, 0 );
    this->MinimumV = minVE.eval( 0, 0 );
    this->MaximumV = maxVE.eval( 0, 0 );
    this->MinimumW = 0;
    this->MaximumW = 0;
    this->JoinU = ( ( openU == "open" )? 0: 1 );
    this->JoinV = ( ( openV == "open" )? 0: 1 );
    this->JoinW = 0;
    this->TwistU = 0;
    this->TwistV = 0;
    this->TwistW = 0;
  } // end if

  Pt[ 0 ] = this->XExpr->eval( uvw[ 0 ], uvw[ 1 ] );
  Pt[ 1 ] = this->YExpr->eval( uvw[ 0 ], uvw[ 1 ] );
  Pt[ 2 ] = this->ZExpr->eval( uvw[ 0 ], uvw[ 1 ] );
}

// -------------------------------------------------------------------------
double PUJ_Ogre::ParametricFunction::
EvaluateScalar( double uvw[ 3 ], double Pt[ 3 ], double Duvw[ 9 ] )
{
  return( 0 );
}

// -------------------------------------------------------------------------
PUJ_Ogre::ParametricFunction::
ParametricFunction( )
  : vtkParametricFunction( )
{
  this->SetParameters( "0 0 closed 0 0 closed" );
  this->SetXExpression( "0" );
  this->SetYExpression( "0" );
  this->SetZExpression( "0" );

  this->DerivativesAvailable = 0;
}

// -------------------------------------------------------------------------
 PUJ_Ogre::ParametricFunction::
 ~ParametricFunction( )
{
  if( this->XExpr != nullptr ) delete this->XExpr;
  if( this->YExpr != nullptr ) delete this->YExpr;
  if( this->ZExpr != nullptr ) delete this->ZExpr;
}

// eof - $RCSfile$
