#ifndef _GMSH_VERTEX_H_
#define _GMSH_VERTEX_H_

#include "Mesh.h"
#include "gmshModel.h"
#include "GVertex.h"

class gmshVertex : public GVertex {
 public:
  gmshVertex(GModel *m, Vertex *_v) : GVertex(m, _v->Num), v(_v){}
  virtual ~gmshVertex() {}
  virtual GPoint point() const{return GPoint ( v->Pos.X,v->Pos.Y,v->Pos.Z,this);}
  virtual double x() const {return v->Pos.X;}
  virtual double y() const {return v->Pos.Y;}
  virtual double z() const {return v->Pos.Z;}
  virtual double tolerance() const {return 1.e-14;}
  void * getNativePtr() const {return v;}
  virtual double prescribedMeshSizeAtVertex () const {return v->lc;}
 protected:
  Vertex *v;
};

#endif
