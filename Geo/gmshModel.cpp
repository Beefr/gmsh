// $Id: gmshModel.cpp,v 1.20 2006-10-10 00:44:41 geuzaine Exp $
//
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

#include "gmshModel.h"
#include "Mesh.h"
#include "Geo.h"
#include "Tools.h"
#include "Numeric.h"
#include "Message.h"
#include "gmshVertex.h"
#include "gmshFace.h"
#include "gmshEdge.h"
#include "gmshRegion.h"

extern Mesh *THEM;

void gmshModel::import()
{
  if(Tree_Nbr(THEM->Points)) {
    List_T *points = Tree2List(THEM->Points);
    for(int i = 0; i < List_Nbr(points); i++){
      Vertex *p;
      List_Read(points, i, &p);
      GVertex *v = vertexByTag(p->Num);
      if(!v){
	v = new gmshVertex(this, p);
	add(v);
      }
      if(!p->Visible) v->setVisibility(0);
    }
    List_Delete(points);
  }
  if(Tree_Nbr(THEM->Curves)) {
    List_T *curves = Tree2List(THEM->Curves);
    for(int i = 0; i < List_Nbr(curves); i++){
      Curve *c;
      List_Read(curves, i, &c);
      if(c->Num >= 0 && c->beg && c->end){
	GEdge *e = edgeByTag(c->Num);
	if(!e){
	  e = new gmshEdge(this, c,
			   vertexByTag(c->beg->Num),
			   vertexByTag(c->end->Num));
	  add(e);
	}
	if(!c->Visible) e->setVisibility(0);
	if(c->Color.type) e->setColor(c->Color.mesh);
      }
    }
    List_Delete(curves);
  }
  if(Tree_Nbr(THEM->Surfaces)) {
    List_T *surfaces = Tree2List(THEM->Surfaces);
    for(int i = 0; i < List_Nbr(surfaces); i++){
      Surface *s;
      List_Read(surfaces, i, &s);
      GFace *f = faceByTag(s->Num);
      if(!f){
	f = new gmshFace(this, s);
	add(f);
      }
      if(!s->Visible) f->setVisibility(0);
      if(s->Color.type) f->setColor(s->Color.mesh);
    }
    List_Delete(surfaces);
  } 
  if(Tree_Nbr(THEM->Volumes)) {
    List_T *volumes = Tree2List(THEM->Volumes);
    for(int i = 0; i < List_Nbr(volumes); i++){
      Volume *v;
      List_Read(volumes, i, &v);
      GRegion *r = regionByTag(v->Num);
      if(!r){
	r = new gmshRegion(this, v);
	add(r);
      }
      if(!v->Visible) r->setVisibility(0);
      if(v->Color.type) r->setColor(v->Color.mesh);
    }
    List_Delete(volumes);
  }
  for(int i = 0; i < List_Nbr(THEM->PhysicalGroups); i++){
    PhysicalGroup *p;
    List_Read(THEM->PhysicalGroups, i, &p);
    for(int j = 0; j < List_Nbr(p->Entities); j++){
      int num;
      List_Read(p->Entities, j, &num);
      GEntity *ge = 0;
      switch(p->Typ){
      case MSH_PHYSICAL_POINT:   ge = vertexByTag(abs(num)); break;
      case MSH_PHYSICAL_LINE:    ge = edgeByTag(abs(num)); break;
      case MSH_PHYSICAL_SURFACE: ge = faceByTag(abs(num)); break;
      case MSH_PHYSICAL_VOLUME:  ge = regionByTag(abs(num)); break;
      }
      int pnum = sign(num) * p->Num;
      if(ge && std::find(ge->physicals.begin(), ge->physicals.end(), pnum) == 
	 ge->physicals.end())
	ge->physicals.push_back(pnum);
    }
  }
  
  Msg(DEBUG, "gmshModel:");
  Msg(DEBUG, "%d Vertices", vertices.size());
  Msg(DEBUG, "%d Edges", edges.size());
  Msg(DEBUG, "%d Faces", faces.size());
  Msg(DEBUG, "%d Regions", regions.size());
}

static FILE *geo = 0;

class writeGVertexGEO {
 public :
  void operator () (GVertex *gv)
  {
    Vertex *v = (Vertex*)gv->getNativePtr();
    fprintf(geo, "Point(%d) = {%.16g, %.16g, %.16g, %.16g};\n",
	    v->Num, v->Pos.X, v->Pos.Y, v->Pos.Z, v->lc);
  }
};

class writeGEdgeGEO {
 public :
  void operator () (GEdge *ge)
  {
    Curve *c = (Curve *)ge->getNativePtr();
    
    if(c->Num < 0 || c->Typ == MSH_SEGM_DISCRETE)
      return;

    switch (c->Typ) {
    case MSH_SEGM_LINE:
      fprintf(geo, "Line (%d) = ", c->Num);
      break;
    case MSH_SEGM_CIRC:
    case MSH_SEGM_CIRC_INV:
      fprintf(geo, "Circle (%d) = ", c->Num);
      break;
    case MSH_SEGM_ELLI:
    case MSH_SEGM_ELLI_INV:
      fprintf(geo, "Ellipse (%d) = ", c->Num);
      break;
    case MSH_SEGM_NURBS:
      fprintf(geo, "Nurbs (%d) = {", c->Num);
      for(int i = 0; i < List_Nbr(c->Control_Points); i++) {
	Vertex *v;
	List_Read(c->Control_Points, i, &v);
	if(!i)
	  fprintf(geo, "%d", v->Num);
	else
	  fprintf(geo, ", %d", v->Num);
	if(i % 8 == 7 && i != List_Nbr(c->Control_Points) - 1)
	  fprintf(geo, "\n");
      }
      fprintf(geo, "}\n");
      fprintf(geo, "  Knots {");
      for(int j = 0; j < List_Nbr(c->Control_Points) + c->degre + 1; j++) {
	if(!j)
	  fprintf(geo, "%.16g", c->k[j]);
	else
	  fprintf(geo, ", %.16g", c->k[j]);
	if(j % 5 == 4 && j != List_Nbr(c->Control_Points) + c->degre)
	  fprintf(geo, "\n        ");
      }
      fprintf(geo, "}\n");
      fprintf(geo, "  Order %d;\n", c->degre);
      return;
    case MSH_SEGM_SPLN:
      fprintf(geo, "CatmullRom (%d) = ", c->Num);
      break;
    case MSH_SEGM_BSPLN:
      fprintf(geo, "BSpline (%d) = ", c->Num);
      break;
    case MSH_SEGM_BEZIER:
      fprintf(geo, "Bezier (%d) = ", c->Num);
      break;
    default:
      Msg(GERROR, "Unknown curve type %d", c->Typ);
      return;
    }

    for(int i = 0; i < List_Nbr(c->Control_Points); i++) {
      Vertex *v;
      List_Read(c->Control_Points, i, &v);
      if(i)
	fprintf(geo, ", %d", v->Num);
      else
	fprintf(geo, "{%d", v->Num);
      if(i % 6 == 7)
	fprintf(geo, "\n");
    }
    
    fprintf(geo, "};\n");
  }
};

class writeGFaceGEO {
 public :
  void operator () (GFace *gf)
  {
    Surface *s = (Surface *)gf->getNativePtr();
    
    if(s->Typ == MSH_SURF_DISCRETE)
      return;
    
    int NUMLOOP = s->Num + 1000000;
    if(s->Typ != MSH_SURF_NURBS) {
      if(List_Nbr(s->Generatrices)){
	fprintf(geo, "Line Loop (%d) = ", NUMLOOP);
	for(int i = 0; i < List_Nbr(s->Generatrices); i++) {
	  Curve *c;
	  List_Read(s->Generatrices, i, &c);
	  if(i)
	    fprintf(geo, ", %d", c->Num);
	  else
	    fprintf(geo, "{%d", c->Num);
	}
	fprintf(geo, "};\n");
      }
    }
    
    switch (s->Typ) {
    case MSH_SURF_REGL:
    case MSH_SURF_TRIC:
      fprintf(geo, "Ruled Surface (%d) = {%d};\n", s->Num, NUMLOOP);
      break;
    case MSH_SURF_PLAN:
      fprintf(geo, "Plane Surface (%d) = {%d};\n", s->Num, NUMLOOP);
      break;
    case MSH_SURF_TRIMMED:
      fprintf(geo, "Trimmed Surface (%d) = %d {%d};\n", s->Num,
	      s->Support->Num, NUMLOOP);
      break;
    case MSH_SURF_NURBS:
      fprintf(geo, "Nurbs Surface (%d) = {\n", s->Num);
      for(int i = 0; i < s->Nv; i++) {
	fprintf(geo, "  {");
	for(int j = 0; j < s->Nu; j++) {
	  Vertex *v;
	  List_Read(s->Control_Points, j + s->Nu * i, &v);
	  if(!j)
	    fprintf(geo, "%d", v->Num);
	  else
	    fprintf(geo, ", %d", v->Num);
	}
	if(i != s->Nv - 1)
	  fprintf(geo, "},\n");
	else
	  fprintf(geo, "}}\n");
      }
      fprintf(geo, "  Knots\n  {");
      for(int j = 0; j < s->Nu + s->OrderU + 1; j++) {
	if(!j)
	  fprintf(geo, "%.16g", s->ku[j]);
	else
	  fprintf(geo, ", %.16g", s->ku[j]);
	if(j % 5 == 4 && j != s->Nu + s->OrderU)
	  fprintf(geo, "\n  ");
      }
      fprintf(geo, "}\n  {");
      for(int j = 0; j < s->Nv + s->OrderV + 1; j++) {
	if(!j)
	  fprintf(geo, "%.16g", s->kv[j]);
	else
	  fprintf(geo, ", %.16g", s->kv[j]);
	if(j % 5 == 4 && j != s->Nv + s->OrderV)
	  fprintf(geo, "\n  ");
      }
      fprintf(geo, "}\n  Order %d %d;\n", s->OrderU, s->OrderV);
      break;
    }
  }
};

class writeGRegionGEO {
 public :
  void operator () (GRegion *gr)
  {
    Volume *vol = (Volume *)gr->getNativePtr();
    
    if(vol->Typ == MSH_VOLUME_DISCRETE)
      return;
    
    int NUMLOOP = vol->Num + 1000000;
    
    fprintf(geo, "Surface Loop (%d) = ", NUMLOOP);
    
    for(int i = 0; i < List_Nbr(vol->Surfaces); i++) {
      Surface *s;
      List_Read(vol->Surfaces, i, &s);
      if(i)
	fprintf(geo, ", %d", s->Num);
      else
	fprintf(geo, "{%d", s->Num);
    }
    fprintf(geo, "};\n");
    
    switch (vol->Typ) {
    case MSH_VOLUME:
      fprintf(geo, "Volume (%d) = {%d};\n", vol->Num, NUMLOOP);
      break;
    }
  }
};

void writePhysicalGroupGEO(void *a, void *b)
{
  PhysicalGroup *pg = *(PhysicalGroup **) a;

  switch (pg->Typ) {
  case MSH_PHYSICAL_POINT:
    fprintf(geo, "Physical Point (%d) = ", pg->Num);
    break;
  case MSH_PHYSICAL_LINE:
    fprintf(geo, "Physical Line (%d) = ", pg->Num);
    break;
  case MSH_PHYSICAL_SURFACE:
    fprintf(geo, "Physical Surface (%d) = ", pg->Num);
    break;
  case MSH_PHYSICAL_VOLUME:
    fprintf(geo, "Physical Volume (%d) = ", pg->Num);
    break;
  }

  for(int i = 0; i < List_Nbr(pg->Entities); i++) {
    int j;
    List_Read(pg->Entities, i, &j);
    if(i)
      fprintf(geo, ", %d", j);
    else
      fprintf(geo, "{%d", j);
  }
  fprintf(geo, "};\n");
}

int gmshModel::writeGEO(const std::string &name)
{
  geo = fopen(name.c_str(), "w");
  if(!geo){
    Msg(GERROR, "Unable to open file '%s'", name.c_str());
    return 0;
  }

  std::for_each(firstVertex(), lastVertex(), writeGVertexGEO());
  std::for_each(firstEdge(), lastEdge(), writeGEdgeGEO());
  std::for_each(firstFace(), lastFace(), writeGFaceGEO());
  std::for_each(firstRegion(), lastRegion(), writeGRegionGEO());

  List_Action(THEM->PhysicalGroups, writePhysicalGroupGEO);

  fclose(geo);
  return 1;
}
