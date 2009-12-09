#ifndef _DG_SYSTEM_OF_EQUATIONS_
#define _DG_SYSTEM_OF_EQUATIONS_
#include "GmshConfig.h"
#include "GModel.h"
#include "dgAlgorithm.h"
#include "dgGroupOfElements.h"
#include "dgAlgorithm.h"
#include "dgConservationLaw.h"
#include "Gmsh.h"

#if defined(HAVE_LUA)
// include lua stuff
extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}
// Use luna for c++ class member functions bindings
#include "luna.h"
#endif // HAVE LUA

struct dgDofContainer {
private:
  dgDofContainer (const dgDofContainer&);  
public:
  int _dataSize; // the full data size i.e. concerning all groups
  fullVector<double> * _data; // the full data itself
  std::vector<fullMatrix<double> *> _dataProxys; // proxys 
  dgDofContainer (std::vector<dgGroupOfElements*> &groups, const dgConservationLaw &claw);
  ~dgDofContainer ();  
};


class dgSystemOfEquations {
  // the mesh and the model
  GModel *_gm;
  // the algorithm that computes DG operators
  dgAlgorithm *_algo;
  // the conservation law
  dgConservationLaw *_claw;
  std::string _cLawName;
  // polynomial order (should be more general)
  int _order;
  // dimension of the problem
  int _dimension;
  // solution and righ hand sides
  dgDofContainer *_solution;
  dgDofContainer *_rightHandSide;
  // groups of elements (volume terms)
  std::vector<dgGroupOfElements*> _elementGroups;
  // groups of faces (interface terms)
  std::vector<dgGroupOfFaces*> _faceGroups;
  // groups of faces (boundary conditions)
  std::vector<dgGroupOfFaces*> _boundaryGroups;
  dgSystemOfEquations(const dgSystemOfEquations &) {}
public:
  // lua stuff
#if defined(HAVE_LUA)
  static const char className[];
  static Luna<dgSystemOfEquations>::RegType methods[];
  static void Register(lua_State *L);
  int exportSolution (lua_State *L); // export the solution
  int setOrder (lua_State *L); // set the polynomial order
  int setConservationLaw (lua_State *L); // set the conservationLaw
  int addBoundaryCondition (lua_State *L); // add a boundary condition : "physical name", "type", [options]
  int setup (lua_State *L); // setup the groups and allocate
  int L2Projection (lua_State *L); // assign the solution to a given function
  int RK44 (lua_State *L); // assign the solution to a given function
  dgSystemOfEquations(lua_State *L);
#endif // HAVE LUA
  inline const fullMatrix<double> getSolutionProxy (int iGroup, int iElement){
    return fullMatrix<double> ( *_solution->_dataProxys [iGroup] ,
				iElement * _claw->nbFields(),_claw->nbFields());
  }
  void export_solution_as_is (const std::string &fileName);
  ~dgSystemOfEquations();
private:
};

#endif // _DG_SYSTEM_OF_EQUATIONS_

