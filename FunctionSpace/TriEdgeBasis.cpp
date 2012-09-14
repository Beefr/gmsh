#include "TriEdgeBasis.h"
#include "Legendre.h"

TriEdgeBasis::TriEdgeBasis(const int order){
  // Set Basis Type //
  this->order = order;
 
  type = 1;
  dim  = 2;

  nVertex = 0;
  nEdge   = 3 * (order + 1);
  nFace   = 0;
  nCell   = ((order - 1) * order + order - 1);

  size = (order + 1) * (order + 2);

  // Alloc Temporary Space //
  const int orderPlus     = order + 1;
  const int orderMinus    = order - 1;

  Polynomial* legendre    = new Polynomial[orderPlus];
  Polynomial* intLegendre = new Polynomial[orderPlus];
  Polynomial* u           = new Polynomial[orderPlus];
  Polynomial* v           = new Polynomial[orderPlus];

  Polynomial  lagrange    [3];
  Polynomial  lagrangeSum [3];
  Polynomial  lagrangeSub [3];
  Polynomial rLagrangeSub [3];

  // Classical and Intrated-Scaled Legendre Polynomial //
  Legendre::legendre(legendre, order);
  Legendre::intScaled(intLegendre, orderPlus);

  // Lagrange //
  lagrange[0] = 
    Polynomial(1, 0, 0, 0) - Polynomial(1, 1, 0, 0) - Polynomial(1, 0, 1, 0);

  lagrange[1] = 
    Polynomial(1, 1, 0, 0);

  lagrange[2] = 
    Polynomial(1, 0, 1, 0);

  // Lagrange Sum //
  for(int i = 0, j = 1; i < 3; i++, j = (j + 1) % 3)
     lagrangeSum[i] = lagrange[j] + lagrange[i];

  // Lagrange Sub (& revert) //
  for(int i = 0, j = 1; i < 3; i++, j = (j + 1) % 3){
      lagrangeSub[i] = lagrange[i] - lagrange[j];
     rLagrangeSub[i] = lagrange[j] - lagrange[i];
  }


  // Basis & Revert (temporary --- *no* const) //
  std::vector<std::vector<Polynomial>*>    basis(size);
  std::vector<std::vector<Polynomial>*> revBasis(size);


  // Edge Based (Nedelec) //
  int i = 0;

  for(int j = 1; i < 3; j = (j + 1) % 3){
    // Direct
    std::vector<Polynomial> tmp = lagrange[j].gradient();
    tmp[0].mul(lagrange[i]);
    tmp[1].mul(lagrange[i]);
    tmp[2].mul(lagrange[i]);

    basis[i] = new std::vector<Polynomial>(lagrange[i].gradient());

    basis[i]->at(0).mul(lagrange[j]);
    basis[i]->at(1).mul(lagrange[j]);
    basis[i]->at(2).mul(lagrange[j]);      

    basis[i]->at(0).sub(tmp[0]);
    basis[i]->at(1).sub(tmp[1]);
    basis[i]->at(2).sub(tmp[2]);

    // Revert 
    std::vector<Polynomial> tmpR = lagrange[i].gradient();
    tmpR[0].mul(lagrange[j]);
    tmpR[1].mul(lagrange[j]);
    tmpR[2].mul(lagrange[j]);

    revBasis[i] = new std::vector<Polynomial>(lagrange[j].gradient());

    revBasis[i]->at(0).mul(lagrange[i]);
    revBasis[i]->at(1).mul(lagrange[i]);
    revBasis[i]->at(2).mul(lagrange[i]);
   
    revBasis[i]->at(0).sub(tmpR[0]);
    revBasis[i]->at(1).sub(tmpR[1]);
    revBasis[i]->at(2).sub(tmpR[2]);

    // Next
    i++;
  }

  // Edge Based (High Order) //
  for(int l = 1; l < orderPlus; l++){
    for(int e = 0; e < 3; e++){
      basis[i] = 
	new std::vector<Polynomial>((intLegendre[l].compose(lagrangeSub[e],  
							    lagrangeSum[e])).gradient());

      revBasis[i] = 
	new std::vector<Polynomial>((intLegendre[l].compose(rLagrangeSub[e], 
							    lagrangeSum[e])).gradient());
      i++;
    }
  }

  // Cell Based (Preliminary) //
  Polynomial p = lagrange[2] * 2 - Polynomial(1, 0, 0, 0);
  
  for(int l = 0; l < orderPlus; l++){
    u[l] = intLegendre[l].compose(lagrangeSub[0] * -1, lagrangeSum[0]);
    v[l] = legendre[l].compose(p);
    v[l].mul(lagrange[2]);
  }

  // Cell Based (Type 1) //
  for(int l1 = 1; l1 < order; l1++){
    for(int l2 = 0; l2 + l1 - 1 < orderMinus; l2++){
      std::vector<Polynomial> tmp = v[l2].gradient();
      tmp[0].mul(u[l1]);
      tmp[1].mul(u[l1]);
      tmp[2].mul(u[l1]);

      basis[i] = new std::vector<Polynomial>(u[l1].gradient());
      
      basis[i]->at(0).mul(v[l2]);
      basis[i]->at(1).mul(v[l2]);
      basis[i]->at(2).mul(v[l2]);

      basis[i]->at(0).add(tmp[0]);
      basis[i]->at(1).add(tmp[1]);
      basis[i]->at(2).add(tmp[2]);

      revBasis[i] = basis[i];
      
      i++;
    }
  }

  // Cell Based (Type 2) //
  for(int l1 = 1; l1 < order; l1++){
    for(int l2 = 0; l2 + l1 - 1 < orderMinus; l2++){
      std::vector<Polynomial> tmp = v[l2].gradient();
      tmp[0].mul(u[l1]);
      tmp[1].mul(u[l1]);
      tmp[2].mul(u[l1]);

      basis[i] = new std::vector<Polynomial>(u[l1].gradient());

      basis[i]->at(0).mul(v[l2]);
      basis[i]->at(1).mul(v[l2]);
      basis[i]->at(2).mul(v[l2]);

      basis[i]->at(0).sub(tmp[0]);
      basis[i]->at(1).sub(tmp[1]);
      basis[i]->at(2).sub(tmp[2]);
 
      revBasis[i] = basis[i];

      i++;
    }
  }

  // Cell Based (Type 3) //
  for(int l = 0; l < orderMinus; l++){
    basis[i] = new std::vector<Polynomial>(*basis[0]);

    basis[i]->at(0).mul(v[l]);
    basis[i]->at(1).mul(v[l]);
    basis[i]->at(2).mul(v[l]);

    revBasis[i] = basis[i];
    
    i++;
  }

  // Free Temporary Sapce //
  delete[] legendre;
  delete[] intLegendre;
  delete[] u;
  delete[] v;


  // Set Basis //
  this->basis    = new std::vector<const std::vector<Polynomial>*>
    (basis.begin(), basis.end());

  this->revBasis = new std::vector<const std::vector<Polynomial>*>
    (revBasis.begin(), revBasis.end());
}

TriEdgeBasis::~TriEdgeBasis(void){
  for(int i = 0; i < size; i++){
    delete (*basis)[i];

    if(i >= nVertex && i < nVertex + nEdge)
      delete (*revBasis)[i];
  }

  delete basis;
  delete revBasis;
}
