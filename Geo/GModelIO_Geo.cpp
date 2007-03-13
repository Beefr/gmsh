// $Id: GModelIO_Geo.cpp,v 1.10 2007-03-13 09:25:50 geuzaine Exp $
//
// Copyright (C) 1997-2007 C. Geuzaine, J.-F. Remacle
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

#include "GModel.h"
#include "Geo.h"
#include "OpenFile.h"
#include "Tools.h"
#include "Numeric.h"
#include "Message.h"
#include "gmshVertex.h"
#include "gmshFace.h"
#include "gmshEdge.h"
#include "gmshRegion.h"
#include "Parser.h" // for Symbol_T

extern Mesh *THEM;

int GModel::readGEO(const std::string &name)
{
  ParseFile((char*)name.c_str(), 1);
  return importTHEM();
}

int GModel::importTHEM()
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
	else
	  e->resetMeshAttributes();
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
      else
	f->resetMeshAttributes();
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
      else
	r->resetMeshAttributes();	
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

  Msg(DEBUG, "Gmsh model imported:");
  Msg(DEBUG, "%d Vertices", vertices.size());
  Msg(DEBUG, "%d Edges", edges.size());
  Msg(DEBUG, "%d Faces", faces.size());
  Msg(DEBUG, "%d Regions", regions.size());
  
  return 1;
}

class writeGVertexGEO {
 private :
  FILE *geo;
 public :
  writeGVertexGEO(FILE *fp) { geo = fp ? fp : stdout; }
  void operator() (GVertex *gv)
  {
    if(gv->getNativeType() == GEntity::GmshModel){
      Vertex *v = (Vertex*)gv->getNativePtr();
      if(!v) return;
      fprintf(geo, "Point(%d) = {%.16g, %.16g, %.16g, %.16g};\n",
	      v->Num, v->Pos.X, v->Pos.Y, v->Pos.Z, v->lc);
    }
    else{
      fprintf(geo, "Point(%d) = {%.16g, %.16g, %.16g, %.16g};\n",
	      gv->tag(), gv->x(), gv->y(), gv->z(), 
	      gv->prescribedMeshSizeAtVertex());
    }
  }
};

class writeGEdgeGEO {
 private :
  FILE *geo;
 public :
  writeGEdgeGEO(FILE *fp) { geo = fp ? fp : stdout; }
  void operator () (GEdge *ge)
  {
    if(ge->geomType() == GEntity::DiscreteCurve) return;
    
    if(ge->getNativeType() == GEntity::GmshModel){
      Curve *c = (Curve *)ge->getNativePtr();
      if(!c || c->Num < 0) return;
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
    else{
      if(ge->getBeginVertex() && ge->getEndVertex()){
	if(ge->geomType() == GEntity::Line){
	  fprintf(geo, "Line (%d) = {%d, %d};\n", 
		  ge->tag(), ge->getBeginVertex()->tag(), ge->getEndVertex()->tag());
	}
	else{
	  // approximate all other curves by splines
	  Range<double> bounds = ge->parBounds(0);
	  double umin = bounds.low();
	  double umax = bounds.high();
	  fprintf(geo, "p%d = newp;\n", ge->tag());
	  for(int i = 1; i < ge->minimumDrawSegments(); i++){
	    double u = umin + (double)i / ge->minimumDrawSegments() * (umax - umin);
	    GPoint p = ge->point(u);
	    fprintf(geo, "Point(p%d + %d) = {%.16g, %.16g, %.16g, 1.e+22};\n", 
		    ge->tag(), i, p.x(), p.y(), p.z());
	  }
	  fprintf(geo, "CatmullRom (%d) = {%d", ge->tag(), ge->getBeginVertex()->tag());
	  for(int i = 1; i < ge->minimumDrawSegments(); i++)
	    fprintf(geo, ", p%d + %d", ge->tag(), i);
	  fprintf(geo, ", %d};\n", ge->getEndVertex()->tag());
	}
      }
    }
  }
};

class writeGFaceGEO {
 private :
  FILE *geo;
 public :
  writeGFaceGEO(FILE *fp) { geo = fp ? fp : stdout; }
  void operator () (GFace *gf)
  {
    if(gf->geomType() == GEntity::DiscreteSurface) return;

    std::list<GEdge*> edges = gf->edges();
    std::list<int> orientations = gf->orientations();
    if(edges.size() && orientations.size() == edges.size()){
      std::vector<int> num, ori;
      for(std::list<GEdge*>::iterator it = edges.begin(); it != edges.end(); it++)
	num.push_back((*it)->tag());
      for(std::list<int>::iterator it = orientations.begin(); it != orientations.end(); it++)
	ori.push_back((*it) > 0 ? 1 : -1);
      fprintf(geo, "Line Loop (%d) = ", gf->tag());
      for(unsigned int i = 0; i < num.size(); i++){
	if(i)
	  fprintf(geo, ", %d", num[i] * ori[i]);
	else
	  fprintf(geo, "{%d", num[i] * ori[i]);
      }
      fprintf(geo, "};\n");
      if(gf->geomType() == GEntity::Plane){
	fprintf(geo, "Plane Surface (%d) = {%d};\n", gf->tag(), gf->tag());
      }
      else if(edges.size() == 3 || edges.size() == 4){
	fprintf(geo, "Ruled Surface (%d) = {%d};\n", gf->tag(), gf->tag());
      }
      else{
	Msg(GERROR, "Skipping surface %d in export", gf->tag());
      }
    }
  }
};

class writeGRegionGEO {
 private :
  FILE *geo;
 public :
  writeGRegionGEO(FILE *fp) { geo = fp ? fp : stdout; }
  void operator () (GRegion *gr)
  {
    if(gr->geomType() == GEntity::DiscreteVolume) return;

    std::list<GFace*> faces = gr->faces();
    if(faces.size()){
      fprintf(geo, "Surface Loop (%d) = ", gr->tag());
      for(std::list<GFace*>::iterator it = faces.begin(); it != faces.end(); it++) {
	if(it != faces.begin())
	  fprintf(geo, ", %d", (*it)->tag());
	else
	  fprintf(geo, "{%d", (*it)->tag());
      }
      fprintf(geo, "};\n");
      fprintf(geo, "Volume (%d) = {%d};\n", gr->tag(), gr->tag());
    }
  }
};

class writePhysicalGroupGEO {
 private :
  FILE *geo;
  int dim;
  std::map<int, std::string> &oldLabels, &newLabels;
 public :
  writePhysicalGroupGEO(FILE *fp, int i, 
			std::map<int, std::string> &o,
			std::map<int, std::string> &n)
    : dim(i), oldLabels(o), newLabels(n)
  { 
    geo = fp ? fp : stdout; 
  }
  void operator () (std::pair<const int, std::vector<GEntity *> > &g)
  {
    std::string oldName, newName;
    if(oldLabels.count(g.first)) {
      oldName = oldLabels[g.first];
      fprintf(geo, "%s = %d;\n", oldName.c_str(), g.first);
    }
    else if(newLabels.count(g.first)) {
      newName = newLabels[g.first];
    }

    switch (dim) {
    case 0: fprintf(geo, "Physical Point"); break;
    case 1: fprintf(geo, "Physical Line"); break;
    case 2: fprintf(geo, "Physical Surface"); break;
    case 3: fprintf(geo, "Physical Volume"); break;
    }

    if(oldName.size())
      fprintf(geo, " (%s) = {", oldName.c_str());
    else if(newName.size())
      fprintf(geo, " (\"%s\") = {", newName.c_str());
    else
      fprintf(geo, " (%d) = {", g.first);
    for(unsigned int i = 0; i < g.second.size(); i++) {
      if(i) fprintf(geo, ", ");
      fprintf(geo, "%d", g.second[i]->tag());
    }
    fprintf(geo, "};\n");
  }
};

int GModel::writeGEO(const std::string &name)
{
  FILE *fp = fopen(name.c_str(), "w");

  std::for_each(firstVertex(), lastVertex(), writeGVertexGEO(fp));
  std::for_each(firstEdge(), lastEdge(), writeGEdgeGEO(fp));
  std::for_each(firstFace(), lastFace(), writeGFaceGEO(fp));
  std::for_each(firstRegion(), lastRegion(), writeGRegionGEO(fp));

  // get "old-style" labels from parser
  std::map<int, std::string> labels;
  List_T *old = Tree2List(Symbol_T);
  for(int i = 0; i < List_Nbr(old); i++) {
    Symbol *s = (Symbol *)List_Pointer(old, i);
    for(int j = 0; j < List_Nbr(s->val); j++) {
      double tag;
      List_Read(s->val, j, &tag);
      labels[(int)tag] = std::string(s->Name);
    }
  }
  List_Delete(old);

  std::map<int, std::vector<GEntity*> > groups[4];
  getPhysicalGroups(groups);
  for(int i = 0; i < 4; i++)
    std::for_each(groups[i].begin(), groups[i].end(), 
		  writePhysicalGroupGEO(fp, i, labels, physicalNames));

  if(fp) fclose(fp);
  return 1;
}
