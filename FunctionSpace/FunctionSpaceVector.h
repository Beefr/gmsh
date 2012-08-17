#ifndef _FUNCTIONSPACEVECTOR_H_
#define _FUNCTIONSPACEVECTOR_H_

#include "fullMatrix.h"
#include "BasisVector.h"
#include "FunctionSpace.h"

/**
    @interface FunctionSpaceVector
    @brief Common Interface of all Vectorial FunctionSpaces
    
    This is the @em common @em interface of
    all @em Vectorial FunctionSpaces.@n

    A FunctionSpaceVector can be @em interpolated.

    @note
    A VectorFunctionSpace is an @em interface, so it
    can't be instantiated.
*/


class FunctionSpaceVector : public FunctionSpace{
 public:
  virtual ~FunctionSpaceVector(void);

  virtual fullVector<double> 
    interpolate(const MElement& element, 
		const std::vector<double>& coef,
		const fullVector<double>& xyz) const = 0;

  const BasisVector& getBasis(const MElement& element) const;
};


/**
   @fn FunctionSpaceVector::~FunctionSpaceVector
   Deletes this FunctionSpaceVector
   **

   @fn FunctionSpaceVector::interpolate
   @param element The MElement to interpolate on
   @param coef The coefficients of the interpolation
   @param xyz The coordinate 
   (of point @em inside the given @c element)
   of the interpolation

   @return Returns the (vectorial) interpolated value

   @warning
   If the given coordinate are not in the given
   @c element @em Bad @em Things may happend
   
   @todo
   If the given coordinate are not in the given
   @c element @em Bad @em Things may happend
   ---> check
   **

   @fn FunctionSpaceVector::getBasis
   @param element A MElement of the support 
   of this FunctionSpace
   @return Returns the Basis (BasisVector) associated
   to the given MElement
 */


//////////////////////
// Inline Functions //
//////////////////////

inline const BasisVector& FunctionSpaceVector::
getBasis(const MElement& element) const{
  return static_cast<const BasisVector&>(*basis);
}

#endif
