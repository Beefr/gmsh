#include <stdio.h>
#include <vector>
#include "GModel.h"
#include "dgGroupOfElements.h"
#include "dgAlgorithm.h"
#include "dgConservationLaw.h"
#include "Gmsh.h"
#include "function.h"


#include "MElement.h"
void print (const char *filename,const dgGroupOfElements &els, double *v);


class dgConservationLawInitialCondition : public dgConservationLaw {
  class gaussian : public dataCacheDouble {
    dataCacheDouble &xyz;
    double _xc,_yc;
    public:
    gaussian(dataCacheMap &cacheMap,double xc, double yc):xyz(cacheMap.get("XYZ",this)),_xc(xc),_yc(yc){};
    void _eval () { 
      if(_value.size1() != xyz().size1())
        _value=fullMatrix<double>(xyz().size1(),1);
      for(int i=0; i< _value.size1(); i++) {
        _value(i,0)=exp(-(pow(xyz(i,0)-_xc,2)+pow(xyz(i,1)-_yc,2))*100);
      }
    }
  };
  public:
  dgConservationLawInitialCondition() {
    _nbf = 1;
  }
  dataCacheDouble *newSourceTerm(dataCacheMap &cacheMap)const {
    return new gaussian(cacheMap,0.2,0.3);
  }
};

int main(int argc, char **argv){
  GmshMergeFile("square1.msh");
  //we probably need a class to group those three ones
  std::vector<dgGroupOfElements*> elementGroups;
  std::vector<dgGroupOfFaces*> faceGroups;
  std::vector<dgGroupOfFaces*> boundaryGroups;
  int order=1;
  int dimension=2;
  dgAlgorithm algo;
  function::registerAllFunctions();
  algo.buildGroups(GModel::current(),dimension,order,elementGroups,faceGroups,boundaryGroups);

  //for now, we suppose there is only one group of elements
  int nbNodes = elementGroups[0]->getNbNodes();
  fullMatrix<double> sol(nbNodes,elementGroups[0]->getNbElements());
  fullMatrix<double> residu(nbNodes,elementGroups[0]->getNbElements());
  // initial condition
  {
    dgConservationLawInitialCondition initLaw;
    algo.residualVolume(initLaw,*elementGroups[0],sol,residu);
    algo.multAddInverseMassMatrix(*elementGroups[0],residu,sol);
  }
  print("init.pos",*elementGroups[0],&sol(0,0));
  //advection
  dgConservationLaw *law = dgNewConservationLawAdvection();
  for(int iT=0; iT<1000; iT++) {
    algo.residual(*law,elementGroups,faceGroups,boundaryGroups,sol,residu);
    residu.scale(0.01);
    algo.multAddInverseMassMatrix(*elementGroups[0],residu,sol);
    if(iT%10==0){
      char name[100];
      sprintf(name,"test_%05i.pos",iT/10);
      printf("%i\n",iT);
      print(name,*elementGroups[0],&sol(0,0));
    }
  }
  delete law;
}

void print (const char *filename,const dgGroupOfElements &els, double *v) {
  FILE *file = fopen(filename,"w");
  fprintf(file,"View \"%s\" {\n", filename);
  for(int iel=0;iel<els.getNbElements();iel++){
    MElement *el = els.getElement(iel);
    fprintf(file,"ST (");
    for (int iv=0; iv<el->getNumVertices(); iv++) {
      MVertex *vertex = el->getVertex(iv);
      SPoint3 coord = vertex->point();
      fprintf(file,"%e, %e, %e%c ",coord.x(),coord.y(),coord.z(),iv==el->getNumVertices()-1?')':',');
    }
    fprintf(file,"{");
    for (int iv=0; iv<el->getNumVertices(); iv++)
      fprintf(file,"%e%c ",*(v++),iv==el->getNumVertices()-1?'}':',');
    fprintf(file,";\n");
  }
  fprintf(file,"};");
  fclose(file);
}
