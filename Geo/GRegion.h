#ifndef _GREGION_H_
#define _GREGION_H_

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

#include "GEntity.h"
#include "MElement.h"

// A model region.
class GRegion : public GEntity {
 protected:
  std::list<GFace *> l_faces;
  std::list<int> l_dirs;

 public:
  GRegion(GModel *model, int tag) : GEntity (model, tag) {}
  virtual ~GRegion();

  virtual int dim() const {return 3;}
  virtual void setVisibility(char val, bool recursive=false);

  // The bounding box
  virtual SBoundingBox3d bounds() const; 

  // Recompute the mesh partitions defined on this region.
  void recomputeMeshPartitions();

  std::vector<MTetrahedron*> tetrahedra;
  std::vector<MHexahedron*> hexahedra;
  std::vector<MPrism*> prisms;
  std::vector<MPyramid*> pyramids;
};

#endif
