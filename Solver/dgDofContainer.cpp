#include "GmshConfig.h"
#include "dgDofContainer.h"
#include "function.h"
#include "dgGroupOfElements.h"
#include "dgConservationLaw.h"
#include "dgAlgorithm.h"
#ifdef HAVE_MPI
#include "mpi.h"
#else
#include "string.h"
#endif
#include <sstream>
#include "MElement.h"
dgDofContainer::dgDofContainer (dgGroupCollection *groups, int nbFields):
  _groups(*groups), _mshStep(0)
{
  int _dataSize = 0;
  _dataSizeGhost=0;
  _totalNbElements = 0;
  _parallelStructureExists = false;
  _nbFields = nbFields;
  for (int i=0; i< _groups.getNbElementGroups();i++){
    int nbNodes    = _groups.getElementGroup(i)->getNbNodes();
    int nbElements = _groups.getElementGroup(i)->getNbElements();
    _totalNbElements +=nbElements;
    _dataSize += nbNodes*_nbFields*nbElements;
  }

  // allocate the big vectors
  _data      = new fullVector<double>(_dataSize);
  // create proxys for each group
  int offset = 0;
  _dataProxys.resize(_groups.getNbElementGroups()+_groups.getNbGhostGroups());
  for (int i=0;i<_groups.getNbElementGroups();i++){    
    dgGroupOfElements *group = _groups.getElementGroup(i);
    _groupId[group] = i;
    int nbNodes    = group->getNbNodes();
    int nbElements = group->getNbElements();
    _dataProxys[i] = new fullMatrix<double> (&(*_data)(offset),nbNodes, _nbFields*nbElements);
    offset += nbNodes*_nbFields*nbElements;
  }  

  //ghosts
  
  int totalNbElementsGhost =0;
  for (int i=0; i<_groups.getNbGhostGroups(); i++){
    int nbNodes    = _groups.getGhostGroup(i)->getNbNodes();
    int nbElements = _groups.getGhostGroup(i)->getNbElements();
    totalNbElementsGhost +=nbElements;
    _dataSizeGhost += nbNodes*_nbFields*nbElements;
  }

  _dataProxys.resize(_groups.getNbElementGroups()+_groups.getNbGhostGroups());
  _ghostData = new fullVector<double>(_dataSizeGhost);
  offset=0;
  for (int i=0;i<_groups.getNbGhostGroups();i++){    
    dgGroupOfElements *group = _groups.getGhostGroup(i);
    int nbNodes    = group->getNbNodes();
    int nbElements = group->getNbElements();
    _groupId[group] = i+_groups.getNbElementGroups();
    _dataProxys[i+_groups.getNbElementGroups()] = new fullMatrix<double> (&(*_ghostData)(offset),nbNodes, _nbFields*nbElements);
    offset += nbNodes*_nbFields*nbElements;
  }  

}

dgDofContainer::~dgDofContainer (){
  if(_parallelStructureExists) {
    delete []countSend;
    delete []countRecv;
    delete []shiftSend;
    delete []shiftRecv;
    delete []groupShiftRecv;
    delete []sendBuf;
    delete []recvBuf;
  }
  for (int i=0;i<_dataProxys.size();++i) delete _dataProxys[i];
  delete _data;
  delete _ghostData;
}

void dgDofContainer::buildParallelStructure(){
  if (_parallelStructureExists)
    return;

  // MPI all2all buffers
  countSend = new int[Msg::GetCommSize()];
  shiftSend = new int[Msg::GetCommSize()];
  countRecv = new int[Msg::GetCommSize()];
  shiftRecv = new int[Msg::GetCommSize()];
  groupShiftRecv = new int[_groups.getNbGhostGroups()];
  for(int i=0;i<Msg::GetCommSize();i++){
    countSend[i]=0;
    countRecv[i]=0;
    for(size_t j=0;j<_groups.getNbImageElementsOnPartition(i);j++){
      countSend[i] += _groups.getElementGroup(_groups.getImageElementGroup(i,j))->getNbNodes()*_nbFields;
    }
  }
  for(size_t i=0; i<_groups.getNbGhostGroups(); i++){
    dgGroupOfElements *group = _groups.getGhostGroup(i);
    groupShiftRecv[i] = countRecv[group->getGhostPartition()];
    countRecv[group->getGhostPartition()]+=group->getNbElements()*group->getNbNodes()*_nbFields;
  }
  shiftSend[0] = shiftRecv[0]=0;
  int totalSend = countSend[0];
  int totalRecv = countRecv[0];
  for(size_t i=1; i<Msg::GetCommSize(); i++){
    shiftSend[i] = shiftSend[i-1]+countSend[i-1];
    shiftRecv[i] = shiftRecv[i-1]+countRecv[i-1];
    totalSend += countSend[i];
    totalRecv += countRecv[i];
  }
  for(size_t i=0; i<_groups.getNbGhostGroups(); i++){
    dgGroupOfElements *group = _groups.getGhostGroup(i);
    groupShiftRecv[i] += shiftRecv[group->getGhostPartition()];
  }
  sendBuf = new double[totalSend];
  recvBuf = new double[totalRecv];

  _parallelStructureExists = true;
}

void dgDofContainer::scatter() {
  if(!_parallelStructureExists)
    buildParallelStructure();
  //1) fill
  int index=0;
  for(int i=0;i<Msg::GetCommSize();i++){
    for(size_t j=0;j<_groups.getNbImageElementsOnPartition(i);j++){
      fullMatrix<double> &sol = getGroupProxy(_groups.getImageElementGroup(i,j));
      int eid = _groups.getImageElementPositionInGroup(i,j);
      for(int l=0;l<_nbFields;l++) {
        for(int k=0;k<sol.size1();k++) {
          sendBuf[index++] = sol(k, eid *_nbFields+l);
        }
      }
    }
  }
  //2) send
  #ifdef HAVE_MPI
  MPI_Alltoallv(sendBuf,countSend,shiftSend,MPI_DOUBLE,recvBuf,countRecv,shiftRecv,MPI_DOUBLE,MPI_COMM_WORLD);
  #else
  memcpy(recvBuf,sendBuf,countSend[0]*sizeof(double));
  #endif
  //3) distribute
  for(int i=0; i< _groups.getNbGhostGroups();i++) {
    fullMatrix<double> &sol = getGroupProxy(i+_groups.getNbElementGroups());
    fullMatrix<double> recvProxy (recvBuf + groupShiftRecv[i], sol.size1(), sol.size2());
    sol.setAll(recvProxy);
  }
}
void dgDofContainer::setAll(double v) {
  for(int i=0;i<_data->size();i++)
    (*_data)(i)=v;
  for(int i=0;i<_ghostData->size();i++)
    (*_ghostData)(i)=v;
}
void dgDofContainer::scale(double f) 
{
  _data->scale(f);
  _ghostData->scale(f); 
}
void dgDofContainer::scale(std::vector<dgGroupOfElements*>groupsVector, double f){
  for(int i=0;i<groupsVector.size();i++){
    dgGroupOfElements *g=groupsVector[i];
    fullMatrix<double> &proxy=getGroupProxy(g);
    proxy.scale(f);
  }
}

void dgDofContainer::axpy(dgDofContainer &x, double a)
{
  _data->axpy(*x._data,a);
  _ghostData->axpy(*x._ghostData,a); 
}
void dgDofContainer::axpy(std::vector<dgGroupOfElements*>groupsVector,dgDofContainer &x, double a){
  for(int i=0;i<groupsVector.size();i++){
    dgGroupOfElements *g=groupsVector[i];
    fullMatrix<double> &proxy=getGroupProxy(g);
    fullMatrix<double> &xProxy=x.getGroupProxy(g);
    proxy.add(xProxy,a);
  }
}

double dgDofContainer::norm() {
  double localNorm = _data->norm();
  #ifdef HAVE_MPI
  double globalNorm;
  MPI_Allreduce(&localNorm, &globalNorm, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
  return globalNorm;
  #else
  return localNorm;
  #endif
}
void dgDofContainer::save(const std::string name) {
  FILE *f = fopen (name.c_str(),"wb");
  _data->binarySave(f);
  fclose(f);
}
void dgDofContainer::load(const std::string name) {
  FILE *f = fopen (name.c_str(),"rb");
  _data->binaryLoad(f);
  fclose(f);
}

void dgDofContainer::L2Projection(std::string functionName){
  dgDofContainer rhs(&_groups, _nbFields);
  for (int iGroup=0;iGroup<_groups.getNbElementGroups();iGroup++) {
    const dgGroupOfElements &group = *_groups.getElementGroup(iGroup);
    fullMatrix<double> Source = fullMatrix<double> (group.getNbIntegrationPoints(),group.getNbElements() * _nbFields);
    dataCacheMap cacheMap;
    cacheMap.setNbEvaluationPoints(group.getNbIntegrationPoints());
    dataCacheElement &cacheElement = cacheMap.getElement();
    cacheMap.provideData("UVW",1,3).set(group.getIntegrationPointsMatrix());
    dataCacheDouble &sourceTerm = cacheMap.get(functionName);
    fullMatrix<double> source;
    for (int iElement=0 ; iElement<group.getNbElements() ;++iElement) {
      cacheElement.set(group.getElement(iElement));
      source.setAsProxy(Source, iElement*_nbFields, _nbFields);
      for (int iPt =0; iPt< group.getNbIntegrationPoints(); iPt++) {
        const double detJ = group.getDetJ (iElement, iPt);
        for (int k=0;k<_nbFields;k++)
          source(iPt,k) = sourceTerm(iPt,k)*detJ;
      }
    }
    rhs.getGroupProxy(iGroup).gemm(group.getSourceRedistributionMatrix(), Source);
    dgAlgorithm::multAddInverseMassMatrix(group, rhs.getGroupProxy(iGroup), getGroupProxy(iGroup));
  }
}

void dgDofContainer::exportGroupIdMsh(){
  // the elementnodedata::export does not work !!

  std::ostringstream name_oss;
  name_oss<<"groups.msh";
  if(Msg::GetCommSize()>1)
    name_oss<<"_"<<Msg::GetCommRank();
  FILE *f = fopen (name_oss.str().c_str(),"w");
  int COUNT = 0;
  for (int i=0;i < _groups.getNbElementGroups() ;i++){
    COUNT += _groups.getElementGroup(i)->getNbElements();
  }
  fprintf(f,"$MeshFormat\n2.1 0 8\n$EndMeshFormat\n");  
  fprintf(f,"$ElementNodeData\n");
  fprintf(f,"1\n");
  fprintf(f,"\"%s\"\n","GroupsIds");
  fprintf(f,"1\n");
  fprintf(f,"0.0\n");
  fprintf(f,"%d\n", Msg::GetCommSize() > 1 ? 4 : 3);
  fprintf(f,"0\n 1\n %d\n",COUNT);
  if(Msg::GetCommSize() > 1) fprintf(f,"%d\n", Msg::GetCommRank());
  for (int i=0;i < _groups.getNbElementGroups()  ;i++){
    dgGroupOfElements *group = _groups.getElementGroup(i);
    for (int iElement=0 ; iElement< group->getNbElements() ;++iElement) {
      MElement *e = group->getElement(iElement);
      int num = e->getNum();
      fullMatrix<double> sol(getGroupProxy(i), iElement*_nbFields,_nbFields);
      fprintf(f,"%d %d",num,sol.size1());
      for (int k=0;k<sol.size1();++k) {
        fprintf(f," %.16E ",i*1.0);
      }
      fprintf(f,"\n");
    }
  }
  fprintf(f,"$EndElementNodeData\n");
  fclose(f);
  return;
  // we should discuss that : we do a copy of the solution, this should
  // be avoided !

  /*std::map<int, std::vector<double> > data;
  
  for (int i=0;i < _groups.getNbElementGroups() ;i++){
    dgGroupOfElements *group = _groups.getElementGroup(i);
    for (int iElement=0 ; iElement< group->getNbElements() ;++iElement) {
      MElement *e = group->getElement(iElement);
      int num = e->getNum();
      fullMatrix<double> sol(getGroupProxy(i), iElement*_nbFields,_nbFields);
      std::vector<double> val;
      //      val.resize(sol.size2()*sol.size1());
      val.resize(sol.size1());
      int counter = 0;
      //      for (int iC=0;iC<sol.size2();iC++)
      printf("%g %g %g\n",sol(0,0),sol(1,0),sol(2,0));
      for (int iR=0;iR<sol.size1();iR++)val[counter++] = sol(iR,0);
      data[num] = val;
    }
  }

  PView *pv = new PView (name, "ElementNodeData", _gm, data, 0.0, 1);
  pv->getData()->writeMSH(name+".msh", false); 
  delete pv;
  */
}

void dgDofContainer::exportMsh(const std::string name)
{
  // the elementnodedata::export does not work !!

  for (int ICOMP = 0; ICOMP<_nbFields;++ICOMP){
    std::ostringstream name_oss, name_view;
    name_view<<"comp_"<<ICOMP;
    name_oss<<name<<"_COMP_"<<ICOMP<<".msh";
    if(Msg::GetCommSize()>1)
      name_oss<<"_"<<Msg::GetCommRank();
    FILE *f = fopen (name_oss.str().c_str(),"w");
    int COUNT = 0;
    for (int i=0;i < _groups.getNbElementGroups() ;i++){
      COUNT += _groups.getElementGroup(i)->getNbElements();
    }
    fprintf(f,"$MeshFormat\n2.1 0 8\n$EndMeshFormat\n");  
    fprintf(f,"$ElementNodeData\n");
    fprintf(f,"1\n");
    fprintf(f,"\"%s\"\n",name_view.str().c_str());
    fprintf(f,"1\n");
    fprintf(f,"%d\n", _mshStep); // should print actual time here
    fprintf(f,"%d\n", Msg::GetCommSize() > 1 ? 4 : 3);
    fprintf(f,"%d\n 1\n %d\n", _mshStep, COUNT);
    if(Msg::GetCommSize() > 1) fprintf(f,"%d\n", Msg::GetCommRank());
    for (int i=0;i < _groups.getNbElementGroups()  ;i++){
      dgGroupOfElements *group = _groups.getElementGroup(i);
      for (int iElement=0 ; iElement< group->getNbElements() ;++iElement) {
        MElement *e = group->getElement(iElement);
        int num = e->getNum();
        fullMatrix<double> sol(getGroupProxy(i), iElement*_nbFields,_nbFields);
        fprintf(f,"%d %d",num,sol.size1());
        for (int k=0;k<sol.size1();++k) {
          fprintf(f," %.16E ",sol(k,ICOMP));
        }
        fprintf(f,"\n");
      }
    }
    fprintf(f,"$EndElementNodeData\n");
    fclose(f);
  }
  
  _mshStep++;

  // we should discuss that : we do a copy of the solution, this should
  // be avoided !

  /*std::map<int, std::vector<double> > data;
  
  for (int i=0;i < _groups.getNbElementGroups() ;i++){
    dgGroupOfElements *group = _groups.getElementGroup(i);
    for (int iElement=0 ; iElement< group->getNbElements() ;++iElement) {
      MElement *e = group->getElement(iElement);
      int num = e->getNum();
      fullMatrix<double> sol(getGroupProxy(i), iElement*_nbFields,_nbFields);
      std::vector<double> val;
      //      val.resize(sol.size2()*sol.size1());
      val.resize(sol.size1());
      int counter = 0;
      //      for (int iC=0;iC<sol.size2();iC++)
      printf("%g %g %g\n",sol(0,0),sol(1,0),sol(2,0));
      for (int iR=0;iR<sol.size1();iR++)val[counter++] = sol(iR,0);
      data[num] = val;
    }
  }

  PView *pv = new PView (name, "ElementNodeData", _gm, data, 0.0, 1);
  pv->getData()->writeMSH(name+".msh", false); 
  delete pv;
  */
}


#include "LuaBindings.h"
void dgDofContainer::registerBindings(binding *b){
  classBinding *cb = b->addClass<dgDofContainer>("dgDofContainer");
  cb->setDescription("The DofContainer class provides a vector containing the degree of freedoms");
  methodBinding *cm;
  cm = cb->setConstructor<dgDofContainer,dgGroupCollection*,int>();
  cm->setDescription("Build a vector");
  cm->setArgNames("GroupCollection","nbFields",NULL);
  cm = cb->addMethod("L2Projection",&dgDofContainer::L2Projection);
  cm->setDescription("Project a function onto this vector");
  cm->setArgNames("functionName",NULL);
  cm = cb->addMethod("exportMsh",&dgDofContainer::exportMsh);
  cm->setDescription("Export the dof for gmsh visualization");
  cm->setArgNames("filename", NULL);
  cm = cb->addMethod("exportGroupIdMsh",&dgDofContainer::exportGroupIdMsh);
  cm->setDescription("Export the group ids for gmsh visualization");
  cm = cb->addMethod("norm",&dgDofContainer::norm);
  cm->setDescription("Returns the norm of the vector");
}
