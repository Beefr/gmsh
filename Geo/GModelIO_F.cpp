#include <string>
#include "GModel.h"
#include "Message.h"
#include "FVertex.h"
#include "FEdge.h"
#include "FFace.h"
#include "GModelIO_F.h"

#if defined(HAVE_FOURIER_MODEL)

#include "FM_FPatch.h"
#include "FM_FCurve.h"
#include "FM_TopoVertex.h"
#include "FM_TopoEdge.h"
#include "FM_TopoFace.h"
#include "FM_Reader.h"

extern GModel *GMODEL;

void makeGFace(FM::Patch* patch)
{
  double LL[2], LR[2], UL[2], UR[2];
  LL[0] = 0.0; LL[1] = 0.0;
  LR[0] = 1.0; LR[1] = 0.0;
  UL[0] = 0.0; UL[1] = 1.0;
  UR[0] = 1.0; UR[1] = 1.0;
  
  int i1, i2;
  double xx,yy,zz;
  
  int tagVertex = GMODEL->numVertex();
  patch->F(LL[0],LL[1],xx,yy,zz);
  FM::TopoVertex* vLL = new FM::TopoVertex(++tagVertex,xx,yy,zz);
  GMODEL->add(new FVertex(GMODEL,vLL->GetTag(),vLL));
  patch->F(LR[0],LR[1],xx,yy,zz);
  FM::TopoVertex* vLR = new FM::TopoVertex(++tagVertex,xx,yy,zz);
  GMODEL->add(new FVertex(GMODEL,vLR->GetTag(),vLR));
  patch->F(UL[0],UL[1],xx,yy,zz);
  FM::TopoVertex* vUL = new FM::TopoVertex(++tagVertex,xx,yy,zz);
  GMODEL->add(new FVertex(GMODEL,vUL->GetTag(),vUL));
  patch->F(UR[0],UR[1],xx,yy,zz);
  FM::TopoVertex* vUR = new FM::TopoVertex(++tagVertex,xx,yy,zz);
  GMODEL->add(new FVertex(GMODEL,vUR->GetTag(),vUR));
  
  FM::Curve* curveB = new FM::FCurve(0,patch,LL,LR);
  FM::Curve* curveR = new FM::FCurve(0,patch,LR,UR);
  FM::Curve* curveT = new FM::FCurve(0,patch,UR,UL);
  FM::Curve* curveL = new FM::FCurve(0,patch,UL,LL);
  
  int tagEdge = GMODEL->numEdge();
  FM::TopoEdge* eB = new FM::TopoEdge(++tagEdge,curveB,vLL,vLR);
  i1 = eB->GetStartPoint()->GetTag();
  i2 = eB->GetEndPoint()->GetTag();
  GMODEL->add(new FEdge(GMODEL,eB,eB->GetTag(),GMODEL->vertexByTag(i1),
			GMODEL->vertexByTag(i2)));
  FM::TopoEdge* eR = new FM::TopoEdge(++tagEdge,curveR,vLR,vUR); 
  i1 = eR->GetStartPoint()->GetTag();
  i2 = eR->GetEndPoint()->GetTag();
  GMODEL->add(new FEdge(GMODEL,eR,eR->GetTag(),GMODEL->vertexByTag(i1),
			GMODEL->vertexByTag(i2))); 
  FM::TopoEdge* eT = new FM::TopoEdge(++tagEdge,curveT,vUR,vUL);
  i1 = eT->GetStartPoint()->GetTag();
  i2 = eT->GetEndPoint()->GetTag();
  GMODEL->add(new FEdge(GMODEL,eT,eT->GetTag(),GMODEL->vertexByTag(i1),
			GMODEL->vertexByTag(i2)));
  FM::TopoEdge* eL = new FM::TopoEdge(++tagEdge,curveL,vUL,vLL); 
  i1 = eL->GetStartPoint()->GetTag();
  i2 = eL->GetEndPoint()->GetTag();
  GMODEL->add(new FEdge(GMODEL,eL,eL->GetTag(),GMODEL->vertexByTag(i1),
			GMODEL->vertexByTag(i2)));
  
  FM::TopoFace* face = new FM::TopoFace(GMODEL->numFace() + 1,patch);
  face->AddEdge(eB); face->AddEdge(eR); 
  face->AddEdge(eT); face->AddEdge(eL);
  std::list<GEdge*> l_edges;
  for (int j=0;j<face->GetNumEdges();j++) {
    int tag = face->GetEdge(j)->GetTag(); 
    l_edges.push_back(GMODEL->edgeByTag(tag));
  }
  GMODEL->add(new FFace(GMODEL,face,face->GetTag(),l_edges));
}

int GModel::readF(const std::string &filename)
{
  FM::Reader* reader = new FM::Reader(filename.c_str());
  for (int i = 0; i < reader->GetNumPatches(); i++)
    makeGFace(reader->GetPatch(i));

  return 1;
}

#else

int GModel::readF(const std::string &fn)
{
  Msg(GERROR,"Gmsh has to be compiled with Fourier Model support to load %s",
      fn.c_str());
  return 0;
}

#endif
