// $Id: VertexArray.cpp,v 1.12 2006-08-16 05:25:22 geuzaine Exp $
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

#include "Gmsh.h"
#include "VertexArray.h"
#include "Context.h"
#include "Numeric.h"

extern Context_T CTX;

VertexArray::VertexArray(int numNodesPerElement, int numElements) 
{
  type = numNodesPerElement;
  if(type < 1 || type > 4){
    Msg(GERROR, "Vertex arrays not done for %d-node element", type);
    type = 3;
  }
  fill = 0;
  if(!numElements)
    numElements = 1;
  int nb = numElements * numNodesPerElement;
  vertices = List_Create(nb * 3, 3000, sizeof(float));
  normals = List_Create(nb * 3, 3000, sizeof(char));
  colors = List_Create(nb * 4, 4000, sizeof(unsigned char));
}

VertexArray::~VertexArray()
{
  List_Delete(vertices);
  List_Delete(normals);
  List_Delete(colors);
}

int VertexArray::n()
{
  return List_Nbr(vertices) / 3;
}

void VertexArray::add(float x, float y, float z, 
		      float n0, float n1, float n2, unsigned int col)
{
  List_Add(vertices, &x);
  List_Add(vertices, &y);
  List_Add(vertices, &z);

  char c0 = float2char(n0);
  char c1 = float2char(n1);
  char c2 = float2char(n2);
  List_Add(normals, &c0);
  List_Add(normals, &c1);
  List_Add(normals, &c2);

  unsigned char r = CTX.UNPACK_RED(col);
  unsigned char g = CTX.UNPACK_GREEN(col);
  unsigned char b = CTX.UNPACK_BLUE(col);
  unsigned char a = CTX.UNPACK_ALPHA(col);
  List_Add(colors, &r);
  List_Add(colors, &g);
  List_Add(colors, &b);
  List_Add(colors, &a);
}

void VertexArray::add(float x, float y, float z, unsigned int col)
{
  List_Add(vertices, &x);
  List_Add(vertices, &y);
  List_Add(vertices, &z);

  unsigned char r = CTX.UNPACK_RED(col);
  unsigned char g = CTX.UNPACK_GREEN(col);
  unsigned char b = CTX.UNPACK_BLUE(col);
  unsigned char a = CTX.UNPACK_ALPHA(col);
  List_Add(colors, &r);
  List_Add(colors, &g);
  List_Add(colors, &b);
  List_Add(colors, &a);
}

static double theeye[3];

int compareTriEye(const void *a, const void *b)
{
  float *q = (float*)a;
  float *w = (float*)b;
  double d, dq, dw, cgq[3] = { 0., 0., 0. }, cgw[3] = { 0., 0., 0.};
  for(int i = 0; i < 3; i++) {
    cgq[0] += q[3*i];
    cgq[1] += q[3*i + 1];
    cgq[2] += q[3*i + 2];
    cgw[0] += w[3*i];
    cgw[1] += w[3*i + 1];
    cgw[2] += w[3*i + 2];
  }
  prosca(theeye, cgq, &dq);
  prosca(theeye, cgw, &dw);
  d = dq - dw;
  if(d > 0)
    return 1;
  if(d < 0)
    return -1;
  return 0;
}

void VertexArray::sort(double eye[3])
{
  // sort assumes that the all the arrays are filled
  if(!List_Nbr(vertices) || !List_Nbr(normals) || !List_Nbr(colors))
    return;

  if(type != 3){
    Msg(GERROR, "VertexArray sort only implemented for triangles");
    return;
  }
  
  theeye[0] = eye[0];
  theeye[1] = eye[1];
  theeye[2] = eye[2];

  int num = n() / 3;
  int nb = List_Nbr(vertices) + List_Nbr(normals) + List_Nbr(colors);
  float *tmp = new float[nb];
  
  int iv = 0, in = 0, ic = 0, k = 0;
  for(int i = 0; i < num; i++){
    for(int j = 0; j < 9; j++)
      List_Read(vertices, iv++, &tmp[k++]);
    for(int j = 0; j < 9; j++)
      List_Read(normals, in++, &tmp[k++]);
    for(int j = 0; j < 12; j++){
      unsigned char c;
      List_Read(colors, ic++, &c);
      tmp[k++] = c;
    }
  }

  List_Reset(vertices);
  List_Reset(normals);
  List_Reset(colors);

  qsort(tmp, num, (9+9+12)*sizeof(float), compareTriEye);

  k = 0;
  for(int i = 0; i < num; i++){
    for(int j = 0; j < 9; j++)
      List_Add(vertices, &tmp[k++]);
    for(int j = 0; j < 9; j++)
      List_Add(normals, &tmp[k++]);
    for(int j = 0; j < 12; j++){
      unsigned char c = (unsigned char)tmp[k++];
      List_Add(colors, &c);
    }
  }  

  delete [] tmp;
}

