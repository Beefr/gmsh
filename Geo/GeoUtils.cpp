// $Id: GeoUtils.cpp,v 1.1 2004-02-28 00:48:49 geuzaine Exp $
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

#include "Gmsh.h"
#include "Geo.h"
#include "CAD.h"
#include "Mesh.h"

extern Mesh *THEM;

// A small function to sort the edges in an EdgeLoop. Without this
// sort, it is very difficult to write general scriptable surface
// generation in complex cases

void sortEdgesInLoop(int num, List_T *edges)
{
  Curve *c, *c0, *c1, *c2;
  int nbEdges = List_Nbr(edges);
  List_T *temp = List_Create(nbEdges, 1, sizeof(Curve *));

  for(int i = 0; i < nbEdges; i++) {
    int j;
    List_Read(edges, i, &j);
    if((c = FindCurve(j, THEM)))
      List_Add(temp, &c);
    else
      Msg(GERROR, "Unknown Curve %d in Line Loop %d", j, num);
  }
  List_Reset(edges);
  
  int j = 0, k = 0;
  c0 = c1 = *(Curve **) List_Pointer(temp, 0);
  List_Add(edges, &c1->Num);
  List_PSuppress(temp, 0);
  while(List_Nbr(edges) < nbEdges) {
    for(int i = 0; i < List_Nbr(temp); i++) {
      c2 = *(Curve **) List_Pointer(temp, i);
      if(c1->end == c2->beg) {
	List_Add(edges, &c2->Num);
	List_PSuppress(temp, i);
	c1 = c2;
	if(c2->end == c0->beg) {
	  if(List_Nbr(temp)) {
	    Msg(INFO, "Starting subloop %d in Line Loop %d (are you sure about this?)",
		++k, num);
	    c0 = c1 = *(Curve **) List_Pointer(temp, 0);
	    List_Add(edges, &c1->Num);
	    List_PSuppress(temp, 0);
	  }
	}
	break;
      }
    }
    if(j++ > nbEdges) {
      Msg(GERROR, "Line Loop %d is wrong", num);
      break;
    }
  }
  List_Delete(temp);
}

// Fills in the generatrices for a given surface, given the indices of
// edge loops

void setSurfaceGeneratrices(Surface *s, List_T *loops)
{
  int nbLoop = List_Nbr(loops);
  s->Generatrices = List_Create(4, 4, sizeof(Curve *));
  for(int i = 0; i < nbLoop; i++) {
    int iLoop;
    List_Read(loops, i, &iLoop);
    EdgeLoop *el;
    if(!(el = FindEdgeLoop(iLoop, THEM))) {
      Msg(GERROR, "Unknown Line Loop %d", iLoop);
      List_Delete(s->Generatrices);
      return;
    }
    else {
      for(int j = 0; j < List_Nbr(el->Curves); j++) {
	int ic;
        List_Read(el->Curves, j, &ic);
	Curve *c;
        if(!(c = FindCurve(ic, THEM))) {
          Msg(GERROR, "Unknown Curve %d", ic);
          List_Delete(s->Generatrices);
          return;
        }
        else
          List_Add(s->Generatrices, &c);
      }
    }
  }
}

// Fills in the boundary of a volume, given the indices of surface
// loops

void setVolumeSurfaces(Volume *v, List_T * loops)
{
  v->Surfaces = List_Create(4, 1, sizeof(Surface *));
  for(int i = 0; i < List_Nbr(loops); i++) {
    int il;
    List_Read(loops, i, &il);
    SurfaceLoop *sl;
    if(!(sl = FindSurfaceLoop(il, THEM))) {
      Msg(GERROR, "Unknown Surface Loop %d", il);
      List_Delete(v->Surfaces);
      return;
    }
    else {
      for(int j = 0; j < List_Nbr(sl->Surfaces); j++) {
	int is;
        List_Read(sl->Surfaces, j, &is);
	Surface *s;
        if(!(s = FindSurface(abs(is), THEM))) {
          Msg(GERROR, "Unknown Surface %d", is);
          List_Delete(v->Surfaces);
          return;
        }
        else
          List_Add(v->Surfaces, &s);
      }
    }
  }
}
