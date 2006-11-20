#ifndef _MVERTEX_H_
#define _MVERTEX_H_

// Copyright (C) 1997-2006 C. Geuzaine, J.-F. Remacle
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA.
// 
// Please report all bugs and problems to <gmsh@geuz.org>.

#include <stdio.h>
#include <algorithm>
#include "SPoint3.h"

class GEntity;

// A mesh vertex.
class MVertex{
 private:
  static int _globalNum;
  int _num;
  char _visible;
  double _x, _y, _z;
  GEntity *_ge;

 public :
  MVertex(double x, double y, double z, GEntity *ge=0, int num=0) 
    : _visible(true), _x(x), _y(y), _z(z), _ge(ge)
  {
    if(num){
      _num = num;
      _globalNum = std::max(_globalNum, _num);
    }
    else{
      _num = ++_globalNum;
    }
  }
  virtual ~MVertex(){}

  // reset the global node number
  static void resetGlobalNumber(){ _globalNum = 0; }

  // get/set the visibility flag
  virtual char getVisibility(){ return _visible; }
  virtual void setVisibility(char val){ _visible = val; }
  
  // get/set the coordinates
  inline double x() const {return _x;}
  inline double y() const {return _y;}
  inline double z() const {return _z;}
  inline double & x() {return _x;}
  inline double & y() {return _y;}
  inline double & z() {return _z;}
  inline SPoint3 point() { return SPoint3(_x, _y, _z); }

  // get/set the parent entity
  inline GEntity* onWhat() const {return _ge;}
  inline void setEntity(GEntity *ge) { _ge = ge; }

  // get/set the number
  inline int getNum() const {return _num;}
  inline void setNum(int num) { _num = num; }

  // get/set ith parameter
  virtual bool getParameter(int i, double &par){ return false; }
  virtual bool setParameter(int i, double par){ return false; }

  // measure distance to another vertex
  double distance(MVertex *v)
  {
    double dx = _x - v->x();
    double dy = _y - v->y();
    double dz = _z - v->z();
    return sqrt(dx * dx + dy * dy + dz * dz);
  }

  // Get the data associated with this vertex
  virtual void *getData(){ return 0; }

  // IO routines
  void writeMSH(FILE *fp, bool binary=false, double scalingFactor=1.0);
  void writeMSH(FILE *fp, double version, bool binary, int num, 
		int elementary, int physical);
  void writeVRML(FILE *fp, double scalingFactor=1.0);
  void writeUNV(FILE *fp, double scalingFactor=1.0);
  void writeMESH(FILE *fp, double scalingFactor=1.0);
  void writeBDF(FILE *fp, int format=0, double scalingFactor=1.0);
};

class MEdgeVertex : public MVertex{
 protected:
  double _u;
 public :
  MEdgeVertex(double x, double y, double z, GEntity *ge, double u) 
    : MVertex(x, y, z, ge), _u(u)
  {
  }
  ~MEdgeVertex(){}
  virtual bool getParameter(int i, double &par){ par = _u; return true; }
  virtual bool setParameter(int i, double par){ _u = par; return true; }
};

class MSeamEdgeVertex : public MEdgeVertex{
 private:
  MEdgeVertex *_ev;
 public :
  MSeamEdgeVertex(MEdgeVertex* ev) 
    : MEdgeVertex(ev->x(), ev->y(), ev->z(), ev->onWhat(),0), _ev(ev)
    {
      _ev->getParameter(0, _u);
    }
  ~MSeamEdgeVertex(){}
};


class MFaceVertex : public MVertex{
 protected:
  double _u, _v;
 public :
  MFaceVertex(double x, double y, double z, GEntity *ge, double u, double v) 
    : MVertex(x, y, z, ge), _u(u), _v(v)
  {
  }
  ~MFaceVertex(){}
  virtual bool getParameter(int i, double &par){ par = (i ? _v : _u); return true; }
  virtual bool setParameter(int i, double par){ if(!i) _u = par; else _v = par; return true; }
};

template<class T>
class MDataFaceVertex : public MFaceVertex{
 private:
  T _data;
 public :
  MDataFaceVertex(double x, double y, double z, GEntity *ge, double u, double v, T data) 
    : MFaceVertex(x, y, z, ge, u, v), _data(data)
  {
  }
  ~MDataFaceVertex(){}
  virtual bool getData(T &data){ data = _data; return true; }
  virtual void *getData(){ return (void*)&_data; }
};

class MVertexLessThanLexicographic{
 public:
  static double tolerance;
  bool operator()(const MVertex *v1, const MVertex *v2) const
  {
    if(v1->x() - v2->x() >  tolerance) return true;
    if(v1->x() - v2->x() < -tolerance) return false;
    if(v1->y() - v2->y() >  tolerance) return true;
    if(v1->y() - v2->y() < -tolerance) return false;
    if(v1->z() - v2->z() >  tolerance) return true;
    return false;
  }
};

#endif
