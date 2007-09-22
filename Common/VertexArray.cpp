// $Id: VertexArray.cpp,v 1.26 2007-09-22 20:35:18 geuzaine Exp $
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

#include <algorithm>
#include "VertexArray.h"
#include "Context.h"
#include "Numeric.h"

extern Context_T CTX;

VertexArray::VertexArray(int numVerticesPerElement, int numElements) 
  : _numVerticesPerElement(numVerticesPerElement)
{
  int nb = (numElements ? numElements : 1) * _numVerticesPerElement;
  _vertices.reserve(nb * 3);
  _normals.reserve(nb * 3);
  _colors.reserve(nb * 4);
}

void VertexArray::add(float x, float y, float z, float n0, float n1, float n2,
		      unsigned int col, MElement *ele)
{
  _vertices.push_back(x);
  _vertices.push_back(y);
  _vertices.push_back(z);

  // storing the normals as bytes hurts rendering performance, but it
  // significantly reduces the memory footprint
  char c0 = float2char(n0);
  char c1 = float2char(n1);
  char c2 = float2char(n2);
  _normals.push_back(c0);
  _normals.push_back(c1);
  _normals.push_back(c2);

  unsigned char r = CTX.UNPACK_RED(col);
  unsigned char g = CTX.UNPACK_GREEN(col);
  unsigned char b = CTX.UNPACK_BLUE(col);
  unsigned char a = CTX.UNPACK_ALPHA(col);
  _colors.push_back(r);
  _colors.push_back(g);
  _colors.push_back(b);
  _colors.push_back(a);

  if(ele && CTX.pick_elements) _elements.push_back(ele);
}

void VertexArray::add(double *x, double *y, double *z, SVector3 *n,
		      unsigned int *col, MElement *ele, bool unique)
{
  int npe = getNumVerticesPerElement();

  /*
  bool boundary = true;
  if(boundary && npe == 3){
    ElementData<3> e(x, y, z, n, col, ele);
    std::set<ElementData<3>, ElementDataLessThan<3> >::iterator it = _data3.find(e);
    if(it == _data3.end())
      _data3.insert(e);
    else
      _data3.erase(it);
    return;
  }
  */

  if(unique){
    Barycenter pc(0., 0., 0.);
    for(int i = 0; i < npe; i++)
      pc += Barycenter(x[i], y[i], z[i]);
    if(_barycenters.find(pc) != _barycenters.end()) return;
    _barycenters.insert(pc);
  }
  
  for(int i = 0; i < npe; i++)
    add(x[i], y[i], z[i], n[i].x(), n[i].y(), n[i].z(), col[i], ele);
}

void VertexArray::finalize()
{
  if(_data3.size()){
    std::set<ElementData<3>, ElementDataLessThan<3> >::iterator it = _data3.begin();
    for(; it != _data3.end(); it++)
      for(int i = 0; i < 3; i++)
	add(it->x(i), it->y(i), it->z(i), it->nx(i), it->ny(i), it->nz(i), 
	    it->col(i), it->ele());
    _data3.clear();
  }
  _barycenters.clear();
}

class AlphaElement {
public:
  AlphaElement(float *vp, char *np, unsigned char *cp) : v(vp), n(np), c(cp) {}
  float *v;
  char *n;
  unsigned char *c;
};

class AlphaElementLessThan {
 public:
  static int numVertices;
  static double eye[3];
  bool operator()(const AlphaElement &e1, const AlphaElement &e2) const
  {
    double cg1[3] = { 0., 0., 0. }, cg2[3] = { 0., 0., 0.};
    for(int i = 0; i < numVertices; i++) {
      cg1[0] += e1.v[3 * i];
      cg1[1] += e1.v[3 * i + 1];
      cg1[2] += e1.v[3 * i + 2];
      cg2[0] += e2.v[3 * i];
      cg2[1] += e2.v[3 * i + 1];
      cg2[2] += e2.v[3 * i + 2];
    }
    double d1, d2;
    prosca(eye, cg1, &d1);
    prosca(eye, cg2, &d2);
    if(d1 < d2)
      return true;
    return false;
  }
};

int AlphaElementLessThan::numVertices = 0;
double AlphaElementLessThan::eye[3] = {0., 0., 0.};

void VertexArray::sort(double x, double y, double z)
{
  // This simplementation is pretty bad: it copies the whole data
  // twice. We should think about a more efficient way to sort the
  // three arrays in place.

  int npe = getNumVerticesPerElement();
  int n = getNumVertices() / npe;

  AlphaElementLessThan::numVertices = npe;
  AlphaElementLessThan::eye[0] = x;
  AlphaElementLessThan::eye[1] = y;
  AlphaElementLessThan::eye[2] = z;

  std::vector<AlphaElement> elements;
  elements.reserve(n);
  if(_normals.size())
    for(int i = 0; i < n; i++)
      elements.push_back(AlphaElement(&_vertices[3 * npe * i], 
				      &_normals[3 * npe * i], 
				      &_colors[4 * npe * i]));
  else
    for(int i = 0; i < n; i++)
      elements.push_back(AlphaElement(&_vertices[3 * npe * i], 
				      0, 
				      &_colors[4 * npe * i]));
  
  std::sort(elements.begin(), elements.end(), AlphaElementLessThan());

  std::vector<float> sortedVertices;
  std::vector<char> sortedNormals;
  std::vector<unsigned char> sortedColors;
  sortedVertices.reserve(_vertices.size());
  sortedNormals.reserve(_normals.size());
  sortedColors.reserve(_colors.size());

  for(int i = 0; i < n; i++){
    for(int j = 0; j < npe; j++){
      for(int k = 0; k < 3; k++){
	sortedVertices.push_back(elements[i].v[3 * j + k]);
	if(elements[i].v)
	  sortedNormals.push_back(elements[i].n[3 * j + k]);
      }
      for(int k = 0; k < 4; k++){
	sortedColors.push_back(elements[i].c[4 * j + k]);
      }
    }
  }

  _vertices = sortedVertices;
  _normals = sortedNormals;
  _colors = sortedColors;
}
