// Gmsh - Copyright (C) 1997-2009 C. Geuzaine, J.-F. Remacle
//
// See the LICENSE.txt file for license information. Please report all
// bugs and problems to <gmsh@geuz.org>.

#include <stdlib.h>
#include "GmshConfig.h"
#include "GModel.h"
#include "FiniteElement.h"
#include "gmshAssembler.h"
#include "gmshProjection.h"
#include "gmshLaplace.h"
#include "gmshHelmholtz.h"
#include "gmshLinearSystemGmm.h"

#if defined(HAVE_MATH_EVAL)
#include "matheval.h"
#endif

StringXNumber FiniteElementOptions_Number[] = {
  {GMSH_FULLRC, "Omega", NULL, 1.},
  {GMSH_FULLRC, "Gamma1", NULL, 0.},
  {GMSH_FULLRC, "Gamma1Value", NULL, 0.},
  {GMSH_FULLRC, "Gamma2", NULL, 0.},
  {GMSH_FULLRC, "Gamma2Value", NULL, 0.}
};

StringXString FiniteElementOptions_String[] = {
  {GMSH_FULLRC, "Equation", NULL, "Projection"},
  {GMSH_FULLRC, "Parameter", NULL, "Sin(x*y)"},
  {GMSH_FULLRC, "Gamma1BC", NULL, ""},
  {GMSH_FULLRC, "Gamma2BC", NULL, ""},
};

extern "C"
{
  GMSH_Plugin *GMSH_RegisterFiniteElementPlugin()
  {
    return new GMSH_FiniteElementPlugin();
  }
}

void GMSH_FiniteElementPlugin::getName(char *name) const
{
  strcpy(name, "Finite Element");
}

void GMSH_FiniteElementPlugin::getInfos(char *author, char *copyright,
                                   char *help_text) const
{
  strcpy(author, "C. Geuzaine, J.-F. Remacle");
  strcpy(copyright, "C. Geuzaine, J.-F. Remacle");
  strcpy(help_text,
         "Plugin(FiniteElement) solves simple PDEs\n"
         "using the finite element method. This is only\n"
         "intended as a demonstration tool: it is NOT\n"
         "intended for general use."
         "\n"
         "Plugin(FiniteElement) creates a new view.\n");
}

int GMSH_FiniteElementPlugin::getNbOptions() const
{
  return sizeof(FiniteElementOptions_Number) / sizeof(StringXNumber);
}

StringXNumber *GMSH_FiniteElementPlugin::getOption(int iopt)
{
  return &FiniteElementOptions_Number[iopt];
}

int GMSH_FiniteElementPlugin::getNbOptionsStr() const
{
  return sizeof(FiniteElementOptions_String) / sizeof(StringXString);
}

StringXString *GMSH_FiniteElementPlugin::getOptionStr(int iopt)
{
  return &FiniteElementOptions_String[iopt];
}

void GMSH_FiniteElementPlugin::catchErrorMessage(char *errorMessage) const
{
  strcpy(errorMessage, "FiniteElement failed...");
}

template<class scalar>
class gmshMathEvalFunction : public gmshFunction<scalar> {
 private:
  std::string _str;
  void *_f;
 public:
  gmshMathEvalFunction(std::string str) : _str(str)
  {
#if defined(HAVE_MATH_EVAL)
    _f = evaluator_create((char*)_str.c_str());
    if(!_f) Msg::Error("Invalid expression '%s'", _str.c_str());
#endif
  }
  virtual ~gmshMathEvalFunction()
  {
#if defined(HAVE_MATH_EVAL)
    if(_f) evaluator_destroy(_f);
#endif
  }
  virtual scalar operator () (double x, double y, double z) const 
  { 
#if defined(HAVE_MATH_EVAL)
    if(_f){
      char *names[] = {"x", "y", "z"};
      double values[] = {x, y, z};
      return evaluator_evaluate(_f, sizeof(names)/sizeof(names[0]), names, values);
    }
    return atof(_str.c_str());
#else
    return atof(_str.c_str());
#endif
  }
};

#if defined(HAVE_GMM)
template<class scalar>
class solver{
 public:
  gmshLinearSystemGmm<scalar> *lsys;
  gmshAssembler<scalar> *myAssembler;
  solver()
  {
    int volume = (int)FiniteElementOptions_Number[0].def;
    int surface1 = (int)FiniteElementOptions_Number[1].def;
    double value1 = FiniteElementOptions_Number[2].def;
    int surface2 = (int)FiniteElementOptions_Number[3].def;
    double value2 = FiniteElementOptions_Number[4].def;
    std::string bc1 = FiniteElementOptions_String[2].def;
    std::string bc2 = FiniteElementOptions_String[3].def;

    lsys = new gmshLinearSystemGmm<scalar>;
    lsys->setPrec(1.e-10);
    lsys->setNoisy(2);
    myAssembler = new gmshAssembler<scalar>(lsys);

    GModel *m = GModel::current();
    std::vector<MVertex*> vertices;

    int dim = m->getNumRegions() ? 3 : 2;
    
    if(bc1 == "Dirichlet"){
      m->getMeshVertices(surface1, dim-1, vertices);
      for(unsigned int i = 0; i < vertices.size(); i++)
        myAssembler->fixVertex(vertices[i], 0, 1, value1);
    }
    if(bc2 == "Dirichlet"){
      m->getMeshVertices(surface2, dim-1, vertices);
      for(unsigned int i = 0; i < vertices.size(); i++)
        myAssembler->fixVertex(vertices[i], 0, 1, value2);
    }
    m->getMeshVertices(volume, dim, vertices);
    for(unsigned int i = 0; i < vertices.size(); i++)
      myAssembler->numberVertex(vertices[i], 0, 1);

    Msg::Info("Assembler: %d dofs, %d fixed",
              myAssembler->sizeOfR(), myAssembler->sizeOfF());
  }
};
#endif

PView *GMSH_FiniteElementPlugin::execute(PView *v)
{
  std::string equation = FiniteElementOptions_String[0].def;
  std::string parameter = FiniteElementOptions_String[1].def;
  int volume = (int)FiniteElementOptions_Number[0].def;

#if defined(HAVE_GMM)
  GModel *m = GModel::current();
  std::map<int, std::vector<GEntity*> > groups[4];
  m->getPhysicalGroups(groups);
  int dim = m->getNumRegions() ? 3 : 2;

  std::vector<MVertex*> vertices;
  m->getMeshVertices(volume, dim, vertices);
  std::map<int, std::vector<double> > data;

  if(equation == "Projection"){
    solver<double> s;
    if(!s.myAssembler->sizeOfR()) return 0;
    gmshMathEvalFunction<double> par(parameter);
    gmshProjectionTerm projection(m, 1);
    for(unsigned int i = 0; i < groups[dim][volume].size(); i++)
      projection.addToMatrix(*s.myAssembler, groups[dim][volume][i]);
    projection.addNeumann(volume, dim, 0, 1, par, *s.myAssembler);
    s.lsys->systemSolve();
    for (unsigned int i = 0; i < vertices.size(); i++)
      data[vertices[i]->getNum()].push_back
        (s.myAssembler->getDofValue(vertices[i], 0, 1));
    PView *pv = new PView("projection", "NodeData", m, data, 0.);
  }
  else if(equation == "Laplace"){
    solver<double> s;
    if(!s.myAssembler->sizeOfR()) return 0;
    gmshMathEvalFunction<double> par(parameter);
    gmshLaplaceTerm laplace(m, &par, 1);
    for(unsigned int i = 0; i < groups[dim][volume].size(); i++)
      laplace.addToMatrix(*s.myAssembler, groups[dim][volume][i]);
    s.lsys->systemSolve();
    for (unsigned int i = 0; i < vertices.size(); i++)
      data[vertices[i]->getNum()].push_back
        (s.myAssembler->getDofValue(vertices[i], 0, 1));
    PView *pv = new PView("laplace", "NodeData", m, data, 0.);
  }
  else if(equation == "Helmholtz"){
    solver<std::complex<double> > s;
    if(!s.myAssembler->sizeOfR()) return 0;
    gmshMathEvalFunction<std::complex<double> > par(parameter);
    gmshHelmholtzTerm helmholtz(m, &par, 1);
    for(unsigned int i = 0; i < groups[dim][volume].size(); i++)
      helmholtz.addToMatrix(*s.myAssembler, groups[dim][volume][i]);
    s.lsys->setGmres(1);
    s.lsys->systemSolve();
    for (unsigned int i = 0; i < vertices.size(); i++)
      data[vertices[i]->getNum()].push_back
        (s.myAssembler->getDofValue(vertices[i], 0, 1).real());
    PView *pv = new PView("helmholtz", "NodeData", m, data, 0.);
    data.clear();
    for (unsigned int i = 0; i < vertices.size(); i++)
      data[vertices[i]->getNum()].push_back
        (s.myAssembler->getDofValue(vertices[i], 0, 1).imag());
    pv->addStep(m, data, 1.);
  }
  else{
    Msg::Error("Unknown equation to solve");
  }
#endif
  return 0;
}
