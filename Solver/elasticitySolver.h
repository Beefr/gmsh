// Gmsh - Copyright (C) 1997-2010 C. Geuzaine, J.-F. Remacle
//
// See the LICENSE.txt file for license information. Please report all
// bugs and problems to <gmsh@geuz.org>.

#ifndef _ELASTICITY_SOLVER_H_
#define _ELASTICITY_SOLVER_H_

#include <map>
#include <string>
#include "SVector3.h"
#include "dofManager.h"
#include "simpleFunction.h"
#include "functionSpace.h"

class GModel;
class PView;
class groupOfElements;

struct LagrangeMultiplierField {
  int _tag;
  groupOfElements *g;
  double _tau;
  SVector3 _d;
  simpleFunction<double> _f;
  LagrangeMultiplierField() : g(0),_tag(0){}
};

struct elasticField {
  int _tag; // tag for the dofManager
  groupOfElements *g; // support for this field
  double _E, _nu; // specific elastic datas (should be somewhere else)
  elasticField () : g(0),_tag(0){}
};

struct BoundaryCondition
{
  enum location{UNDEF,ON_VERTEX,ON_EDGE,ON_FACE,ON_VOLUME};
  location onWhat; // on vertices or elements
  int _tag; // tag for the dofManager
  groupOfElements *g; // support for this BC
  BoundaryCondition() : g(0),_tag(0),onWhat(UNDEF) {}
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
class elasticitySolver
{
 protected:
  GModel *pModel;
  int _dim, _tag;
  dofManager<double> *pAssembler;
  FunctionSpace<SVector3> *LagSpace;
  FunctionSpace<double> *LagrangeMultiplierSpace;

  // young modulus and poisson coefficient per physical
  std::vector<elasticField> elasticFields;
  std::vector<LagrangeMultiplierField> LagrangeMultiplierFields;
  // neumann BC
  std::vector<neumannBC> allNeumann;
  // dirichlet BC
  std::vector<dirichletBC> allDirichlet;
  
 public:
  elasticitySolver(int tag) : _tag(tag),LagSpace(0),pAssembler(0),LagrangeMultiplierSpace(0) {}
  virtual ~elasticitySolver()
  {
    if (LagSpace) delete LagSpace;
    if (LagrangeMultiplierSpace) delete LagrangeMultiplierSpace;
    if (pAssembler) delete pAssembler;
  }
  void readInputFile(const std::string &meshFileName);
  void read(char *s) {readInputFile(s);}
  virtual void setMesh(const std::string &meshFileName);
  void solve();
  virtual PView *buildDisplacementView(const std::string &postFileName);
  virtual PView *buildLagrangeMultiplierView(const std::string &posFileName);
  virtual PView *buildElasticEnergyView(const std::string &postFileName);
  virtual PView *buildVonMisesView(const std::string &postFileName);
  // std::pair<PView *, PView*> buildErrorEstimateView
  //   (const std::string &errorFileName, double, int);
  // std::pair<PView *, PView*> buildErrorEstimateView
  //   (const std::string &errorFileName, const elasticityData &ref, double, int);
};

class binding;
void elasticitySolverRegisterBindings(binding *b);

#endif
