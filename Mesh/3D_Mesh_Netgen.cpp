// $Id: 3D_Mesh_Netgen.cpp,v 1.3 2004-06-27 00:20:25 geuzaine Exp $
//
// Copyright (C) 1997-2004 C. Geuzaine, J.-F. Remacle
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
//
// Contributor(s):
//   Nicolas Tardieu
//

#include "Gmsh.h"
#include "Mesh.h"
#include "Numeric.h"
#include "Context.h"

extern Context_T CTX;
extern Mesh *THEM;

#if !defined(HAVE_NETGEN)

int Mesh_Netgen(Volume * v)
{
  if(CTX.mesh.algo3d == FRONTAL_NETGEN)
    Msg(GERROR, "Netgen is not compiled in this version of Gmsh");
  return 0;
}

void Optimize_Netgen(Volume * v)
{
  Msg(GERROR, "Netgen is not compiled in this version of Gmsh");
}

#else

extern "C"
{
#include "nglib.h"
}

class Netgen{
 private:
  List_T *_vertices;
  Volume *_vol;
  Ng_Mesh *_ngmesh;
 public:
  Netgen(Volume *vol, int importVolumeMesh = 0);
  Netgen(Surface *s, int importSurfaceMesh = 0);
  ~Netgen();
  void MeshVolume();
  void OptimizeVolume();
};

Netgen::Netgen(Volume *vol, int importVolumeMesh)
  : _vol(vol)
{
  // creates Netgen mesh structure
  Ng_Init();
  _ngmesh = Ng_NewMesh();

  if(importVolumeMesh){
    _vertices = Tree2List(_vol->Vertices);
  }
  else{
    // Transfert all surface vertices we must *not* add 2 times the
    // same vertex (the same vertex can belong to several surfaces)
    _vertices = List_Create(100, 100, sizeof(Vertex*));
    for(int i = 0; i < List_Nbr(_vol->Surfaces); i++) {
      Surface *s;
      List_Read(_vol->Surfaces, i, &s);
      List_T *vlist = Tree2List(s->Vertices);
      for(int j = 0; j < List_Nbr(vlist); j++) 
	List_Insert(_vertices, List_Pointer(vlist, j), compareVertex);
      List_Delete(vlist);
    }
  }
  List_Sort(_vertices, compareVertex);

  // Transfer the vertices
  for(int i = 0; i < List_Nbr(_vertices); i++){
    Vertex *v;
    List_Read(_vertices, i, &v);
    double tmp[3];
    tmp[0] = v->Pos.X;
    tmp[1] = v->Pos.Y;
    tmp[2] = v->Pos.Z;
    Ng_AddPoint(_ngmesh, tmp);
  }

  // Transfert all surface simplices
  for(int i = 0; i < List_Nbr(_vol->Surfaces); i++) {
    Surface *s;
    List_Read(_vol->Surfaces, i, &s);
    int sign;
    List_Read(_vol->SurfacesOrientations, i, &sign);
    List_T *simplist = Tree2List(s->Simplexes);
    for(int j = 0; j < List_Nbr(simplist); j++) {
      Simplex *simp;
      List_Read(simplist, j, &simp);
      int tmp[3], index[3];
      if(sign > 0){
	index[0] = 0;
	index[1] = 1;
	index[2] = 2;
      }
      else{
	index[0] = 0;
	index[1] = 2;
	index[2] = 1;
      }
      tmp[0] = 1 + List_ISearch(_vertices, &simp->V[index[0]], compareVertex);
      tmp[1] = 1 + List_ISearch(_vertices, &simp->V[index[1]], compareVertex);
      tmp[2] = 1 + List_ISearch(_vertices, &simp->V[index[2]], compareVertex);
      Ng_AddSurfaceElement(_ngmesh, NG_TRIG, tmp);
    }
    List_Delete(simplist);
  }
  
  // Transfer the volume elements
  if(importVolumeMesh){
    List_T *simplist = Tree2List(_vol->Simplexes);
    for(int i = 0; i < List_Nbr(simplist); i++) {
      Simplex *simp;
      List_Read(simplist, i, &simp);
      int tmp[4];
      tmp[0] = 1 + List_ISearch(_vertices, &simp->V[0], compareVertex);
      tmp[1] = 1 + List_ISearch(_vertices, &simp->V[1], compareVertex);
      tmp[2] = 1 + List_ISearch(_vertices, &simp->V[2], compareVertex);
      tmp[3] = 1 + List_ISearch(_vertices, &simp->V[3], compareVertex);
      Ng_AddVolumeElement(_ngmesh, NG_TET, tmp);
    }
    List_Delete(simplist);    
  }
}

Netgen::Netgen(Surface *sur, int importSurfaceMesh)
{
  // todo
}

Netgen::~Netgen()
{
  List_Delete(_vertices);
  Ng_DeleteMesh(_ngmesh);
  Ng_Exit();
}

void Netgen::MeshVolume()
{
  Ng_Meshing_Parameters mp;
  mp.maxh = 1;
  mp.fineness = 1;
  mp.secondorder = 0;
  Ng_GenerateVolumeMesh(_ngmesh, &mp);

  // Gets total number of vertices of Netgen's mesh
  int nbv = Ng_GetNP(_ngmesh);
  Vertex **vtable = (Vertex **)Malloc(nbv * sizeof(Vertex*));
  
  // Get existing vertices
  for(int i = 0; i < List_Nbr(_vertices); i++)
    List_Read(_vertices, i, &vtable[i]);

  // Create new volume vertices
  for(int i = List_Nbr(_vertices); i < nbv; i++) {
    double tmp[3];
    Ng_GetPoint(_ngmesh, i+1, tmp);
    vtable[i] = Create_Vertex(++(THEM->MaxPointNum), tmp[0], tmp[1], tmp[2], 1., 0);
    Tree_Add(_vol->Vertices, &vtable[i]);
    Tree_Add(THEM->Vertices, &vtable[i]);
  }

  // Get total number of simplices of Netgen's mesh
  int nbe = Ng_GetNE(_ngmesh);

  // Create new volume simplices
  for(int i = 0; i < nbe; i++) {
    int tmp[4];
    Ng_GetVolumeElement(_ngmesh, i+1, tmp);
    Simplex *simp = Create_Simplex(vtable[tmp[0]-1], vtable[tmp[1]-1],
				   vtable[tmp[2]-1], vtable[tmp[3]-1]);
    simp->iEnt = _vol->Num;
    Tree_Add(_vol->Simplexes, &simp);
  }
  
  Free(vtable);
}

void Netgen::OptimizeVolume()
{
  // remove the pure volume vertices from THEM->Vertices and v->Vertices
  // remove the tets from THEM->Simplexes
  // reset v->Simplexes
  //RemoveIllegalElements(*_ngmesh);
  //OptimizeVolume(mparam, *_ngmesh, NULL);
  // transfer vertices and tets back into v and THEM
}

int Mesh_Netgen(Volume * v)
{
  if(CTX.mesh.algo3d != FRONTAL_NETGEN)
    return 0;

  if(THEM->BGM.Typ == ONFILE){
    Msg(GERROR, "Netgen is not ready to be used with a background mesh");
    return 0;
  }

  Msg(STATUS3, "Meshing volume %d", v->Num);
  Netgen ng(v);
  ng.MeshVolume();
  
  return 1;
}

void Optimize_Netgen(Volume * v)
{
  Msg(STATUS3, "Optimizing volume %d", v->Num);
  Netgen ng(v, 1);
  ng.OptimizeVolume();
}

#endif // !HAVE_NETGEN
