// $Id: SecondOrder.cpp,v 1.23 2004-05-25 04:10:05 geuzaine Exp $
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
#include "Mesh.h"
#include "Utils.h"
#include "Interpolation.h"
#include "Numeric.h"

// FIXME: still to add middle face nodes for quads, hexas, prisms and
// pyramids

extern Mesh *THEM;
extern int edges_tetra[6][2];
extern int edges_quad[4][2];
extern int edges_hexa[12][2];
extern int edges_prism[9][2];
extern int edges_pyramid[8][2];

static Surface *THES = NULL;
static Curve *THEC = NULL;
static EdgesContainer *edges = NULL;
static List_T *VerticesToDelete = NULL;

Vertex *oncurve(Vertex * v1, Vertex * v2)
{
  Vertex v, *pv;

  if(!THEC)
    return NULL;

  int ok1 = 1, ok2 = 1;
  double u1 = 0., u2 = 0.;

  if(List_Nbr(v1->ListCurves) == 1){
    u1 = v1->u;
  }
  else if(v1->Num == THEC->beg->Num){
    u1 = THEC->ubeg;
  }
  else if(v1->Num == THEC->end->Num){
    u1 = THEC->uend;
  }
  else{
    ok1 = 0;
  }

  if(List_Nbr(v2->ListCurves) == 1){
    u2 = v2->u;
  }
  else if(v2->Num == THEC->beg->Num){
    u2 = THEC->ubeg;
  }
  else if(v2->Num == THEC->end->Num){
    u2 = THEC->uend;
  }
  else{
    ok2 = 0;
  }

  if(ok1 && ok2){
    v = InterpolateCurve(THEC, 0.5 * (u1 + u2), 0);
  }
  else{
    // too bad (should normally not happen)
    v.Pos.X = (v1->Pos.X + v2->Pos.X) * 0.5;
    v.Pos.Y = (v1->Pos.Y + v2->Pos.Y) * 0.5;
    v.Pos.Z = (v1->Pos.Z + v2->Pos.Z) * 0.5;
  }

  pv = Create_Vertex(++THEM->MaxPointNum, v.Pos.X, v.Pos.Y, v.Pos.Z, v.lc, v.u);
  pv->Degree = 2;

  if(!pv->ListCurves) {
    pv->ListCurves = List_Create(1, 1, sizeof(Curve *));
  }
  List_Add(pv->ListCurves, &THEC);
  return pv;
}

Vertex *onsurface(Vertex * v1, Vertex * v2)
{
  Vertex v, *pv;
  double U, V, U1, U2, V1, V2;

  if(!THES)
    return NULL;
  if(THES->Typ == MSH_SURF_PLAN)
    return NULL;

  XYZtoUV(THES, v1->Pos.X, v1->Pos.Y, v1->Pos.Z, &U1, &V1, 1.0);
  XYZtoUV(THES, v2->Pos.X, v2->Pos.Y, v2->Pos.Z, &U2, &V2, 1.0);

  U = 0.5 * (U1 + U2);
  V = 0.5 * (V1 + V2);
  v = InterpolateSurface(THES, U, V, 0, 0);
  pv = Create_Vertex(++THEM->MaxPointNum, v.Pos.X, v.Pos.Y, v.Pos.Z, v.lc, v.u);
  pv->Degree = 2;

  return pv;
}

void PutMiddlePoint(void *a, void *b)
{
  Edge *ed;
  Vertex *v;
  int i, j, N;

  ed = (Edge *) a;

  if(ed->newv){
    v = ed->newv;
  }
  else if((v = oncurve(ed->V[0], ed->V[1]))){
  }
  else if((v = onsurface(ed->V[0], ed->V[1]))){
  }
  else{
    v = Create_Vertex(++THEM->MaxPointNum,
                      0.5 * (ed->V[0]->Pos.X + ed->V[1]->Pos.X),
                      0.5 * (ed->V[0]->Pos.Y + ed->V[1]->Pos.Y),
                      0.5 * (ed->V[0]->Pos.Z + ed->V[1]->Pos.Z),
                      0.5 * (ed->V[0]->lc + ed->V[1]->lc),
                      0.5 * (ed->V[0]->u + ed->V[1]->u));
    v->Degree = 2;
  }

  ed->newv = v;
  Tree_Insert(THEM->Vertices, &v);
      
  for(i = 0; i < List_Nbr(ed->Simplexes); i++) {
    Simplex *s;
    List_Read(ed->Simplexes, i, &s);
    if(s->V[3]) // tetrahedron
      N = 6;
    else if(s->V[2]) // triangle
      N = 3;
    else // line
      N = 1;
    if(!s->VSUP)
      s->VSUP = (Vertex **) Malloc(N * sizeof(Vertex *));
    for(j = 0; j < N; j++) {
      if((!compareVertex(&s->V[edges_tetra[j][0]], &ed->V[0]) &&
	  !compareVertex(&s->V[edges_tetra[j][1]], &ed->V[1])) ||
	 (!compareVertex(&s->V[edges_tetra[j][0]], &ed->V[1]) &&
	  !compareVertex(&s->V[edges_tetra[j][1]], &ed->V[0]))) {
	s->VSUP[j] = v;
      }
    }
  }

  for(i = 0; i < List_Nbr(ed->Quadrangles); i++) {
    Quadrangle *q;
    List_Read(ed->Quadrangles, i, &q);
    if(!q->VSUP)
      q->VSUP = (Vertex **) Malloc(4 * sizeof(Vertex *));
    for(j = 0; j < 4; j++) {
      if((!compareVertex(&q->V[edges_quad[j][0]], &ed->V[0]) &&
          !compareVertex(&q->V[edges_quad[j][1]], &ed->V[1])) ||
         (!compareVertex(&q->V[edges_quad[j][0]], &ed->V[1]) &&
          !compareVertex(&q->V[edges_quad[j][1]], &ed->V[0]))) {
        q->VSUP[j] = v;
      }
    }
  }

  for(i = 0; i < List_Nbr(ed->Hexahedra); i++) {
    Hexahedron *h;
    List_Read(ed->Hexahedra, i, &h);
    if(!h->VSUP)
      h->VSUP = (Vertex **) Malloc(12 * sizeof(Vertex *));
    for(j = 0; j < 12; j++) {
      if((!compareVertex(&h->V[edges_hexa[j][0]], &ed->V[0]) &&
          !compareVertex(&h->V[edges_hexa[j][1]], &ed->V[1])) ||
         (!compareVertex(&h->V[edges_hexa[j][0]], &ed->V[1]) &&
          !compareVertex(&h->V[edges_hexa[j][1]], &ed->V[0]))) {
        h->VSUP[j] = v;
      }
    }
  }

  for(i = 0; i < List_Nbr(ed->Prisms); i++) {
    Prism *p;
    List_Read(ed->Prisms, i, &p);
    if(!p->VSUP)
      p->VSUP = (Vertex **) Malloc(9 * sizeof(Vertex *));
    for(j = 0; j < 9; j++) {
      if((!compareVertex(&p->V[edges_prism[j][0]], &ed->V[0]) &&
          !compareVertex(&p->V[edges_prism[j][1]], &ed->V[1])) ||
         (!compareVertex(&p->V[edges_prism[j][0]], &ed->V[1]) &&
          !compareVertex(&p->V[edges_prism[j][1]], &ed->V[0]))) {
        p->VSUP[j] = v;
      }
    }
  }

  for(i = 0; i < List_Nbr(ed->Pyramids); i++) {
    Pyramid *p;
    List_Read(ed->Pyramids, i, &p);
    if(!p->VSUP)
      p->VSUP = (Vertex **) Malloc(8 * sizeof(Vertex *));
    for(j = 0; j < 8; j++) {
      if((!compareVertex(&p->V[edges_pyramid[j][0]], &ed->V[0]) &&
          !compareVertex(&p->V[edges_pyramid[j][1]], &ed->V[1])) ||
         (!compareVertex(&p->V[edges_pyramid[j][0]], &ed->V[1]) &&
          !compareVertex(&p->V[edges_pyramid[j][1]], &ed->V[0]))) {
        p->VSUP[j] = v;
      }
    }
  }

}

void ResetDegre2_Vertex(void *a, void *b)
{
  Vertex *v = *(Vertex**)a;
  if(v->Degree == 2)
    List_Add(VerticesToDelete, &v);
}

void ResetDegre2_Simplex(void *a, void *b)
{
  Simplex *s = *(Simplex**)a;
  Free(s->VSUP);  
  s->VSUP = NULL;
}

void ResetDegre2_Quadrangle(void *a, void *b)
{
  Quadrangle *q = *(Quadrangle**)a;
  Free(q->VSUP);  
  q->VSUP = NULL;
}

void ResetDegre2_Hexahedron(void *a, void *b)
{
  Hexahedron *h = *(Hexahedron**)a;
  Free(h->VSUP);  
  h->VSUP = NULL;
}

void ResetDegre2_Prism(void *a, void *b)
{
  Prism *p = *(Prism**)a;
  Free(p->VSUP);  
  p->VSUP = NULL;
}

void ResetDegre2_Pyramid(void *a, void *b)
{
  Pyramid *p = *(Pyramid**)a;
  Free(p->VSUP);  
  p->VSUP = NULL;
}

void ResetDegre2_Curve(void *a, void *b)
{
  Curve *c = *(Curve**)a;
  if(c->Dirty) return;
  Tree_Action(c->Simplexes, ResetDegre2_Simplex);
}

void ResetDegre2_Surface(void *a, void *b)
{
  Surface *s = *(Surface**)a;
  if(s->Dirty) return;
  Tree_Action(s->Simplexes, ResetDegre2_Simplex);
  Tree_Action(s->Quadrangles, ResetDegre2_Quadrangle);
}

void ResetDegre2_Volume(void *a, void *b)
{
  Volume *v = *(Volume**)a;
  if(v->Dirty) return;
  Tree_Action(v->Simplexes, ResetDegre2_Simplex);
  Tree_Action(v->Hexahedra, ResetDegre2_Hexahedron);
  Tree_Action(v->Prisms, ResetDegre2_Prism);
  Tree_Action(v->Pyramids, ResetDegre2_Pyramid);
}

void Degre1()
{
  // (re-)initialize the global tree of edges
  if(edges)
    delete edges;
  edges = new EdgesContainer();

  // reset VSUP in each element
  Tree_Action(THEM->Curves, ResetDegre2_Curve);
  Tree_Action(THEM->Surfaces, ResetDegre2_Surface);
  Tree_Action(THEM->Volumes, ResetDegre2_Volume);

  // remove any supp vertex from the database
  if(VerticesToDelete)
    List_Delete(VerticesToDelete);
  VerticesToDelete = List_Create(100, 100, sizeof(Vertex*));
  Tree_Action(THEM->Vertices, ResetDegre2_Vertex);
  for(int i = 0; i < List_Nbr(VerticesToDelete); i++){
    Vertex **v = (Vertex**)List_Pointer(VerticesToDelete, i);
    Tree_Suppress(THEM->Vertices, v);
    Free_Vertex(v, NULL);
  }
}

void Degre2_Curve(void *a, void *b)
{
  Curve *c = *(Curve**)a;
  if(c->Dirty) return;
  edges->AddSimplexTree(c->Simplexes);
  THEC = c;
  THES = NULL;
  Tree_Action(edges->AllEdges, PutMiddlePoint);
}

void Degre2_Surface(void *a, void *b)
{
  Surface *s = *(Surface**)a;
  if(s->Dirty) return;
  edges->AddSimplexTree(s->Simplexes);
  edges->AddQuadrangleTree(s->Quadrangles);
  THEC = NULL;
  THES = s;
  Tree_Action(edges->AllEdges, PutMiddlePoint);
}

void Degre2_Volume(void *a, void *b)
{
  Volume *v = *(Volume**)a;
  if(v->Dirty) return;

  edges->AddSimplexTree(v->Simplexes);
  edges->AddHexahedronTree(v->Hexahedra);
  edges->AddPrismTree(v->Prisms);
  edges->AddPyramidTree(v->Pyramids);
  THEC = NULL;
  THES = NULL;
  Tree_Action(edges->AllEdges, PutMiddlePoint);
}

void Degre2(int dim)
{
  Msg(STATUS2, "Mesh second order...");
  double t1 = Cpu();

  Degre1();

  if(dim >= 1)
    Tree_Action(THEM->Curves, Degre2_Curve);
  if(dim >= 2)
    Tree_Action(THEM->Surfaces, Degre2_Surface);
  if(dim >= 3)
    Tree_Action(THEM->Volumes, Degre2_Volume);

  double t2 = Cpu();
  Msg(STATUS2, "Mesh second order complete (%g s)", t2 - t1);
}
