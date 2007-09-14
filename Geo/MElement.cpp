// $Id: MElement.cpp,v 1.41 2007-09-14 01:54:20 geuzaine Exp $
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

#include <math.h>
#include "MElement.h"
#include "GEntity.h"
#include "GFace.h"
#include "Numeric.h"
#include "Message.h"
#include "Context.h"

extern Context_T CTX;

int MElement::_globalNum = 0;
double MElementLessThanLexicographic::tolerance = 1.e-6;

char MElement::getVisibility()
{
  if(CTX.hide_unselected && _visible < 2) return false;
  return _visible; 
}

double MElement::minEdge()
{
  double m = 1.e25;
  for(int i = 0; i < getNumEdges(); i++){
    MEdge e = getEdge(i);
    m = std::min(m, e.getVertex(0)->distance(e.getVertex(1)));
  }
  return m;
}

double MElement::maxEdge()
{
  double m = 0.;
  for(int i = 0; i < getNumEdges(); i++){
    MEdge e = getEdge(i);
    m = std::max(m, e.getVertex(0)->distance(e.getVertex(1)));
  }
  return m;
}

double MElement::rhoShapeMeasure()
{
  double min = minEdge();
  double max = maxEdge();
  if(max)
    return min / max;
  else
    return 0.;
}

int MElement::getNumEdgesRep()
{
  return getNumEdges();
}

int MElement::getEdgeRep(int num, double *x, double *y, double *z, SVector3 *n)
{
  MEdge e = getEdge(num);
  SVector3 normal = (getDim() == 2) ? getFace(0).normal() : e.normal();
  for(int i = 0; i < 2; i++){
    MVertex *v = e.getVertex(i);
    x[i] = v->x();
    y[i] = v->y();
    z[i] = v->z();
    n[i] = normal;
  }
  return 2;
}

int MElement::_getEdgeRep(const int edge[2], double *x, double *y, double *z,
			  SVector3 *n, int faceIndex)
{
  MEdge e(getVertex(edge[0]), getVertex(edge[1]));
  SVector3 normal = (faceIndex >= 0) ? getFace(faceIndex).normal() : e.normal();
  for(int i = 0; i < 2; i++){
    MVertex *v = e.getVertex(i);
    x[i] = v->x(); 
    y[i] = v->y(); 
    z[i] = v->z();
    n[i] = normal;
  }
  return 2;
}

int MElement::getNumFacesRep()
{
  return getNumFaces();
}

int MElement::getFaceRep(int num, double *x, double *y, double *z, SVector3 *n)
{
  MFace f = getFace(num);
  SVector3 normal = f.normal();
  for(int i = 0; i < f.getNumVertices(); i++){
    MVertex *v = f.getVertex(i);
    x[i] = v->x();
    y[i] = v->y();
    z[i] = v->z();
    n[i] = normal;
  }
  return f.getNumVertices();
}

int MElement::_getFaceRep(const int face[3], double *x, double *y, double *z,
			  SVector3 *n)
{
  for(int i = 0; i < 3; i++){
    MVertex *v = getVertex(face[i]);
    x[i] = v->x(); 
    y[i] = v->y(); 
    z[i] = v->z();
  }
  SVector3 t1(x[1] - x[0], y[1] - y[0], z[1] - z[0]);
  SVector3 t2(x[2] - x[0], y[2] - y[0], z[2] - z[0]);
  SVector3 normal = crossprod(t1, t2);
  normal.normalize();
  for(int i = 0; i < 3; i++) n[i] = normal;
  return 3;
}

double MTetrahedron::gammaShapeMeasure()
{
  double p0[3] = { _v[0]->x(), _v[0]->y(), _v[0]->z() };
  double p1[3] = { _v[1]->x(), _v[1]->y(), _v[1]->z() };
  double p2[3] = { _v[2]->x(), _v[2]->y(), _v[2]->z() };
  double p3[3] = { _v[3]->x(), _v[3]->y(), _v[3]->z() };
  double s1 = fabs(triangle_area(p0, p1, p2));
  double s2 = fabs(triangle_area(p0, p2, p3));
  double s3 = fabs(triangle_area(p0, p1, p3));
  double s4 = fabs(triangle_area(p1, p2, p3));
  double rhoin = 3. * fabs(getVolume()) / (s1 + s2 + s3 + s4);
  return 12. * rhoin / (sqrt(6.) * maxEdge());
}

double MTetrahedron::etaShapeMeasure()
{
  double lij2 = 0.;
  for(int i = 0; i <= 3; i++) {
    for(int j = i + 1; j <= 3; j++) {
      double lij = _v[i]->distance(_v[j]);
      lij2 += lij * lij;
    }
  }
  double v = fabs(getVolume());
  return 12. * pow(0.9 * v * v, 1./3.) / lij2;
}

SPoint3 MElement::barycenter()
{
  SPoint3 p(0., 0., 0.);
  int n = getNumVertices();
  for(int i = 0; i < n; i++) {
    MVertex *v = getVertex(i);
    p[0] += v->x();
    p[1] += v->y();
    p[2] += v->z();
  }
  p[0] /= (double)n;
  p[1] /= (double)n;
  p[2] /= (double)n;
  return p;
}

std::string MElement::getInfoString()
{
  char tmp[256];
  sprintf(tmp, "Element %d", getNum());
  return std::string(tmp);
}

void MElement::writeMSH(FILE *fp, double version, bool binary, int num, 
			int elementary, int physical)
{
  int type = getTypeForMSH();

  if(!type) return;

  // if necessary, change the ordering of the vertices to get positive
  // volume
  setVolumePositive();
  int n = getNumVertices();

  if(!binary){
    fprintf(fp, "%d %d", num ? num : _num, type);
    if(version < 2.0)
      fprintf(fp, " %d %d %d", abs(physical), elementary, n);
    else
      fprintf(fp, " 3 %d %d %d", abs(physical), elementary, _partition);
  }
  else{
    int tags[4] = {num ? num : _num, abs(physical), elementary, _partition};
    fwrite(tags, sizeof(int), 4, fp);
  }

  if(physical < 0) revert();

  int verts[30];
  for(int i = 0; i < n; i++)
    verts[i] = getVertex(i)->getNum();

  if(!binary){
    for(int i = 0; i < n; i++)
      fprintf(fp, " %d", verts[i]);
    fprintf(fp, "\n");
  }
  else{
    fwrite(verts, sizeof(int), n, fp);
  }

  if(physical < 0) revert();
}

void MElement::writePOS(FILE *fp, double scalingFactor, int elementary)
{
  const char *str = getStringForPOS();
  if(!str) return;

  int n = getNumVertices();
  double gamma = gammaShapeMeasure();
  double eta = etaShapeMeasure();
  double rho = rhoShapeMeasure();
  fprintf(fp, "%s(", str);
  for(int i = 0; i < n; i++){
    if(i) fprintf(fp, ",");
    fprintf(fp, "%g,%g,%g", getVertex(i)->x() * scalingFactor, 
	    getVertex(i)->y() * scalingFactor, getVertex(i)->z() * scalingFactor);
  }
  fprintf(fp, "){");
  for(int i = 0; i < n; i++)
    fprintf(fp, "%d,", elementary);
  for(int i = 0; i < n; i++)
    fprintf(fp, "%d,", getNum());
  for(int i = 0; i < n; i++)
    fprintf(fp, "%g,", gamma);
  for(int i = 0; i < n; i++)
    fprintf(fp, "%g,", eta);
  for(int i = 0; i < n; i++){
    if(i == n - 1)
      fprintf(fp, "%g", rho);
    else
      fprintf(fp, "%g,", rho);
  }
  fprintf(fp, "};\n");
}

void MElement::writeSTL(FILE *fp, bool binary, double scalingFactor)
{
  if(getNumEdges() != 3 && getNumEdges() != 4) return;
  int qid[3] = {0, 2, 3};
  SVector3 n = getFace(0).normal();
  if(!binary){
    fprintf(fp, "facet normal %g %g %g\n", n[0], n[1], n[2]);
    fprintf(fp, "  outer loop\n");
    for(int j = 0; j < 3; j++)
      fprintf(fp, "    vertex %g %g %g\n", 
	      getVertex(j)->x() * scalingFactor, 
	      getVertex(j)->y() * scalingFactor, 
	      getVertex(j)->z() * scalingFactor);
    fprintf(fp, "  endloop\n");
    fprintf(fp, "endfacet\n");
    if(getNumVertices() == 4){
      fprintf(fp, "facet normal %g %g %g\n", n[0], n[1], n[2]);
      fprintf(fp, "  outer loop\n");
      for(int j = 0; j < 3; j++)
	fprintf(fp, "    vertex %g %g %g\n", 
		getVertex(qid[j])->x() * scalingFactor, 
		getVertex(qid[j])->y() * scalingFactor, 
		getVertex(qid[j])->z() * scalingFactor);
      fprintf(fp, "  endloop\n");
      fprintf(fp, "endfacet\n");
    }
  }
  else{
    char data[50];
    float *coords = (float*)data;
    coords[0] = n[0];
    coords[1] = n[1];
    coords[2] = n[2];
    for(int j = 0; j < 3; j++){
      coords[3 + 3 * j] = getVertex(j)->x() * scalingFactor;
      coords[3 + 3 * j + 1] = getVertex(j)->y() * scalingFactor;
      coords[3 + 3 * j + 2] = getVertex(j)->z() * scalingFactor;
    }
    data[48] = data[49] = 0;
    fwrite(data, sizeof(char), 50, fp);
    if(getNumVertices() == 4){
      for(int j = 0; j < 3; j++){
	coords[3 + 3 * j] = getVertex(qid[j])->x() * scalingFactor;
	coords[3 + 3 * j + 1] = getVertex(qid[j])->y() * scalingFactor;
	coords[3 + 3 * j + 2] = getVertex(qid[j])->z() * scalingFactor;
      }
      fwrite(data, sizeof(char), 50, fp);
    }
  }
}

void MElement::writeVRML(FILE *fp)
{
  for(int i = 0; i < getNumVertices(); i++)
    fprintf(fp, "%d,", getVertex(i)->getNum() - 1);
  fprintf(fp, "-1,\n");
}

void MElement::writeUNV(FILE *fp, int num, int elementary, int physical)
{
  int type = getTypeForUNV();
  if(!type) return;

  setVolumePositive();
  int n = getNumVertices();
  int physical_property = elementary;
  int material_property = abs(physical);
  int color = 7;
  fprintf(fp, "%10d%10d%10d%10d%10d%10d\n",
	  num ? num : _num, type, physical_property, material_property, color, n);
  if(type == 21 || type == 24) // linear beam or parabolic beam
    fprintf(fp, "%10d%10d%10d\n", 0, 0, 0);

  if(physical < 0) revert();

  for(int k = 0; k < n; k++) {
    fprintf(fp, "%10d", getVertexUNV(k)->getNum());
    if(k % 8 == 7)
      fprintf(fp, "\n");
  }
  if(n - 1 % 8 != 7)
    fprintf(fp, "\n");

  if(physical < 0) revert();
}

void MElement::writeMESH(FILE *fp, int elementary)
{
  for(int i = 0; i < getNumVertices(); i++)
    fprintf(fp, " %d", getVertex(i)->getNum());
  fprintf(fp, " %d\n", elementary);
}

void MElement::writeBDF(FILE *fp, int format, int elementary)
{
  const char *str = getStringForBDF();
  if(!str) return;

  setVolumePositive();
  int n = getNumVertices();
  const char *cont[4] = {"E", "F", "G", "H"};
  int ncont = 0;
  
  if(format == 0){ // free field format
    fprintf(fp, "%s,%d,%d", str, _num, elementary);
    for(int i = 0; i < n; i++){
      fprintf(fp, ",%d", getVertex(i)->getNum());
      if(i != n - 1 && !((i + 3) % 8)){
	fprintf(fp, ",+%s%d\n+%s%d", cont[ncont], _num, cont[ncont], _num);
	ncont++;
      }
    }
    if(n == 2) // CBAR
      fprintf(fp, ",0.,0.,0.");
    fprintf(fp, "\n");
  }
  else{ // small or large field format
    fprintf(fp, "%-8s%-8d%-8d", str, _num, elementary);
    for(int i = 0; i < n; i++){
      fprintf(fp, "%-8d", getVertex(i)->getNum());
      if(i != n - 1 && !((i + 3) % 8)){
	fprintf(fp, "+%s%-6d\n+%s%-6d", cont[ncont], _num, cont[ncont], _num);
	ncont++;
      }
    }
    if(n == 2) // CBAR
      fprintf(fp, "%-8s%-8s%-8s", "0.", "0.", "0.");
    fprintf(fp, "\n");
  }
}

bool MTriangle::invertmappingXY(double *p, double *uv, double tol)
{
  double mat[2][2];
  double b[2];
  getMat(mat);
  b[0] = p[0] - getVertex(0)->x();
  b[1] = p[1] - getVertex(0)->y();
  sys2x2(mat, b, uv);

  if(uv[0] >= -tol && 
     uv[1] >= -tol && 
     uv[0] <= 1. + tol && 
     uv[1] <= 1. + tol && 
     1. - uv[0] - uv[1] > -tol) {
    return true;
  }
  return false; 
}

bool MTriangle::invertmappingUV(GFace* gf, double *p, double *uv, double tol)
{
  double mat[2][2];
  double b[2];
  double u0, v0, u1, v1, u2, v2;

  parametricCoordinates(getVertex(0), gf, u0, v0);
  parametricCoordinates(getVertex(1), gf, u1, v1);
  parametricCoordinates(getVertex(2), gf, u2, v2);
  
  mat[0][0] = u1 - u0;
  mat[0][1] = u2 - u0;
  mat[1][0] = v1 - v0;
  mat[1][1] = v2 - v0;

  b[0] = p[0] - u0;
  b[1] = p[1] - v0;
  sys2x2(mat, b, uv);

  if(uv[0] >= -tol && 
     uv[1] >= -tol && 
     uv[0] <= 1. + tol && 
     uv[1] <= 1. + tol && 
     1. - uv[0] - uv[1] > -tol) {
    return true;
  }
  return false; 
}

double MTriangle::getSurfaceUV(GFace *gf)
{
  double u3, v3, u1, v1, u2, v2;

  parametricCoordinates(getVertex(0), gf, u1, v1);
  parametricCoordinates(getVertex(1), gf, u2, v2);
  parametricCoordinates(getVertex(2), gf, u3, v3);

  const double vv1 [2] = {u2 - u1, v2 - v1};
  const double vv2 [2] = {u3 - u1, v3 - v1};

  double s = vv1[0] * vv2[1] - vv1[1] * vv2[0]; 
  return s * 0.5;
}

double MTriangle::getSurfaceXY() const
{
  const double x1 = _v[0]->x();
  const double x2 = _v[1]->x();
  const double x3 = _v[2]->x();
  const double y1 = _v[0]->y();
  const double y2 = _v[1]->y();
  const double y3 = _v[2]->y();

  const double v1 [2] = {x2 - x1, y2 - y1};
  const double v2 [2] = {x3 - x1, y3 - y1};

  double s = v1[0] * v2[1] - v1[1] * v2[0]; 
  return s * 0.5;
}

void MTriangle::circumcenterXYZ(double *p1, double *p2, double *p3, 
				double *res, double *uv)
{
  double v1[3] = {p2[0] - p1[0], p2[1] - p1[1], p2[2] - p1[2]};
  double v2[3] = {p3[0] - p1[0], p3[1] - p1[1], p3[2] - p1[2]};
  double vx[3] = {p2[0] - p1[0], p2[1] - p1[1], p2[2] - p1[2]};
  double vy[3] = {p3[0] - p1[0], p3[1] - p1[1], p3[2] - p1[2]};
  double vz[3]; prodve(vx, vy, vz); prodve(vz, vx, vy);
  norme(vx); norme(vy); norme(vz);
  double p1P[2] = {0.0, 0.0};
  double p2P[2]; prosca(v1, vx, &p2P[0]); prosca(v1, vy, &p2P[1]);
  double p3P[2]; prosca(v2, vx, &p3P[0]); prosca(v2, vy, &p3P[1]);
  double resP[2];

  circumcenterXY(p1P, p2P, p3P,resP);

  if(uv){
    double mat[2][2] = {{p2P[0] - p1P[0], p3P[0] - p1P[0]},
			{p2P[1] - p1P[1], p3P[1] - p1P[1]}};
    double rhs[2] = {resP[0] - p1P[0], resP[1] - p1P[1]};
    sys2x2(mat, rhs, uv);
  }
  
  res[0] = p1[0] + resP[0] * vx[0] + resP[1] * vy[0];
  res[1] = p1[1] + resP[0] * vx[1] + resP[1] * vy[1];
  res[2] = p1[2] + resP[0] * vx[2] + resP[1] * vy[2];
}

void MTriangle::circumcenterXY(double *p1, double *p2, double *p3, double *res)
{
  double d, a1, a2, a3;

  const double x1 = p1[0];
  const double x2 = p2[0];
  const double x3 = p3[0];
  const double y1 = p1[1];
  const double y2 = p2[1];
  const double y3 = p3[1];

  d = 2. * (double)(y1 * (x2 - x3) + y2 * (x3 - x1) + y3 * (x1 - x2));
  if(d == 0.0) {
    Msg(GERROR, "Colinear points in circum circle computation");
    res[0] = res[1] = -99999.;
    return ;
  }

  a1 = x1 * x1 + y1 * y1;
  a2 = x2 * x2 + y2 * y2;
  a3 = x3 * x3 + y3 * y3;
  res[0] = (double)((a1 * (y3 - y2) + a2 * (y1 - y3) + a3 * (y2 - y1)) / d);
  res[1] = (double)((a1 * (x2 - x3) + a2 * (x3 - x1) + a3 * (x1 - x2)) / d);
}

void MTriangle::circumcenterUV(GFace *gf, double *res)
{
  double u3, v3, u1, v1, u2, v2;

  parametricCoordinates(getVertex(0), gf, u1, v1);
  parametricCoordinates(getVertex(1), gf, u2, v2);
  parametricCoordinates(getVertex(2), gf, u3, v3);

  double p1[2] = {u1, v1};
  double p2[2] = {u2, v2};
  double p3[2] = {u3, v3};

  circumcenterXY(p1, p2, p3, res);
}

void MTriangle::circumcenterXY(double *res) const
{
  double p1[2] = {_v[0]->x(), _v[0]->y()};
  double p2[2] = {_v[1]->x(), _v[1]->y()};
  double p3[2] = {_v[2]->x(), _v[2]->y()};
  circumcenterXY(p1, p2, p3, res);
}

int P1[3][2] = {
  {0,0},
  {1,0},
  {0,1}
};

int P2[6][2] = {
  {0,0},
  {1,0},
  {0,1},
  {2,0},
  {0,2},
  {1,1}
};

int P3[9][2] = {
  {0,0},
  {1,0},
  {0,1},
  {2,0},
  {0,2},
  {3,0},
  {0,3},
  {2,1},
  {1,2}
};

int P4[12][2] = {
  {0,0},
  {1,0},
  {0,1},
  {2,0},
  {0,2},
  {3,0},
  {0,3},
  {4,0},
  {0,4},
  {3,1},
  {1,3},
  {2,2}
};

int P5[15][2] = {
  {0,0},
  {1,0},
  {0,1},
  {2,0},
  {0,2},
  {3,0},
  {0,3},
  {4,0},
  {0,4},
  {5,0},
  {0,5},
  {4,1},
  {3,2},
  {2,3},
  {1,4}
};

double coef1[3][3]={
  {  1.00000000,  -1.00000000,  -1.00000000},
  {  0.00000000,   1.00000000,   0.00000000},
  {  0.00000000,   0.00000000,   1.00000000}
};

double coef2[6][6]={
  {  1.00000000,  -3.00000000,  -3.00000000,   2.00000000,   2.00000000,   4.00000000},
  {  0.00000000,  -1.00000000,   0.00000000,   2.00000000,  -0.00000000,  -0.00000000},
  {  0.00000000,   0.00000000,  -1.00000000,  -0.00000000,   2.00000000,  -0.00000000},
  {  0.00000000,   4.00000000,   0.00000000,  -4.00000000,  -0.00000000,  -4.00000000},
  {  0.00000000,   0.00000000,   0.00000000,  -0.00000000,  -0.00000000,   4.00000000},
  {  0.00000000,   0.00000000,   4.00000000,  -0.00000000,  -4.00000000,  -4.00000000}
};

double coef3[9][9]={
  {  1.00000000,  -5.50000000,  -5.50000000,   9.00000000,   9.00000000,  -4.50000000,
     -4.50000000,   4.50000000,   4.50000000},
  {  0.00000000,   1.00000000,   0.00000000,  -4.50000000,  -0.00000000,   4.50000000,
     -0.00000000,  -0.00000000,  -0.00000000},
  {  0.00000000,   0.00000000,   1.00000000,  -0.00000000,  -4.50000000,  -0.00000000,
     4.50000000,  -0.00000000,  -0.00000000},
  {  0.00000000,   9.00000000,   0.00000000, -22.50000000,  -0.00000000,  13.50000000,
     0.00000000,   4.50000000,  -9.00000000},
  {  0.00000000,  -4.50000000,  -0.00000000,  18.00000000,   0.00000000, -13.50000000,
     -0.00000000,  -9.00000000,   4.50000000},
  {  0.00000000,   0.00000000,   0.00000000,  -0.00000000,  -0.00000000,  -0.00000000,
     0.00000000,   9.00000000,  -4.50000000},
  {  0.00000000,   0.00000000,  -0.00000000,  -0.00000000,   0.00000000,  -0.00000000,
     -0.00000000,  -4.50000000,   9.00000000},
  {  0.00000000,   0.00000000,  -4.50000000,  -0.00000000,  18.00000000,  -0.00000000,
     -13.50000000,   4.50000000,  -9.00000000},
  {  0.00000000,   0.00000000,   9.00000000,  -0.00000000, -22.50000000,  -0.00000000,
     13.50000000,  -9.00000000,   4.50000000}
};

double coef4[12][12]={
{  1.00000000,  -8.33333333,  -8.33333333,  23.33333333,  23.33333333, -26.66666667,
   -26.66666667,  10.66666667,  10.66666667,   9.33333333,   9.33333333,  -2.66666667},
{  0.00000000,  -1.00000000,   0.00000000,   7.33333333,  -0.00000000, -16.00000000,
   0.00000000,  10.66666667,  -0.00000000,  -0.00000000,  -0.00000000,  -0.00000000},
{  0.00000000,   0.00000000,  -1.00000000,  -0.00000000,   7.33333333,  -0.00000000,
   -16.00000000,  -0.00000000,  10.66666667,  -0.00000000,  -0.00000000,  -0.00000000},
{  0.00000000,  16.00000000,  -0.00000000, -69.33333333,   0.00000000,  96.00000000,
    -0.00000000, -42.66666667,   0.00000000,  -5.33333333, -16.00000000,  21.33333333},
{  0.00000000, -12.00000000,   0.00000000,  76.00000000,  -0.00000000, -128.00000000,
    0.00000000,  64.00000000,  -0.00000000,  12.00000000,  12.00000000, -40.00000000},
{  0.00000000,   5.33333333,  -0.00000000, -37.33333333,   0.00000000,  74.66666667,
    -0.00000000, -42.66666667,   0.00000000, -16.00000000,  -5.33333333,  21.33333333},
{  0.00000000,   0.00000000,   0.00000000,  -0.00000000,  -0.00000000,  -0.00000000,
    0.00000000,  -0.00000000,  -0.00000000,  16.00000000,   5.33333333, -21.33333333},
{  0.00000000,   0.00000000,  -0.00000000,  -0.00000000,   0.00000000,  -0.00000000,
    -0.00000000,  -0.00000000,   0.00000000, -12.00000000, -12.00000000,  40.00000000},
{  0.00000000,   0.00000000,   0.00000000,  -0.00000000,  -0.00000000,  -0.00000000,
    0.00000000,  -0.00000000,  -0.00000000,   5.33333333,  16.00000000, -21.33333333},
{  0.00000000,   0.00000000,   5.33333333,  -0.00000000, -37.33333333,  -0.00000000,
    74.66666667,  -0.00000000, -42.66666667,  -5.33333333, -16.00000000,  21.33333333},
{  0.00000000,   0.00000000, -12.00000000,  -0.00000000,  76.00000000,  -0.00000000,
    -128.00000000,  -0.00000000,  64.00000000,  12.00000000,  12.00000000, -40.00000000},
{  0.00000000,   0.00000000,  16.00000000,  -0.00000000, -69.33333333,  -0.00000000,
    96.00000000,  -0.00000000, -42.66666667, -16.00000000,  -5.33333333,  21.33333333}
};
double coef5[15][15]={
{  1.00000000, -11.41666667, -11.41666667,  46.87500000,  46.87500000, -88.54166667,
   -88.54166667,  78.12500000,  78.12500000, -26.04166667, -26.04166667,  10.41666667,
   5.20833333,   5.20833333,  10.41666667},
{  0.00000000,   1.00000000,  -0.00000000, -10.41666667,   0.00000000,  36.45833333,
   -0.00000000, -52.08333333,   0.00000000,  26.04166667,   0.00000000,  -0.00000000,
   0.00000000,  -0.00000000,   0.00000000},
{  0.00000000,   0.00000000,   1.00000000,  -0.00000000, -10.41666667,  -0.00000000,
   36.45833333,  -0.00000000, -52.08333333,   0.00000000,  26.04166667,  -0.00000000,
   0.00000000,  -0.00000000,   0.00000000},
{  0.00000000,  25.00000000,  -0.00000000, -160.41666667,   0.00000000, 369.79166667,
   -0.00000000, -364.58333333,   0.00000000, 130.20833333,  -0.00000000,   6.25000000,
   -38.54166667,  60.41666667, -25.00000000},
{  0.00000000, -25.00000000,   0.00000000, 222.91666667,  -0.00000000, -614.58333333,
   0.00000000, 677.08333333,  -0.00000000, -260.41666667,   0.00000000, -16.66666667,
   95.83333333, -122.91666667,  25.00000000},
{  0.00000000,  16.66666667,  -0.00000000, -162.50000000,   0.00000000, 510.41666667,
   -0.00000000, -625.00000000,   0.00000000, 260.41666667,  -0.00000000,  25.00000000,
   -122.91666667,  95.83333333, -16.66666667},
{  0.00000000,  -6.25000000,   0.00000000,  63.54166667,  -0.00000000, -213.54166667, 
   0.00000000, 286.45833333,  -0.00000000, -130.20833333,   0.00000000, -25.00000000,
    60.41666667, -38.54166667,   6.25000000},
{  0.00000000,   0.00000000,  -0.00000000,  -0.00000000,   0.00000000,  -0.00000000,
    -0.00000000,  -0.00000000,   0.00000000,   0.00000000,  -0.00000000,  25.00000000,
    -60.41666667,  38.54166667,  -6.25000000},
{  0.00000000,   0.00000000,   0.00000000,  -0.00000000,  -0.00000000,  -0.00000000,
    0.00000000,  -0.00000000,  -0.00000000,   0.00000000,   0.00000000, -25.00000000,
    122.91666667, -95.83333333,  16.66666667},
{  0.00000000,   0.00000000,  -0.00000000,  -0.00000000,   0.00000000,  -0.00000000,
    -0.00000000,  -0.00000000,   0.00000000,   0.00000000,  -0.00000000,  16.66666667,
    -95.83333333, 122.91666667, -25.00000000},
{  0.00000000,   0.00000000,   0.00000000,  -0.00000000,  -0.00000000,  -0.00000000,
    0.00000000,  -0.00000000,  -0.00000000,   0.00000000,   0.00000000,  -6.25000000,
    38.54166667, -60.41666667,  25.00000000},
{  0.00000000,   0.00000000,  -6.25000000,  -0.00000000,  63.54166667,  -0.00000000,
    -213.54166667,  -0.00000000, 286.45833333,   0.00000000, -130.20833333,   6.25000000,
    -38.54166667,  60.41666667, -25.00000000},
{  0.00000000,   0.00000000,  16.66666667,  -0.00000000, -162.50000000,  -0.00000000,
    510.41666667,  -0.00000000, -625.00000000,   0.00000000, 260.41666667, -16.66666667,
    95.83333333, -122.91666667,  25.00000000},
{  0.00000000,   0.00000000, -25.00000000,  -0.00000000, 222.91666667,  -0.00000000,
    -614.58333333,  -0.00000000, 677.08333333,   0.00000000, -260.41666667,  25.00000000,
    -122.91666667,  95.83333333, -16.66666667},
{  0.00000000,   0.00000000,  25.00000000,  -0.00000000, -160.41666667,  -0.00000000,
    369.79166667,  -0.00000000, -364.58333333,   0.00000000, 130.20833333, -25.00000000,
    60.41666667, -38.54166667,   6.25000000}
};

void GradGeomShapeFunctionP1(double u, double v, double grads[6][2]) 
{
  for (int i = 0; i < 3; i++){
    grads[i][0] = 0;
    grads[i][1] = 0;
    for(int j = 0; j < 3; j++){
      if(P1[j][0] > 0) grads[i][0] += coef1[i][j] * pow(u,P1[j][0] - 1) * pow(v, P1[j][1]);
      if(P1[j][1] > 0) grads[i][1] += coef1[i][j] * pow(u,P1[j][0]) * pow(v, P1[j][1] - 1);
    }
  }
}

void GradGeomShapeFunctionP2(double u, double v, double grads[6][2]) 
{
  for(int i = 0; i < 6; i++){
    grads[i][0] = 0;
    grads[i][1] = 0;
    for (int j = 0; j < 6; j++){
      if(P2[j][0] > 0) grads[i][0] += coef2[i][j] * pow(u, P2[j][0] - 1) * pow(v, P2[j][1]);
      if(P2[j][1] > 0) grads[i][1] += coef2[i][j] * pow(u, P2[j][0]) * pow(v, P2[j][1] - 1);
    }
  }
}

void GradGeomShapeFunctionP3 (double u, double v, double grads[9][2])
{
  for(int i = 0; i < 9; i++){
    grads[i][0] = 0;
    grads[i][1] = 0;
    for(int j = 0; j < 9; j++){
      if(P3[j][0] > 0) grads[i][0] += coef3[i][j] * pow(u, P3[j][0] - 1) * pow(v, P3[j][1]);
      if(P3[j][1] > 0) grads[i][1] += coef3[i][j] * pow(u, P3[j][0]) * pow(v, P3[j][1] - 1);
    }
  }
}

void GradGeomShapeFunctionP4(double u, double v, double grads[12][2]) 
{
  for(int i = 0; i < 12; i++){
    grads[i][0] = 0;
    grads[i][1] = 0;
    for(int j = 0; j < 12; j++){
      if(P4[j][0] > 0) grads[i][0] += coef4[i][j] * pow(u, P4[j][0] - 1) * pow(v, P4[j][1]);
      if(P4[j][1] > 0) grads[i][1] += coef4[i][j] * pow(u, P4[j][0]) * pow(v, P4[j][1] - 1);
    }
  }
}

void GradGeomShapeFunctionP5(double u, double v, double grads[15][2]) 
{
  for(int i = 0; i < 15; i++){
    grads[i][0] = 0;
    grads[i][1] = 0;
    for (int j = 0; j < 15; j++){
      if(P5[j][0] > 0) grads[i][0] += coef5[i][j] * pow(u, P5[j][0] - 1) * pow(v, P5[j][1]);
      if(P5[j][1] > 0) grads[i][1] += coef5[i][j] * pow(u, P5[j][0]) * pow(v, P5[j][1] - 1);
    }
  }
}

void MTriangle::jac(int ord, MVertex *vs[], double uu, double vv, double j[2][2])
{
  double grads[256][2];
  switch(ord){
  case 1: GradGeomShapeFunctionP1(uu, vv, grads); break;
  case 2: GradGeomShapeFunctionP2(uu, vv, grads); break;
  case 3: GradGeomShapeFunctionP3(uu, vv, grads); break;
  case 4: GradGeomShapeFunctionP4(uu, vv, grads); break;
  case 5: GradGeomShapeFunctionP5(uu, vv, grads); break;
  default: throw;
  }
  j[0][0] = 0 ; for(int i = 0; i < 3; i++) j[0][0] += grads [i][0] * _v[i] -> x();
  j[1][0] = 0 ; for(int i = 0; i < 3; i++) j[1][0] += grads [i][1] * _v[i] -> x();
  j[0][1] = 0 ; for(int i = 0; i < 3; i++) j[0][1] += grads [i][0] * _v[i] -> y();
  j[1][1] = 0 ; for(int i = 0; i < 3; i++) j[1][1] += grads [i][1] * _v[i] -> y();
  for(int i = 3; i < 3 * ord; i++) j[0][0] += grads[i][0] * vs[i - 3] -> x();
  for(int i = 3; i < 3 * ord; i++) j[1][0] += grads[i][1] * vs[i - 3] -> x();
  for(int i = 3; i < 3 * ord; i++) j[0][1] += grads[i][0] * vs[i - 3] -> y();
  for(int i = 3; i < 3 * ord; i++) j[1][1] += grads[i][1] * vs[i - 3] -> y();
}

void MTriangleN::jac(double uu, double vv , double j[2][2])  
{
  MTriangle::jac(_order, &(*(_vs.begin())), uu, vv, j);
}

void MTriangle6::jac(double uu, double vv , double j[2][2])  
{
  MTriangle::jac(2, _vs, uu, vv, j);
}

void MTriangle::jac(double uu, double vv, double j[2][2])
{
  jac(1, 0, uu, vv, j);
}
