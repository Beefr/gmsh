#include "BasisGenerator.h"
#include "GroupOfJacobian.h"
#include "Quadrature.h"

#include "Exception.h"
#include "FormulationPoisson.h"

using namespace std;

// Source Terms //
const unsigned int FormulationPoisson::sourceOrder = 1;

double FormulationPoisson::gSource(fullVector<double>& xyz){
  return 1;
}

// Poisson //
FormulationPoisson::FormulationPoisson(GroupOfElement& goe,
                                       unsigned int order){
  // Can't have 0th order //
  if(order == 0)
    throw
      Exception("Can't have a Poisson formulation of order 0");

  // Function Space & Basis //
  basis  = BasisGenerator::generate(goe.get(0).getType(),
                                    0, order, "hierarchical");

  fspace = new FunctionSpaceScalar(goe, *basis);

  // Gaussian Quadrature //
  Quadrature gaussGradGrad(goe.get(0).getType(), order - 1, 2);
  Quadrature gaussFF(goe.get(0).getType(), order + sourceOrder, 1);

  const fullMatrix<double>& gCL = gaussGradGrad.getPoints();
  const fullVector<double>& gWL = gaussGradGrad.getWeights();

  const fullMatrix<double>& gCR = gaussFF.getPoints();
  const fullVector<double>& gWR = gaussFF.getWeights();

  // Local Terms //
  basis->preEvaluateDerivatives(gCL);
  basis->preEvaluateFunctions(gCR);

  GroupOfJacobian jacL(goe, gCL, "invert");
  GroupOfJacobian jacR(goe, gCR, "jacobian");

  localTermsL = new TermGradGrad(jacL, *basis, gWL);
  localTermsR = new TermProjectionField(jacR, *basis, gWR, gCR, gSource);
}

FormulationPoisson::~FormulationPoisson(void){
  delete basis;
  delete fspace;

  delete localTermsL;
  delete localTermsR;
}

double FormulationPoisson::weak(unsigned int dofI, unsigned int dofJ,
                                unsigned int elementId) const{

  return localTermsL->getTerm(dofI, dofJ, elementId);
}

double FormulationPoisson::rhs(unsigned int equationI,
                               unsigned int elementId) const{

  return localTermsR->getTerm(0, equationI, elementId);
}

bool FormulationPoisson::isGeneral(void) const{
  return false;
}

double FormulationPoisson::weakB(unsigned int dofI,
                                 unsigned int dofJ,
                                 unsigned int elementId) const{
  return 0;
}

const FunctionSpace& FormulationPoisson::fs(void) const{
  return *fspace;
}
