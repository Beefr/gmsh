// $Id: Element.cpp,v 1.7 2005-03-30 19:17:07 geuzaine Exp $
//
// Copyright (C) 1997-2005 C. Geuzaine, J.-F. Remacle
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
#include "Mesh.h"
#include "Numeric.h"
#include "Element.h"

extern int edges_quad[4][2];
extern int edges_hexa[12][2];
extern int edges_prism[9][2];
extern int edges_pyramid[8][2];

int Element::TotalNumber = 0;

Element::Element()
  : iEnt(-1), iPart(-1), Visible(VIS_MESH), VSUP(NULL)
{
  Num = ++TotalNumber; 
}

Element::~Element()
{
  if(VSUP) Free(VSUP);
}

double Element::lij(Vertex *Vi, Vertex *Vj)
{
  return sqrt(DSQR(Vi->Pos.X - Vj->Pos.X) +
              DSQR(Vi->Pos.Y - Vj->Pos.Y) +
              DSQR(Vi->Pos.Z - Vj->Pos.Z));
}

// Quads

Quadrangle::Quadrangle()
  : Element()
{
  for(int i = 0; i < 4; i++) V[i] = NULL;
}

Quadrangle::Quadrangle(Vertex *v1, Vertex *v2, Vertex *v3, Vertex *v4)
  : Element()
{
  V[0] = v1; V[1] = v2; V[2] = v3; V[3] = v4;
}

double Quadrangle::maxEdge()
{
  double maxlij = 0.;
  for(int i = 0; i < 4; i++)
      maxlij = DMAX(maxlij, lij(V[edges_quad[i][0]], V[edges_quad[i][1]]));
  return maxlij;
}

Quadrangle *Create_Quadrangle(Vertex *v1, Vertex *v2, Vertex *v3, Vertex *v4)
{
  return new Quadrangle(v1, v2, v3, v4);
}

void Quadrangle::ExportLcField(FILE * f)
{
  if(!VSUP)
    fprintf(f, "SQ(%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g){%g,%g,%g,%g};\n",
	    V[0]->Pos.X, V[0]->Pos.Y, V[0]->Pos.Z, V[1]->Pos.X, V[1]->Pos.Y,
	    V[1]->Pos.Z, V[2]->Pos.X, V[2]->Pos.Y, V[2]->Pos.Z, V[3]->Pos.X,
	    V[3]->Pos.Y, V[3]->Pos.Z, V[0]->lc, V[1]->lc, V[2]->lc, V[3]->lc);
  else
    fprintf(f, "SQ2(%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,"
	    "%g,%g,%g,%g,%g,%g,%g,%g,%g){%g,%g,%g,%g,%g,%g,%g,%g,%g};\n",
	    V[0]->Pos.X, V[0]->Pos.Y, V[0]->Pos.Z, V[1]->Pos.X, V[1]->Pos.Y,
	    V[1]->Pos.Z, V[2]->Pos.X, V[2]->Pos.Y, V[2]->Pos.Z, V[3]->Pos.X,
	    V[3]->Pos.Y, V[3]->Pos.Z, 
	    VSUP[0]->Pos.X, VSUP[0]->Pos.Y, VSUP[0]->Pos.Z, VSUP[1]->Pos.X, VSUP[1]->Pos.Y,
	    VSUP[1]->Pos.Z, VSUP[2]->Pos.X, VSUP[2]->Pos.Y, VSUP[2]->Pos.Z, VSUP[3]->Pos.X,
	    VSUP[3]->Pos.Y, VSUP[3]->Pos.Z, VSUP[4]->Pos.X, VSUP[4]->Pos.Y, VSUP[4]->Pos.Z, 
	    V[0]->lc, V[1]->lc, V[2]->lc, V[3]->lc, 
	    VSUP[0]->lc, VSUP[1]->lc, VSUP[2]->lc, VSUP[3]->lc, VSUP[4]->lc);
}

void Free_Quadrangle(void *a, void *b)
{
  Quadrangle *q = *(Quadrangle **) a;
  if(q) {
    delete q;
    q = NULL;
  }
}

int compareQuadrangle(const void *a, const void *b)
{
  Quadrangle *q = *(Quadrangle **) a;
  Quadrangle *w = *(Quadrangle **) b;
  return (q->Num - w->Num);
}

// Hexas

Hexahedron::Hexahedron()
  : Element()
{
  for(int i = 0; i < 8; i++) V[i] = NULL;
}

Hexahedron::Hexahedron(Vertex *v1, Vertex *v2, Vertex *v3, Vertex *v4,
		       Vertex *v5, Vertex *v6, Vertex *v7, Vertex *v8)
  : Element()
{
  V[0] = v1; V[1] = v2; V[2] = v3; V[3] = v4;
  V[4] = v5; V[5] = v6; V[6] = v7; V[7] = v8;
}

double Hexahedron::Orientation()
{
  double mat[3][3];
  mat[0][0] = V[1]->Pos.X - V[0]->Pos.X;
  mat[0][1] = V[3]->Pos.X - V[0]->Pos.X;
  mat[0][2] = V[4]->Pos.X - V[0]->Pos.X;
  mat[1][0] = V[1]->Pos.Y - V[0]->Pos.Y;
  mat[1][1] = V[3]->Pos.Y - V[0]->Pos.Y;
  mat[1][2] = V[4]->Pos.Y - V[0]->Pos.Y;
  mat[2][0] = V[1]->Pos.Z - V[0]->Pos.Z;
  mat[2][1] = V[3]->Pos.Z - V[0]->Pos.Z;
  mat[2][2] = V[4]->Pos.Z - V[0]->Pos.Z;
  return det3x3(mat);
}

double Hexahedron::maxEdge()
{
  double maxlij = 0.;
  for(int i = 0; i < 12; i++)
      maxlij = DMAX(maxlij, lij(V[edges_hexa[i][0]], V[edges_hexa[i][1]]));
  return maxlij;
}

Hexahedron *Create_Hexahedron(Vertex * v1, Vertex * v2, Vertex * v3,
                              Vertex * v4, Vertex * v5, Vertex * v6,
                              Vertex * v7, Vertex * v8)
{
  return new Hexahedron(v1, v2, v3, v4, v5, v6, v7, v8);
}

void Hexahedron::ExportLcField(FILE * f)
{
  if(!VSUP)
    fprintf(f,"SH(%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,"
	    "%g,%g,%g,%g,%g,%g){%g,%g,%g,%g,%g,%g,%g,%g};\n",
	    V[0]->Pos.X, V[0]->Pos.Y, V[0]->Pos.Z, V[1]->Pos.X, V[1]->Pos.Y,
	    V[1]->Pos.Z, V[2]->Pos.X, V[2]->Pos.Y, V[2]->Pos.Z, V[3]->Pos.X, 
	    V[3]->Pos.Y, V[3]->Pos.Z, V[4]->Pos.X, V[4]->Pos.Y, V[4]->Pos.Z,
	    V[5]->Pos.X, V[5]->Pos.Y, V[5]->Pos.Z, V[6]->Pos.X, V[6]->Pos.Y, 
	    V[6]->Pos.Z, V[7]->Pos.X, V[7]->Pos.Y, V[7]->Pos.Z, V[0]->lc, 
	    V[1]->lc, V[2]->lc, V[3]->lc, V[4]->lc, V[5]->lc, V[6]->lc, V[7]->lc);
  else{
    fprintf(f,"SH2(%g,%g,%g", V[0]->Pos.X, V[0]->Pos.Y, V[0]->Pos.Z);
    for(int i = 1; i < 8; i++) 
      fprintf(f,",%g,%g,%g", V[i]->Pos.X, V[i]->Pos.Y, V[i]->Pos.Z);
    for(int i = 0; i < 19; i++) 
      fprintf(f,",%g,%g,%g", VSUP[i]->Pos.X, VSUP[i]->Pos.Y, VSUP[i]->Pos.Z);
    fprintf(f,"){%g", V[0]->lc);
    for(int i = 1; i < 8; i++) 
      fprintf(f,",%g", V[i]->lc);
    for(int i = 0; i < 19; i++) 
      fprintf(f,",%g", VSUP[i]->lc);
    fprintf(f,"};\n");
  }
}

void Free_Hexahedron(void *a, void *b)
{
  Hexahedron *h = *(Hexahedron **) a;
  if(h) {
    delete h;
    h = NULL;
  }
}

int compareHexahedron(const void *a, const void *b)
{
  Hexahedron *q = *(Hexahedron **) a;
  Hexahedron *w = *(Hexahedron **) b;
  return (q->Num - w->Num);
}

// Prisms

Prism::Prism()
  : Element()
{
  for(int i = 0; i < 6; i++) V[i] = NULL;
}

Prism::Prism(Vertex *v1, Vertex *v2, Vertex *v3, 
	     Vertex *v4, Vertex *v5, Vertex *v6)
  : Element()
{
  V[0] = v1; V[1] = v2; V[2] = v3; 
  V[3] = v4; V[4] = v5; V[5] = v6;
}

double Prism::Orientation()
{
  double mat[3][3];
  mat[0][0] = V[1]->Pos.X - V[0]->Pos.X;
  mat[0][1] = V[2]->Pos.X - V[0]->Pos.X;
  mat[0][2] = V[3]->Pos.X - V[0]->Pos.X;
  mat[1][0] = V[1]->Pos.Y - V[0]->Pos.Y;
  mat[1][1] = V[2]->Pos.Y - V[0]->Pos.Y;
  mat[1][2] = V[3]->Pos.Y - V[0]->Pos.Y;
  mat[2][0] = V[1]->Pos.Z - V[0]->Pos.Z;
  mat[2][1] = V[2]->Pos.Z - V[0]->Pos.Z;
  mat[2][2] = V[3]->Pos.Z - V[0]->Pos.Z;
  return det3x3(mat);
}

double Prism::maxEdge()
{
  double maxlij = 0.;
  for(int i = 0; i < 9; i++)
      maxlij = DMAX(maxlij, lij(V[edges_prism[i][0]], V[edges_prism[i][1]]));
  return maxlij;
}

Prism *Create_Prism(Vertex * v1, Vertex * v2, Vertex * v3,
                    Vertex * v4, Vertex * v5, Vertex * v6)
{
  return new Prism(v1, v2, v3, v4, v5, v6);
}

void Prism::ExportLcField(FILE * f)
{
  if(!VSUP)
    fprintf(f,"SI(%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g)"
	    "{%g,%g,%g,%g,%g,%g};\n",
	    V[0]->Pos.X, V[0]->Pos.Y, V[0]->Pos.Z, V[1]->Pos.X, V[1]->Pos.Y,
	    V[1]->Pos.Z, V[2]->Pos.X, V[2]->Pos.Y, V[2]->Pos.Z, V[3]->Pos.X, 
	    V[3]->Pos.Y, V[3]->Pos.Z, V[4]->Pos.X, V[4]->Pos.Y, V[4]->Pos.Z,
	    V[5]->Pos.X, V[5]->Pos.Y, V[5]->Pos.Z, V[0]->lc, V[1]->lc, V[2]->lc,
	    V[3]->lc, V[4]->lc, V[5]->lc);
  else{
    fprintf(f,"SI2(%g,%g,%g", V[0]->Pos.X, V[0]->Pos.Y, V[0]->Pos.Z);
    for(int i = 1; i < 6; i++) 
      fprintf(f,",%g,%g,%g", V[i]->Pos.X, V[i]->Pos.Y, V[i]->Pos.Z);
    for(int i = 0; i < 12; i++) 
      fprintf(f,",%g,%g,%g", VSUP[i]->Pos.X, VSUP[i]->Pos.Y, VSUP[i]->Pos.Z);
    fprintf(f,"){%g", V[0]->lc);
    for(int i = 1; i < 6; i++) 
      fprintf(f,",%g", V[i]->lc);
    for(int i = 0; i < 12; i++) 
      fprintf(f,",%g", VSUP[i]->lc);
    fprintf(f,"};\n");
  }
}

void Free_Prism(void *a, void *b)
{
  Prism *p = *(Prism **) a;
  if(p) {
    delete p;
    p = NULL;
  }
}

int comparePrism(const void *a, const void *b)
{
  Prism *q = *(Prism **) a;
  Prism *w = *(Prism **) b;
  return (q->Num - w->Num);
}

// Pyramids

Pyramid::Pyramid()
  : Element()
{
  for(int i = 0; i < 5; i++) V[i] = NULL;
}

Pyramid::Pyramid(Vertex *v1, Vertex *v2, Vertex *v3, Vertex *v4, Vertex *v5)
  : Element()
{
  V[0] = v1; V[1] = v2; V[2] = v3; V[3] = v4; V[4] = v5; 
}

double Pyramid::Orientation()
{
  double mat[3][3];
  mat[0][0] = V[1]->Pos.X - V[0]->Pos.X;
  mat[0][1] = V[3]->Pos.X - V[0]->Pos.X;
  mat[0][2] = V[4]->Pos.X - V[0]->Pos.X;
  mat[1][0] = V[1]->Pos.Y - V[0]->Pos.Y;
  mat[1][1] = V[3]->Pos.Y - V[0]->Pos.Y;
  mat[1][2] = V[4]->Pos.Y - V[0]->Pos.Y;
  mat[2][0] = V[1]->Pos.Z - V[0]->Pos.Z;
  mat[2][1] = V[3]->Pos.Z - V[0]->Pos.Z;
  mat[2][2] = V[4]->Pos.Z - V[0]->Pos.Z;
  return det3x3(mat);
}

double Pyramid::maxEdge()
{
  double maxlij = 0.;
  for(int i = 0; i < 8; i++)
      maxlij = DMAX(maxlij, lij(V[edges_pyramid[i][0]], V[edges_pyramid[i][1]]));
  return maxlij;
}

Pyramid *Create_Pyramid(Vertex * v1, Vertex * v2, Vertex * v3,
                        Vertex * v4, Vertex * v5)
{
  return new Pyramid(v1, v2, v3, v4, v5);
}

void Pyramid::ExportLcField(FILE * f)
{
  if(!VSUP)
    fprintf(f,"SY(%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g,%g){%g,%g,%g,%g,%g};\n",
	    V[0]->Pos.X, V[0]->Pos.Y, V[0]->Pos.Z, V[1]->Pos.X, V[1]->Pos.Y,
	    V[1]->Pos.Z, V[2]->Pos.X, V[2]->Pos.Y, V[2]->Pos.Z, V[3]->Pos.X, 
	    V[3]->Pos.Y, V[3]->Pos.Z, V[4]->Pos.X, V[4]->Pos.Y, V[4]->Pos.Z,
	    V[0]->lc, V[1]->lc, V[2]->lc, V[3]->lc, V[4]->lc);
  else{
    fprintf(f,"SY2(%g,%g,%g", V[0]->Pos.X, V[0]->Pos.Y, V[0]->Pos.Z);
    for(int i = 1; i < 5; i++) 
      fprintf(f,",%g,%g,%g", V[i]->Pos.X, V[i]->Pos.Y, V[i]->Pos.Z);
    for(int i = 0; i < 9; i++) 
      fprintf(f,",%g,%g,%g", VSUP[i]->Pos.X, VSUP[i]->Pos.Y, VSUP[i]->Pos.Z);
    fprintf(f,"){%g", V[0]->lc);
    for(int i = 1; i < 5; i++) 
      fprintf(f,",%g", V[i]->lc);
    for(int i = 0; i < 9; i++) 
      fprintf(f,",%g", VSUP[i]->lc);
    fprintf(f,"};\n");
  }
}

void Free_Pyramid(void *a, void *b)
{
  Pyramid *p = *(Pyramid **) a;
  if(p) {
    delete p;
    p = NULL;
  }
}

int comparePyramid(const void *a, const void *b)
{
  Pyramid *q = *(Pyramid **) a;
  Pyramid *w = *(Pyramid **) b;
  return (q->Num - w->Num);
}
