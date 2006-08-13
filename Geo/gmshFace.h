#ifndef _GMSH_FACE_H_
#define _GMSH_FACE_H_

#include "GFace.h"
#include "gmshModel.h"
#include "gmshVertex.h"
#include "Mesh.h"
#include "Range.h"

class gmshFace : public GFace {
 protected:
  Surface *s;

 public:
  gmshFace(GModel *m, Surface *face);
  gmshFace(GModel *m, int num);
  virtual ~gmshFace(){}
  Range<double> parBounds(int i) const; 
  virtual int paramDegeneracies(int dir, double *par) { return 0; }
  
  virtual GPoint point(double par1, double par2) const; 
  virtual GPoint point(const SPoint2 &pt) const; 
  virtual GPoint closestPoint(const SPoint3 & queryPoint) ; 
  
  virtual int containsPoint(const SPoint3 &pt) const;  
  virtual int containsParam(const SPoint2 &pt) const; 
  
  virtual SVector3 normal(const SPoint2 &param) const; 
  virtual Pair<SVector3,SVector3> firstDer(const SPoint2 &param) const; 
  virtual double * nthDerivative(const SPoint2 &param, int n,  
 				 double *array) const {throw;}
  
  virtual GEntity::GeomType geomType() const; 
  virtual int geomDirection() const { return 1; }
  
  virtual bool continuous(int dim) const { return true; }
  virtual bool periodic(int dim) const { return false; }
  virtual bool degenerate(int dim) const { return false; }
  virtual double period(int dir) const {throw;}
  void * getNativePtr() const { return s; }
  virtual bool surfPeriodic(int dim) const {throw;}
  virtual SPoint2 parFromPoint(const SPoint3 &) const;
};

#endif
