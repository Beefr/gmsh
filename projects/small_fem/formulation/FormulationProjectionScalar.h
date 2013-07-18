#ifndef _FORMULATIONPROJECTIONSCALAR_H_
#define _FORMULATIONPROJECTIONSCALAR_H_

#include "FunctionSpaceScalar.h"
#include "fullMatrix.h"

#include "TermFieldField.h"
#include "TermProjectionField.h"

#include "Formulation.h"

/**
   @class FormulationProjectionScalar
   @brief Formulation for the Projection of a Scalar Function problem

   Scalar Formulation for the @em L2 @em Projection problem
 */

class FormulationProjectionScalar: public Formulation{
 private:
  // Function Space & Basis //
  FunctionSpaceScalar* fspace;
  const Basis*         basis;

  // Local Terms //
  TermFieldField*      localTerms1;
  TermProjectionField* localTerms2;

 public:
  FormulationProjectionScalar(double (*f)(fullVector<double>& xyz),
                              FunctionSpaceScalar& fs);

  virtual ~FormulationProjectionScalar(void);

  virtual bool isGeneral(void) const;

  virtual double weak(size_t dofI, size_t dofJ,
                      size_t elementId) const;

  virtual double weakB(size_t dofI, size_t dofJ,
                       size_t elementId) const;

  virtual double rhs(size_t equationI,
                     size_t elementId) const;

  virtual const FunctionSpace& fs(void) const;
};

/**
   @fn FormulationProjectionScalar::FormulationProjectionScalar
   @param f The function to project
   @param fs A FunctionSpaceNode

   Instantiates a new FormulationProjectionScalar to project
   the given function@n

   FormulationProjectionScalar will use the given FunctionSpace
   for the projection
   **

   @fn FormulationProjectionScalar::~FormulationProjectionScalar
   Deletes the this FormulationProjectionScalar
*/

#endif
