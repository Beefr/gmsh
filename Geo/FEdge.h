#ifndef _F_EDGE_H_
#define _F_EDGE_H_

#include "GEdge.h"
#include "GModel.h"
#include "FVertex.h"
#include "Range.h"

#if defined(HAVE_FOURIER_MODEL)

#include "FM_Edge.h"

class FEdge : public GEdge {
 protected:
  FM_Edge* edge;
 public:
  FEdge(GModel *model, FM_Edge* edge_, int tag, GVertex *v1, GVertex *v2) : 
    GEdge(model, tag, v1, v2), edge(edge_) {}
  virtual ~FEdge() {}
  double period() const { throw ; }
  virtual bool periodic(int dim=0) const { return false; }
  virtual Range<double> parBounds(int i) const;
  virtual GeomType geomType() const { throw; }
  virtual bool degenerate(int) const { return false; }
  virtual bool continuous(int dim) const { return true; }
  virtual GPoint point(double p) const;
  virtual GPoint closestPoint(const SPoint3 & queryPoint) { throw; }
  virtual int containsPoint(const SPoint3 &pt) const { throw; }
  virtual int containsParam(double pt) const { throw; }
  virtual SVector3 firstDer(double par) const { throw; }
  virtual SPoint2 reparamOnFace(GFace *face, double epar, int dir) const 
  { throw; }
  virtual double parFromPoint(const SPoint3 &pt) const;
  virtual int minimumMeshSegments () const { throw; }
  virtual int minimumDrawSegments () const { throw; }
  ModelType getNativeType() const { return FourierModel; }
};

#endif

#endif
