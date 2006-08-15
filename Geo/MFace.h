#ifndef _MFACE_H_
#define _MFACE_H_

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

#include "MVertex.h"
#include "SVector3.h"
#include "Numeric.h"

class MFace {
 private:
  MVertex *_v[4];
 public:
  MFace(MVertex *v0, MVertex *v1, MVertex *v2, MVertex *v3=0) 
  {
    _v[0] = v0; _v[1] = v1; _v[2] = v2; _v[3] = v3;
  }
  inline int getNumVertices() const { return _v[3] ? 4 : 3; }
  inline MVertex *getVertex(int i) const { return _v[i]; }
  SVector3 normal()
  {
    double n[3];
    normal3points(_v[0]->x(), _v[0]->y(), _v[0]->z(),
		  _v[1]->x(), _v[1]->y(), _v[1]->z(),
		  _v[2]->x(), _v[2]->y(), _v[2]->z(), n);
    return SVector3(n[0], n[1], n[2]);
  }
};

#endif
