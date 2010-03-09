// Gmsh - Copyright (C) 1997-2009 C. Geuzaine, J.-F. Remacle
//
// See the LICENSE.txt file for license information. Please report all
// bugs and problems to <gmsh@geuz.org>.

#ifndef _DGC0PLATE_SOLVER_H_
#define _DGC0PLATE_SOLVER_H_

#include <map>
#include <string>
#include "SVector3.h"
#include "dofManager.h"
#include "simpleFunction.h"
#include "functionSpace.h"
#include "MInterfaceElement.h"

class GModelWithInterface;
class PView;
class groupOfElements;

struct elasticField {
  int _tag; // tag for the dofManager
  groupOfElements *g; // support for this field
  double _E, _nu; // specific elastic datas (should be somewhere else)
  elasticField () : g(0),_tag(0){}
};

// Elasticfield for DG element (has a group of elements for interfaceelement)
struct DGelasticField{
  int _tag; // tag for the dofManager
  groupOfElements *g; // support for this field
  std::vector<MInterfaceElement*> gi; // support for the interfaceElement TODO cast to a groupOfElements
  std::vector<MInterfaceElement*> gib; // support for the interfaceElement TODO cast to a groupOfElements
  double _E, _nu, _beta1, _h;  // specific elastic datas (should be somewhere else)
  DGelasticField () : g(0), _tag(0){}
};

struct BoundaryCondition
{
  enum location{UNDEF,ON_VERTEX,ON_EDGE,ON_FACE,ON_VOLUME};
  location onWhat; // on vertices or elements
  int _tag; // tag for the dofManager
  groupOfElements *g; // support for this BC
  BoundaryCondition() : g(0),_tag(0),onWhat(UNDEF) {};
};

struct dirichletBC : public BoundaryCondition
{
  int _comp; // component
  simpleFunction<double> _f;
  dirichletBC () :BoundaryCondition(),_comp(0),_f(0){}
};

struct neumannBC  : public BoundaryCondition
{
  simpleFunction<SVector3> _f;
  neumannBC () : BoundaryCondition(),_f(SVector3(0,0,0)){}
};

// an elastic solver ...
class DgC0PlateSolver
{
 protected:
  GModelWithInterface *pModel;
  int _dim, _tag;
  dofManager<double> *pAssembler;
  FunctionSpace<SVector3> *LagSpace;

  // young modulus and poisson coefficient per physical
  std::vector<DGelasticField> elasticFields;
  // neumann BC
  std::vector<neumannBC> allNeumann;
  // dirichlet BC
  std::vector<dirichletBC> allDirichlet;

 public:
  DgC0PlateSolver(int tag) : _tag(tag),LagSpace(0),pAssembler(0) {}
  virtual ~DgC0PlateSolver()
  {
    if (LagSpace) delete LagSpace;
    if (pAssembler) delete pAssembler;
  }
  void readInputFile(const std::string &meshFileName);
  void setMesh(const std::string &meshFileName);
  virtual void solve();
  virtual PView *buildDisplacementView(const std::string &postFileName);
  virtual PView *buildElasticEnergyView(const std::string &postFileName);
  virtual PView *buildVonMisesView(const std::string &postFileName);
  // std::pair<PView *, PView*> buildErrorEstimateView
  //   (const std::string &errorFileName, double, int);
  // std::pair<PView *, PView*> buildErrorEstimateView
  //   (const std::string &errorFileName, const elasticityData &ref, double, int);
};


#endif
