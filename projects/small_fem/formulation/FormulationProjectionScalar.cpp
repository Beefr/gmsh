#include "BasisGenerator.h"
#include "GaussIntegration.h"
#include "GroupOfJacobian.h"
#include "GroupOfElement.h"

#include "FormulationProjectionScalar.h"

using namespace std;

FormulationProjectionScalar::
FormulationProjectionScalar(double (*f)(fullVector<double>& xyz),
			    FunctionSpaceScalar& fs){
  // Save fspace //
  fspace = &fs;
  basis  = &fs.getBasis(0);

  // Domain //
  GroupOfElement& goe = fs.getSupport();

  // Gaussian Quadrature Data  //
  // NB: We need to integrad f_i * f_j or f_i * g
  fullMatrix<double> gC;
  fullVector<double> gW;

  // Look for 1st element to get element type
  // (We suppose only one type of Mesh !!)
 unsigned int order = basis->getOrder();

  // if Order == 0 --> we want Nedelec Basis of ordre *almost* one //
  if(order == 0)
    order = 1;

  gaussIntegration::get(goe.get(0).getType(), 2 * 2 * order, gC, gW);

  // Local Terms //
  basis->preEvaluateFunctions(gC);
  GroupOfJacobian jac(goe, gC, "jacobian");

  localTerms1 = new TermFieldField(jac, *basis, gW);
  localTerms2 = new TermProjectionField(jac, *basis, gW, gC, f);
}

FormulationProjectionScalar::~FormulationProjectionScalar(void){
  delete localTerms2;
  delete localTerms1;
}

double FormulationProjectionScalar::weak(unsigned int dofI, unsigned int dofJ,
                                         unsigned int elementId) const{

  return localTerms1->getTerm(dofI, dofJ, elementId);
}

double FormulationProjectionScalar::rhs(unsigned int equationI,
					unsigned int elementId) const{

  return localTerms2->getTerm(0, equationI, elementId);
}

bool FormulationProjectionScalar::isGeneral(void) const{
  return false;
}

double FormulationProjectionScalar::weakB(unsigned int dofI,
                                          unsigned int dofJ,
                                          unsigned int elementId) const{
  return 0;
}

const FunctionSpace& FormulationProjectionScalar::fs(void) const{
  return *fspace;
}
