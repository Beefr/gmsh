#ifndef _MESH_GREGION_H_
#define _MESH_GREGION_H_

// Copyright (C) 1997-2008 C. Geuzaine, J.-F. Remacle
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

#include <vector>
#include <map>

class GModel;
class GRegion;
class GFace;
class GEdge;
class MVertex;
class MLine;
class MTriangle;
// Create the mesh of the region
class meshGRegion {
 public :
  std::vector<GRegion*> &delaunay;
  meshGRegion(std::vector<GRegion*> &d) : delaunay(d) {}
  void operator () (GRegion *);
};

class meshGRegionExtruded {
 public :
  void operator () (GRegion *);
};

// Optimize the mesh of the region using gmsh's algo
class optimizeMeshGRegionGmsh {
 public :
  void operator () (GRegion *);
};

// Optimize the mesh of the region using netgen's algo
class optimizeMeshGRegionNetgen {
 public :
  void operator () (GRegion *);
};

// destroy the mesh of the region
class deMeshGRegion {
 public :
  void operator () (GRegion *);
};


void MeshDelaunayVolume(std::vector<GRegion*> &delaunay);
int MeshTransfiniteVolume(GRegion *gr);
int SubdivideExtrudedMesh(GModel *m);
void carveHole(GRegion *gr, int num, double distance, std::vector<int> &surfaces);

typedef std::multimap<MVertex*,std::pair<MTriangle*,GFace*> > fs_cont ;
typedef std::multimap<MVertex*,std::pair<MLine*,GEdge*> > es_cont ;
GFace* findInFaceSearchStructure ( MVertex *p1,MVertex *p2,MVertex *p3, const fs_cont&search );
GEdge* findInEdgeSearchStructure ( MVertex *p1,MVertex *p2, const es_cont&search );
bool buildFaceSearchStructure ( GModel *model , fs_cont&search );
bool buildEdgeSearchStructure ( GModel *model , es_cont&search );

// adapt the mesh of a region
class adaptMeshGRegion {
  es_cont *_es; fs_cont *_fs;
 public :
  void operator () (GRegion *);
};

#endif
