// Gmsh - Copyright (C) 1997-2009 C. Geuzaine, J.-F. Remacle
//
// See the LICENSE.txt file for license information. Please report all
// bugs and problems to <gmsh@geuz.org>.

#ifndef _FEM_TERM_H_
#define _FEM_TERM_H_

#include <math.h>
#include <map>
#include <vector>
#include "fullMatrix.h"
#include "simpleFunction.h"
#include "dofManager.h"
#include "GModel.h"
#include "SElement.h"
#include "groupOfElements.h"

// a nodal finite element term : variables are always defined at nodes
// of the mesh
template<class dataVec, class dataMat>
class femTerm {
 protected:
  GModel *_gm;
 public:
  femTerm(GModel *gm) : _gm(gm) {}
  virtual ~femTerm(){}
  // return the number of columns of the element matrix
  virtual int sizeOfC(SElement *se) const = 0;
  // return the number of rows of the element matrix
  virtual int sizeOfR(SElement *se) const = 0;
  // in a given element, return the dof associated to a given row (column)
  // of the local element matrix
  virtual Dof getLocalDofR(SElement *se, int iRow) const = 0;
  // default behavior: symmetric
  virtual Dof getLocalDofC(SElement *se, int iCol) const
  { 
    return getLocalDofR(se, iCol); 
  }
  // compute the elementary matrix
  virtual void elementMatrix(SElement *se, fullMatrix<dataMat> &m) const = 0;
  virtual void elementVector(SElement *se, fullVector<dataVec> &m) const 
  {
    m.scale(0.0);
  }

  // add the contribution from all the elements in the intersection
  // of two element groups L and C
  void addToMatrix(dofManager<dataVec, dataMat> &dm, 
		   groupOfElements &L, 
		   groupOfElements &C) const
  {
    groupOfElements::elementContainer::const_iterator it = L.begin();
    for ( ; it != L.end() ; ++it){
      MElement *eL = *it;
      if ( &C == &L || C.find(eL) ){
	SElement se(eL);
	addToMatrix(dm, &se);
      }
    }
  }

  // add the contribution from a single element to the dof manager
  void addToMatrix(dofManager<dataVec, dataMat> &dm, SElement *se) const
  {
    const int nbR = sizeOfR(se);
    const int nbC = sizeOfC(se);
    fullMatrix<dataMat> localMatrix(nbR, nbC);
    elementMatrix(se, localMatrix);
    addToMatrix(dm, localMatrix, se);
  }
  void addToMatrix(dofManager<dataVec, dataMat> &dm, 
                   fullMatrix<dataMat> &localMatrix, 
                   SElement *se) const
  {
    const int nbR = localMatrix.size1();
    const int nbC = localMatrix.size2();
    std::vector<Dof> R,C;
    for (int j = 0; j < nbR; j++)
      R.push_back(getLocalDofR(se, j));
    for (int k = 0; k < nbC; k++)
      C.push_back(getLocalDofC(se, k));

/*     for (int i = 0; i < nbC; i++) */
/*       for (int j = 0; j < nbR; j++) */
/* 	dm.assemble(getLocalDofR(se, i), getLocalDofC(se, j), localMatrix(i,j)); */
/*     return; */
    
    bool sym = true;
    if (nbR == nbC){
      for (int i=0;i<nbR;i++)
	if (!(C[i] == R[i]))sym = false;
    }
    else sym = false;
    if (!sym)
      dm.assemble(R, C, localMatrix);
    else
      dm.assemble(R, localMatrix);
  }
  void dirichletNodalBC(int physical, int dim, int comp, int field,
                        const simpleFunction<dataVec> &e,
                        dofManager<dataVec,dataMat> &dm)
  {
    std::vector<MVertex *> v;
    GModel *m = _gm;
    m->getMeshVerticesForPhysicalGroup(dim, physical, v);
    for (unsigned int i = 0; i < v.size(); i++)
      dm.fixVertex(v[i], comp, field, e(v[i]->x(), v[i]->y(), v[i]->z()));
  }
  void neumannNodalBC(int physical, int dim, int comp,int field,
                      const simpleFunction<dataVec> &fct,
                      dofManager<dataVec, dataMat> &dm)
  {
    std::map<int, std::vector<GEntity*> > groups[4];
    GModel *m = _gm;
    m->getPhysicalGroups(groups);
    std::map<int, std::vector<GEntity*> >::iterator it = groups[dim].find(physical);  
    if (it == groups[dim].end()) return;
    double jac[3][3];
    double sf[256];
    for (unsigned int i = 0; i < it->second.size(); ++i){
      GEntity *ge = it->second[i];
      for (unsigned int j = 0; j < ge->getNumMeshElements(); j++){
        MElement *e = ge->getMeshElement(j);
        int integrationOrder = 2 * e->getPolynomialOrder();
        int nbNodes = e->getNumVertices();
        int npts;
        IntPt *GP;
        e->getIntegrationPoints(integrationOrder, &npts, &GP);  
        for (int ip = 0; ip < npts; ip++){
          const double u = GP[ip].pt[0];
          const double v = GP[ip].pt[1];
          const double w = GP[ip].pt[2];
          const double weight = GP[ip].weight;
          const double detJ = e->getJacobian(u, v, w, jac);
          SPoint3 p; e->pnt(u, v, w, p);
          e->getShapeFunctions(u, v, w, sf);
          const dataVec FCT = fct(p.x(), p.y(), p.z());
          for (int k = 0; k < nbNodes; k++){
            dm.assemble(e->getVertex(k), comp, field, detJ * weight * sf[k] * FCT);
          }
        }
      }
    }
  }

  void addToRightHandSide(dofManager<dataVec, dataMat> &dm, groupOfElements &C) const 
  {
    groupOfElements::elementContainer::const_iterator it = C.begin();
    for ( ; it != C.end() ; ++it){
      MElement *eL = *it;
      SElement se(eL);
      int nbR = sizeOfR(&se);
      fullVector<dataVec> V(nbR);
      elementVector(&se, V);
      // assembly
      for (int j = 0; j < nbR; j++) dm.assemble(getLocalDofR(&se, j), V(j));
    }
  }
};

#endif
