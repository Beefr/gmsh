// Gmsh - Copyright (C) 1997-2013 C. Geuzaine, J.-F. Remacle
//
// See the LICENSE.txt file for license information. Please report all
// bugs and problems to the public mailing list <gmsh@geuz.org>.


#include "GmshDefines.h"
#include "GmshMessage.h"
#include <vector>
#include "polynomialBasis.h"
#include "pyramidalBasis.h"
#include "pointsGenerators.h"
#include "BasisFactory.h"
#include "Numeric.h"

// Bezier Exponents
static fullMatrix<double> generate1DExponents(int order)
{
  fullMatrix<double> exponents(order + 1, 1);
  exponents(0, 0) = 0;
  if (order > 0) {
    exponents(1, 0) = order;
    for (int i = 2; i < order + 1; i++) exponents(i, 0) = i - 1;
  }

  return exponents;
}

static fullMatrix<double> generateExponentsTriangle(int order)
{
  int nbPoints = (order + 1) * (order + 2) / 2;
  fullMatrix<double> exponents(nbPoints, 2);

  exponents(0, 0) = 0;
  exponents(0, 1) = 0;

  if (order > 0) {
    exponents(1, 0) = order;
    exponents(1, 1) = 0;
    exponents(2, 0) = 0;
    exponents(2, 1) = order;

    if (order > 1) {
      int index = 3, ksi = 0, eta = 0;

      for (int i = 0; i < order - 1; i++, index++) {
        ksi++;
        exponents(index, 0) = ksi;
        exponents(index, 1) = eta;
      }

      ksi = order;

      for (int i = 0; i < order - 1; i++, index++) {
        ksi--;
        eta++;
        exponents(index, 0) = ksi;
        exponents(index, 1) = eta;
      }

      eta = order;
      ksi = 0;

      for (int i = 0; i < order - 1; i++, index++) {
        eta--;
        exponents(index, 0) = ksi;
        exponents(index, 1) = eta;
      }

      if (order > 2) {
        fullMatrix<double> inner = generateExponentsTriangle(order - 3);
        inner.add(1);
        exponents.copy(inner, 0, nbPoints - index, 0, 2, index, 0);
      }
    }
  }

  return exponents;
}
static fullMatrix<double> generateExponentsQuad(int order)
{
  int nbPoints = (order+1)*(order+1);
  fullMatrix<double> exponents(nbPoints, 2);

  exponents(0, 0) = 0;
  exponents(0, 1) = 0;

  if (order > 0) {
    exponents(1, 0) = order;
    exponents(1, 1) = 0;
    exponents(2, 0) = order;
    exponents(2, 1) = order;
    exponents(3, 0) = 0;
    exponents(3, 1) = order;

    if (order > 1) {
      int index = 4;
      const static int edges[4][2]={{0,1},{1,2},{2,3},{3,0}};
      for (int iedge=0; iedge<4; iedge++) {
        int p0 = edges[iedge][0];
        int p1 = edges[iedge][1];
        for (int i = 1; i < order; i++, index++) {
          exponents(index, 0) = exponents(p0, 0) + i*(exponents(p1,0)-exponents(p0,0))/order;
          exponents(index, 1) = exponents(p0, 1) + i*(exponents(p1,1)-exponents(p0,1))/order;
        }
      }
      if (order > 2) {
        fullMatrix<double> inner = generateExponentsQuad(order - 2);
        inner.add(1);
        exponents.copy(inner, 0, nbPoints - index, 0, 2, index, 0);
      }
    }
  }

  //  exponents.print("expq");

  return exponents;
}
static int nbdoftriangle(int order) { return (order + 1) * (order + 2) / 2; }

static void nodepositionface0(int order, double *u, double *v, double *w)
{ // uv surface - orientation v0-v2-v1
  int ndofT = nbdoftriangle(order);
  if (order == 0) { u[0] = 0.; v[0] = 0.; w[0] = 0.; return; }

  u[0]= 0.;    v[0]= 0.;    w[0] = 0.;
  u[1]= 0.;    v[1]= order; w[1] = 0.;
  u[2]= order; v[2]= 0.;    w[2] = 0.;

  // edges
  for (int k = 0; k < (order - 1); k++){
    u[3 + k] = 0.;
    v[3 + k] = k + 1;
    w[3 + k] = 0.;

    u[3 + order - 1 + k] = k + 1;
    v[3 + order - 1 + k] = order - 1 - k ;
    w[3 + order - 1 + k] = 0.;

    u[3 + 2 * (order - 1) + k] = order - 1 - k;
    v[3 + 2 * (order - 1) + k] = 0.;
    w[3 + 2 * (order - 1) + k] = 0.;
  }

  if (order > 2){
    int nbdoftemp = nbdoftriangle(order - 3);
    nodepositionface0(order - 3, &u[3 + 3 * (order - 1)], &v[3 + 3 * (order - 1)],
                      &w[3 + 3* (order - 1)]);
    for (int k = 0; k < nbdoftemp; k++){
      u[3 + k + 3 * (order - 1)] = u[3 + k + 3 * (order - 1)] * (order - 3) + 1.;
      v[3 + k + 3 * (order - 1)] = v[3 + k + 3 * (order - 1)] * (order - 3) + 1.;
      w[3 + k + 3 * (order - 1)] = w[3 + k + 3 * (order - 1)] * (order - 3);
    }
  }
  for (int k = 0; k < ndofT; k++){
    u[k] = u[k] / order;
    v[k] = v[k] / order;
    w[k] = w[k] / order;
  }
}

static void nodepositionface1(int order, double *u, double *v, double *w)
{ // uw surface - orientation v0-v1-v3
  int ndofT = nbdoftriangle(order);
  if (order == 0) { u[0] = 0.; v[0] = 0.; w[0] = 0.; return; }

  u[0] = 0.;    v[0]= 0.;  w[0] = 0.;
  u[1] = order; v[1]= 0.;  w[1] = 0.;
  u[2] = 0.;    v[2]= 0.;  w[2] = order;
  // edges
  for (int k = 0; k < (order - 1); k++){
    u[3 + k] = k + 1;
    v[3 + k] = 0.;
    w[3 + k] = 0.;

    u[3 + order - 1 + k] = order - 1 - k;
    v[3 + order - 1 + k] = 0.;
    w[3 + order - 1+ k ] = k + 1;

    u[3 + 2 * (order - 1) + k] = 0. ;
    v[3 + 2 * (order - 1) + k] = 0.;
    w[3 + 2 * (order - 1) + k] = order - 1 - k;
  }
  if (order > 2){
    int nbdoftemp = nbdoftriangle(order - 3);
    nodepositionface1(order - 3, &u[3 + 3 * (order - 1)], &v[3 + 3 * (order -1 )],
      &w[3 + 3 * (order - 1)]);
    for (int k = 0; k < nbdoftemp; k++){
      u[3 + k + 3 * (order - 1)] = u[3 + k + 3 * (order - 1)] * (order - 3) + 1.;
      v[3 + k + 3 * (order - 1)] = v[3 + k + 3 * (order - 1)] * (order - 3);
      w[3 + k + 3 * (order - 1)] = w[3 + k + 3 * (order - 1)] * (order - 3) + 1.;
    }
  }
  for (int k = 0; k < ndofT; k++){
    u[k] = u[k] / order;
    v[k] = v[k] / order;
    w[k] = w[k] / order;
  }
}

static void nodepositionface2(int order, double *u, double *v, double *w)
{ // vw surface - orientation v0-v3-v2
   int ndofT = nbdoftriangle(order);
   if (order == 0) { u[0] = 0.; v[0] = 0.; return; }

   u[0]= 0.; v[0]= 0.;    w[0] = 0.;
   u[1]= 0.; v[1]= 0.;    w[1] = order;
   u[2]= 0.; v[2]= order; w[2] = 0.;
   // edges
   for (int k = 0; k < (order - 1); k++){

     u[3 + k] = 0.;
     v[3 + k] = 0.;
     w[3 + k] = k + 1;

     u[3 + order - 1 + k] = 0.;
     v[3 + order - 1 + k] = k + 1;
     w[3 + order - 1 + k] = order - 1 - k;

     u[3 + 2 * (order - 1) + k] = 0.;
     v[3 + 2 * (order - 1) + k] = order - 1 - k;
     w[3 + 2 * (order - 1) + k] = 0.;
   }
   if (order > 2){
     int nbdoftemp = nbdoftriangle(order - 3);
     nodepositionface2(order - 3, &u[3 + 3 * (order - 1)], &v[3 + 3 * (order - 1)],
                       &w[3 + 3 * (order - 1)]);
     for (int k = 0; k < nbdoftemp; k++){
       u[3 + k + 3 * (order - 1)] = u[3 + k + 3 * (order - 1)] * (order - 3);
       v[3 + k + 3 * (order - 1)] = v[3 + k + 3 * (order - 1)] * (order - 3) + 1.;
       w[3 + k + 3 * (order - 1)] = w[3 + k + 3 * (order - 1)] * (order - 3) + 1.;
     }
   }
   for (int k = 0; k < ndofT; k++){
     u[k] = u[k] / order;
     v[k] = v[k] / order;
     w[k] = w[k] / order;
   }
}

static void nodepositionface3(int order,  double *u,  double *v,  double *w)
{ // uvw surface  - orientation v3-v1-v2
   int ndofT = nbdoftriangle(order);
   if (order == 0) { u[0] = 0.; v[0] = 0.; w[0] = 0.; return; }

   u[0]= 0.;    v[0]= 0.;    w[0] = order;
   u[1]= order; v[1]= 0.;    w[1] = 0.;
   u[2]= 0.;    v[2]= order; w[2] = 0.;
   // edges
   for (int k = 0; k < (order - 1); k++){

     u[3 + k] = k + 1;
     v[3 + k] = 0.;
     w[3 + k] = order - 1 - k;

     u[3 + order - 1 + k] = order - 1 - k;
     v[3 + order - 1 + k] = k + 1;
     w[3 + order - 1 + k] = 0.;

     u[3 + 2 * (order - 1) + k] = 0.;
     v[3 + 2 * (order - 1) + k] = order - 1 - k;
     w[3 + 2 * (order - 1) + k] = k + 1;
   }
   if (order > 2){
     int nbdoftemp = nbdoftriangle(order - 3);
     nodepositionface3(order - 3, &u[3 + 3 * (order - 1)], &v[3 + 3 * (order - 1)],
                       &w[3 + 3 * (order - 1)]);
     for (int k = 0; k < nbdoftemp; k++){
       u[3 + k + 3 * (order - 1)] = u[3 + k + 3 * (order - 1)] * (order - 3) + 1.;
       v[3 + k + 3 * (order - 1)] = v[3 + k + 3 * (order - 1)] * (order - 3) + 1.;
       w[3 + k + 3 * (order - 1)] = w[3 + k + 3 * (order - 1)] * (order - 3) + 1.;
     }
   }
   for (int k = 0; k < ndofT; k++){
     u[k] = u[k] / order;
     v[k] = v[k] / order;
     w[k] = w[k] / order;
   }
}

static fullMatrix<double> generateExponentsTetrahedron(int order)
{
  int nbPoints = (order + 1) * (order + 2) * (order + 3) / 6;

  fullMatrix<double> exponents(nbPoints, 3);

  exponents(0, 0) = 0;
  exponents(0, 1) = 0;
  exponents(0, 2) = 0;

  if (order > 0) {
    exponents(1, 0) = order;
    exponents(1, 1) = 0;
    exponents(1, 2) = 0;

    exponents(2, 0) = 0;
    exponents(2, 1) = order;
    exponents(2, 2) = 0;

    exponents(3, 0) = 0;
    exponents(3, 1) = 0;
    exponents(3, 2) = order;

    // edges e5 and e6 switched in original version, opposite direction
    // the template has been defined in table edges_tetra and faces_tetra (MElement.h)

    if (order > 1) {
      for (int k = 0; k < (order - 1); k++) {
        exponents(4 + k, 0) = k + 1;
        exponents(4 +      order - 1  + k, 0) = order - 1 - k;
        exponents(4 + 2 * (order - 1) + k, 0) = 0;
        exponents(4 + 3 * (order - 1) + k, 0) = 0;
        // exponents(4 + 4 * (order - 1) + k, 0) = order - 1 - k;
        // exponents(4 + 5 * (order - 1) + k, 0) = 0.;
        exponents(4 + 4 * (order - 1) + k, 0) = 0;
        exponents(4 + 5 * (order - 1) + k, 0) = k+1;

        exponents(4 + k, 1) = 0;
        exponents(4 +      order - 1  + k, 1) = k + 1;
        exponents(4 + 2 * (order - 1) + k, 1) = order - 1 - k;
        exponents(4 + 3 * (order - 1) + k, 1) = 0;
        //         exponents(4 + 4 * (order - 1) + k, 1) = 0.;
        //         exponents(4 + 5 * (order - 1) + k, 1) = order - 1 - k;
        exponents(4 + 4 * (order - 1) + k, 1) = k+1;
        exponents(4 + 5 * (order - 1) + k, 1) = 0;

        exponents(4 + k, 2) = 0;
        exponents(4 +      order - 1  + k, 2) = 0;
        exponents(4 + 2 * (order - 1) + k, 2) = 0;
        exponents(4 + 3 * (order - 1) + k, 2) = order - 1 - k;
        exponents(4 + 4 * (order - 1) + k, 2) = order - 1 - k;
        exponents(4 + 5 * (order - 1) + k, 2) = order - 1 - k;
      }

      if (order > 2) {
        int ns = 4 + 6 * (order - 1);
        int nbdofface = nbdoftriangle(order - 3);

        double *u = new double[nbdofface];
        double *v = new double[nbdofface];
        double *w = new double[nbdofface];

        nodepositionface0(order - 3, u, v, w);

        // u-v plane

        for (int i = 0; i < nbdofface; i++){
          exponents(ns + i, 0) = u[i] * (order - 3) + 1;
          exponents(ns + i, 1) = v[i] * (order - 3) + 1;
          exponents(ns + i, 2) = w[i] * (order - 3);
        }

        ns = ns + nbdofface;

        // u-w plane

        nodepositionface1(order - 3, u, v, w);

        for (int i=0; i < nbdofface; i++){
          exponents(ns + i, 0) = u[i] * (order - 3) + 1;
          exponents(ns + i, 1) = v[i] * (order - 3) ;
          exponents(ns + i, 2) = w[i] * (order - 3) + 1;
        }

        // v-w plane

        ns = ns + nbdofface;

        nodepositionface2(order - 3, u, v, w);

        for (int i = 0; i < nbdofface; i++){
          exponents(ns + i, 0) = u[i] * (order - 3);
          exponents(ns + i, 1) = v[i] * (order - 3) + 1;
          exponents(ns + i, 2) = w[i] * (order - 3) + 1;
        }

        // u-v-w plane

        ns = ns + nbdofface;

        nodepositionface3(order - 3, u, v, w);

        for (int i = 0; i < nbdofface; i++){
          exponents(ns + i, 0) = u[i] * (order - 3) + 1;
          exponents(ns + i, 1) = v[i] * (order - 3) + 1;
          exponents(ns + i, 2) = w[i] * (order - 3) + 1;
        }

        ns = ns + nbdofface;

        delete [] u;
        delete [] v;
        delete [] w;

        if (order > 3) {

          fullMatrix<double> interior = generateExponentsTetrahedron(order - 4);
          for (int k = 0; k < interior.size1(); k++) {
            exponents(ns + k, 0) = 1 + interior(k, 0);
            exponents(ns + k, 1) = 1 + interior(k, 1);
            exponents(ns + k, 2) = 1 + interior(k, 2);
          }
        }
      }
    }
  }

  return exponents;
}

static fullMatrix<double> generateExponentsPrism(int order)
{
  int nbPoints = (order + 1)*(order + 1)*(order + 2)/2;
  fullMatrix<double> exponents(nbPoints, 3);

  int index = 0;
  fullMatrix<double> triExp = generateExponentsTriangle(order);
  fullMatrix<double> lineExp = generate1DExponents(order);
  // First exponents (i < 3) must relate to corner
  for (int i = 0; i < triExp.size1(); i++) {
    exponents(index,0) = triExp(i,0);
    exponents(index,1) = triExp(i,1);
    exponents(index,2) = lineExp(0,0);
    index ++;
    exponents(index,0) = triExp(i,0);
    exponents(index,1) = triExp(i,1);
    exponents(index,2) = lineExp(1,0);
    index ++;
  }
  for (int j = 2; j <lineExp.size1() ; j++) {
    for (int i = 0; i < triExp.size1(); i++) {
      exponents(index,0) = triExp(i,0);
      exponents(index,1) = triExp(i,1);
      exponents(index,2) = lineExp(j,0);
      index ++;
    }
  }

  return exponents;
}

static fullMatrix<double> generateExponentsHex(int order)
{
  int nbPoints = (order+1) * (order+1) * (order+1);
  fullMatrix<double> exponents(nbPoints, 3);

  if (order == 0) {
    exponents(0, 0) = .0;
    return exponents;
  }

  int index = 0;
  fullMatrix<double> lineExp = generate1DExponents(order);

  for (int i = 0; i < 2; ++i) {
    for (int j = 0; j < 2; ++j) {
      for (int k = 0; k < 2; ++k) {
        exponents(index, 0) = lineExp(i, 0);
        exponents(index, 1) = lineExp(j, 0);
        exponents(index, 2) = lineExp(k, 0);
        ++index;
      }
    }
  }

  for (int i = 2; i < lineExp.size1(); ++i) {
    for (int j = 0; j < 2; ++j) {
      for (int k = 0; k < 2; ++k) {
        exponents(index, 0) = lineExp(i, 0);
        exponents(index, 1) = lineExp(j, 0);
        exponents(index, 2) = lineExp(k, 0);
        ++index;
      }
    }
  }
  for (int i = 0; i < lineExp.size1(); ++i) {
    for (int j = 2; j < lineExp.size1(); ++j) {
      for (int k = 0; k < 2; ++k) {
        exponents(index, 0) = lineExp(i, 0);
        exponents(index, 1) = lineExp(j, 0);
        exponents(index, 2) = lineExp(k, 0);
        ++index;
      }
    }
  }
  for (int i = 0; i < lineExp.size1(); ++i) {
    for (int j = 0; j < lineExp.size1(); ++j) {
      for (int k = 2; k < lineExp.size1(); ++k) {
        exponents(index, 0) = lineExp(i, 0);
        exponents(index, 1) = lineExp(j, 0);
        exponents(index, 2) = lineExp(k, 0);
        ++index;
      }
    }
  }

  return exponents;
}

// Sampling Points
static fullMatrix<double> generate1DPoints(int order)
{
  fullMatrix<double> line(order + 1, 1);
  line(0,0) = .0;
  if (order > 0) {
    line(1, 0) = 1.;
    double dd = 1. / order;
    for (int i = 2; i < order + 1; i++) line(i, 0) = dd * (i - 1);
  }

  return line;
}

static fullMatrix<double> generatePointsQuadRecur(int order, bool serendip)
{
  int nbPoints = serendip ? order*4 : (order+1)*(order+1);
  fullMatrix<double> point(nbPoints, 2);

  if (order > 0) {
    point(0, 0) = -1;
    point(0, 1) = -1;
    point(1, 0) = 1;
    point(1, 1) = -1;
    point(2, 0) = 1;
    point(2, 1) = 1;
    point(3, 0) = -1;
    point(3, 1) = 1;

    if (order > 1) {
      int index = 4;
      const static int edges[4][2]={{0,1},{1,2},{2,3},{3,0}};
      for (int iedge=0; iedge<4; iedge++) {
        int p0 = edges[iedge][0];
        int p1 = edges[iedge][1];
        for (int i = 1; i < order; i++, index++) {
          point(index, 0) = point(p0, 0) + i*(point(p1,0)-point(p0,0))/order;
          point(index, 1) = point(p0, 1) + i*(point(p1,1)-point(p0,1))/order;
        }
      }
      if (order >= 2 && !serendip) {
        fullMatrix<double> inner = generatePointsQuadRecur(order - 2, false);
        inner.scale(1. - 2./order);
        point.copy(inner, 0, nbPoints - index, 0, 2, index, 0);
      }
    }
  }
  else {
    point(0, 0) = 0;
    point(0, 1) = 0;
  }


  return point;
}

static fullMatrix<double> generatePointsQuad(int order, bool serendip)
{
  fullMatrix<double>  point = generatePointsQuadRecur(order, serendip);
  // rescale to [0,1] x [0,1]
  for (int i=0;i<point.size1();i++){
    point(i,0) = (1.+point(i,0))*.5;
    point(i,1) = (1.+point(i,1))*.5;
  }
  return point;
}

static fullMatrix<double> generatePointsPrism(int order, bool serendip)
{
  const double prism18Pts[18][3] = {
    {0, 0, 0}, // 0
    {1, 0, 0}, // 1
    {0, 1, 0}, // 2
    {0, 0, 1},  // 3
    {1, 0, 1},  // 4
    {0, 1, 1},  // 5
    {.5, 0, 0},  // 6
    {0, .5, 0},  // 7
    {0, 0, .5},  // 8
    {.5, .5, 0},  // 9
    {1, 0, .5},  // 10
    {0, 1, .5},  // 11
    {.5, 0, 1},  // 12
    {0, .5, 1},  // 13
    {.5, .5, 1},  // 14
    {.5, 0, .5},  // 15
    {0, .5, .5},  // 16
    {.5, .5, .5},  // 17
  };

  int nbPoints = (order + 1)*(order + 1)*(order + 2)/2;
  fullMatrix<double> point(nbPoints, 3);

  int index = 0;

  if (order == 2)
    for (int i =0; i<18; i++)
      for (int j=0; j<3;j++)
        point(i,j) = prism18Pts[i][j];
  else {
    fullMatrix<double> triPoints = gmshGeneratePointsTriangle(order,false);
    fullMatrix<double> linePoints = generate1DPoints(order);
    for (int j = 0; j <linePoints.size1() ; j++) {
      for (int i = 0; i < triPoints.size1(); i++) {
        point(index,0) = triPoints(i,0);
        point(index,1) = triPoints(i,1);
        point(index,2) = linePoints(j,0);
        index ++;
      }
    }
  }
  return point;
}

static fullMatrix<double> generatePointsHex(int order, bool serendip)
{
  int nbPoints = (order+1) * (order+1) * (order+1);
  fullMatrix<double> point(nbPoints, 3);

  fullMatrix<double> linePoints = generate1DPoints(order);
  int index = 0;

  for (int i = 0; i < linePoints.size1(); ++i) {
    for (int j = 0; j < linePoints.size1(); ++j) {
      for (int k = 0; k < linePoints.size1(); ++k) {
        point(index, 0) = linePoints(i, 0);
        point(index, 1) = linePoints(j, 0);
        point(index, 2) = linePoints(k, 0);
        ++index;
      }
    }
  }

  return point;
}

// Sub Control Points
static std::vector< fullMatrix<double> > generateSubPointsLine(int order)
{
  std::vector< fullMatrix<double> > subPoints(2);

  subPoints[0] = generate1DPoints(order);
  subPoints[0].scale(.5);

  subPoints[1].copy(subPoints[0]);
  subPoints[1].add(.5);

  return subPoints;
}

static std::vector< fullMatrix<double> > generateSubPointsTriangle(int order)
{
  std::vector< fullMatrix<double> > subPoints(4);
  fullMatrix<double> prox;

  subPoints[0] = gmshGeneratePointsTriangle(order, false);
  subPoints[0].scale(.5);

  subPoints[1].copy(subPoints[0]);
  prox.setAsProxy(subPoints[1], 0, 1);
  prox.add(.5);

  subPoints[2].copy(subPoints[0]);
  prox.setAsProxy(subPoints[2], 1, 1);
  prox.add(.5);

  subPoints[3].copy(subPoints[0]);
  subPoints[3].scale(-1.);
  subPoints[3].add(.5);

  return subPoints;
}

static std::vector< fullMatrix<double> > generateSubPointsQuad(int order)
{
  std::vector< fullMatrix<double> > subPoints(4);
  fullMatrix<double> prox;

  subPoints[0] = generatePointsQuad(order, false);
  subPoints[0].scale(.5);

  subPoints[1].copy(subPoints[0]);
  prox.setAsProxy(subPoints[1], 0, 1);
  prox.add(.5);

  subPoints[2].copy(subPoints[0]);
  prox.setAsProxy(subPoints[2], 1, 1);
  prox.add(.5);

  subPoints[3].copy(subPoints[1]);
  prox.setAsProxy(subPoints[3], 1, 1);
  prox.add(.5);

  return subPoints;
}

static std::vector< fullMatrix<double> > generateSubPointsTetrahedron(int order)
{
  std::vector< fullMatrix<double> > subPoints(8);
  fullMatrix<double> prox1;
  fullMatrix<double> prox2;

  subPoints[0] = gmshGeneratePointsTetrahedron(order, false);
  subPoints[0].scale(.5);

  subPoints[1].copy(subPoints[0]);
  prox1.setAsProxy(subPoints[1], 0, 1);
  prox1.add(.5);

  subPoints[2].copy(subPoints[0]);
  prox1.setAsProxy(subPoints[2], 1, 1);
  prox1.add(.5);

  subPoints[3].copy(subPoints[0]);
  prox1.setAsProxy(subPoints[3], 2, 1);
  prox1.add(.5);

  // u := .5-u-w
  // v := .5-v-w
  // w := w
  subPoints[4].copy(subPoints[0]);
  prox1.setAsProxy(subPoints[4], 0, 2);
  prox1.scale(-1.);
  prox1.add(.5);
  prox1.setAsProxy(subPoints[4], 0, 1);
  prox2.setAsProxy(subPoints[4], 2, 1);
  prox1.add(prox2, -1.);
  prox1.setAsProxy(subPoints[4], 1, 1);
  prox1.add(prox2, -1.);

  // u := u
  // v := .5-v
  // w := w+v
  subPoints[5].copy(subPoints[0]);
  prox1.setAsProxy(subPoints[5], 2, 1);
  prox2.setAsProxy(subPoints[5], 1, 1);
  prox1.add(prox2);
  prox2.scale(-1.);
  prox2.add(.5);

  // u := .5-u
  // v := v
  // w := w+u
  subPoints[6].copy(subPoints[0]);
  prox1.setAsProxy(subPoints[6], 2, 1);
  prox2.setAsProxy(subPoints[6], 0, 1);
  prox1.add(prox2);
  prox2.scale(-1.);
  prox2.add(.5);

  // u := u+w
  // v := v+w
  // w := .5-w
  subPoints[7].copy(subPoints[0]);
  prox1.setAsProxy(subPoints[7], 0, 1);
  prox2.setAsProxy(subPoints[7], 2, 1);
  prox1.add(prox2);
  prox1.setAsProxy(subPoints[7], 1, 1);
  prox1.add(prox2);
  prox2.scale(-1.);
  prox2.add(.5);


  return subPoints;
}

static std::vector< fullMatrix<double> > generateSubPointsPrism(int order)
{
  std::vector< fullMatrix<double> > subPoints(8);
  fullMatrix<double> prox;

  subPoints[0] = generatePointsPrism(order, false);
  subPoints[0].scale(.5);

  subPoints[1].copy(subPoints[0]);
  prox.setAsProxy(subPoints[1], 0, 1);
  prox.add(.5);

  subPoints[2].copy(subPoints[0]);
  prox.setAsProxy(subPoints[2], 1, 1);
  prox.add(.5);

  subPoints[3].copy(subPoints[0]);
  prox.setAsProxy(subPoints[3], 0, 2);
  prox.scale(-1.);
  prox.add(.5);

  subPoints[4].copy(subPoints[0]);
  prox.setAsProxy(subPoints[4], 2, 1);
  prox.add(.5);

  subPoints[5].copy(subPoints[1]);
  prox.setAsProxy(subPoints[5], 2, 1);
  prox.add(.5);

  subPoints[6].copy(subPoints[2]);
  prox.setAsProxy(subPoints[6], 2, 1);
  prox.add(.5);

  subPoints[7].copy(subPoints[3]);
  prox.setAsProxy(subPoints[7], 2, 1);
  prox.add(.5);

  return subPoints;
}

static std::vector< fullMatrix<double> > generateSubPointsHex(int order)
{
  std::vector< fullMatrix<double> > subPoints(8);
  fullMatrix<double> prox;

  subPoints[0] = generatePointsHex(order, false);
  subPoints[0].scale(.5);

  subPoints[1].copy(subPoints[0]);
  prox.setAsProxy(subPoints[1], 0, 1);
  prox.add(.5);

  subPoints[2].copy(subPoints[0]);
  prox.setAsProxy(subPoints[2], 1, 1);
  prox.add(.5);

  subPoints[3].copy(subPoints[1]);
  prox.setAsProxy(subPoints[3], 1, 1);
  prox.add(.5);

  subPoints[4].copy(subPoints[0]);
  prox.setAsProxy(subPoints[4], 2, 1);
  prox.add(.5);

  subPoints[5].copy(subPoints[1]);
  prox.setAsProxy(subPoints[5], 2, 1);
  prox.add(.5);

  subPoints[6].copy(subPoints[2]);
  prox.setAsProxy(subPoints[6], 2, 1);
  prox.add(.5);

  subPoints[7].copy(subPoints[3]);
  prox.setAsProxy(subPoints[7], 2, 1);
  prox.add(.5);

  return subPoints;
}

// Matrices generation
static int nChoosek(int n, int k)
{
  if (n < k || k < 0) {
    Msg::Error("Wrong argument for combination.");
    return 1;
  }

  if (k > n/2) k = n-k;
  if (k == 1)
    return n;
  if (k == 0)
    return 1;

  int c = 1;
  for (int i = 1; i <= k; i++, n--) (c *= n) /= i;
  return c;
}


static fullMatrix<double> generateBez2LagMatrix
  (const fullMatrix<double> &exponent, const fullMatrix<double> &point,
   int order, int dimSimplex)
{

  if(exponent.size1() != point.size1() || exponent.size2() != point.size2()){
    Msg::Fatal("Wrong sizes for Bezier coefficients generation %d %d -- %d %d",
      exponent.size1(),point.size1(),
      exponent.size2(),point.size2());
    return fullMatrix<double>(1, 1);
  }

  int ndofs = exponent.size1();
  int dim = exponent.size2();

  fullMatrix<double> bez2Lag(ndofs, ndofs);
  for (int i = 0; i < ndofs; i++) {
    for (int j = 0; j < ndofs; j++) {
      double dd = 1.;

      {
        double pointCompl = 1.;
        int exponentCompl = order;
        for (int k = 0; k < dimSimplex; k++) {
          dd *= nChoosek(exponentCompl, (int) exponent(i, k))
            * pow(point(j, k), exponent(i, k));
          pointCompl -= point(j, k);
          exponentCompl -= (int) exponent(i, k);
        }
        dd *= pow(pointCompl, exponentCompl);
      }

      for (int k = dimSimplex; k < dim; k++)
        dd *= nChoosek(order, (int) exponent(i, k))
            * pow(point(j, k), exponent(i, k))
            * pow(1. - point(j, k), order - exponent(i, k));

      bez2Lag(j, i) = dd;
    }
  }
  return bez2Lag;
}



static fullMatrix<double> generateSubDivisor
  (const fullMatrix<double> &exponents, const std::vector< fullMatrix<double> > &subPoints,
   const fullMatrix<double> &lag2Bez, int order, int dimSimplex)
{
  if (exponents.size1() != lag2Bez.size1() || exponents.size1() != lag2Bez.size2()){
    Msg::Fatal("Wrong sizes for Bezier Divisor %d %d -- %d %d",
      exponents.size1(), lag2Bez.size1(),
      exponents.size1(), lag2Bez.size2());
    return fullMatrix<double>(1, 1);
  }

  int nbPts = lag2Bez.size1();
  int nbSubPts = nbPts * subPoints.size();

  fullMatrix<double> intermediate2(nbPts, nbPts);
  fullMatrix<double> subDivisor(nbSubPts, nbPts);

  for (unsigned int i = 0; i < subPoints.size(); i++) {
    fullMatrix<double> intermediate1 =
      generateBez2LagMatrix(exponents, subPoints[i], order, dimSimplex);
    lag2Bez.mult(intermediate1, intermediate2);
    subDivisor.copy(intermediate2, 0, nbPts, 0, nbPts, i*nbPts, 0);
  }
  return subDivisor;
}



// Convert Bezier points to Lagrange points
static fullMatrix<double> bez2LagPoints(bool dim1, bool dim2, bool dim3, const fullMatrix<double> &bezierPoints)
{

  // FIXME BUG for 2nd order quads we seem to try to access bezierPoints(i, 2);
  // but bezierPoints only has 2 columns

  const int numPt = bezierPoints.size1();
  fullMatrix<double> lagPoints(numPt,3);

  for (int i=0; i<numPt; i++) {
    lagPoints(i,0) = dim1 ? -1. + 2*bezierPoints(i,0) : bezierPoints(i,0);
    lagPoints(i,1) = dim2 ? -1. + 2*bezierPoints(i,1) : bezierPoints(i,1);
    lagPoints(i,2) = dim3 ? -1. + 2*bezierPoints(i,2) : bezierPoints(i,2);
  }

  return lagPoints;

}


bezierBasis::bezierBasis(int tag)
{
  order = MElement::OrderFromTag(tag);

  if (MElement::ParentTypeFromTag(tag) == TYPE_PYR) {
    dim = 3;
    numLagPts = 5;
    bezierPoints = gmshGeneratePointsPyramid(order,false);
    lagPoints = bezierPoints;
    matrixLag2Bez.resize(bezierPoints.size1(), bezierPoints.size1(), false);
    matrixBez2Lag.resize(bezierPoints.size1(), bezierPoints.size1(), false);
    for (int i=0; i<matrixBez2Lag.size1(); ++i) {
      matrixLag2Bez(i,i) = 1.;
      matrixBez2Lag(i,i) = 1.;
    }
    // TODO: Implement subdidivsor
  }
  else {
    int dimSimplex;
    std::vector< fullMatrix<double> > subPoints;
    switch (MElement::ParentTypeFromTag(tag)) {
      case TYPE_PNT :
        dim = 0;
        numLagPts = 1;
        exponents = generate1DExponents(0);
        bezierPoints = generate1DPoints(0);
        lagPoints = bezierPoints;
        dimSimplex = 0;
        break;
      case TYPE_LIN : {
        dim = 1;
        numLagPts = 2;
        exponents = generate1DExponents(order);
        bezierPoints = generate1DPoints(order);
        lagPoints = bez2LagPoints(true,false,false,bezierPoints);
        dimSimplex = 0;
        subPoints = generateSubPointsLine(0);
        break;
      }
      case TYPE_TRI : {
        dim = 2;
        numLagPts = 3;
        exponents = generateExponentsTriangle(order);
        bezierPoints = gmshGeneratePointsTriangle(order,false);
        lagPoints = bezierPoints;
        dimSimplex = 2;
        subPoints = generateSubPointsTriangle(order);
        break;
      }
      case TYPE_QUA : {
        dim = 2;
        numLagPts = 4;
        exponents = generateExponentsQuad(order);
        bezierPoints = generatePointsQuad(order,false);
        lagPoints = bez2LagPoints(true,true,false,bezierPoints);
        dimSimplex = 0;
        subPoints = generateSubPointsQuad(order);
        //      points.print("points");
        break;
      }
      case TYPE_TET : {
        dim = 3;
        numLagPts = 4;
        exponents = generateExponentsTetrahedron(order);
        bezierPoints = gmshGeneratePointsTetrahedron(order,false);
        lagPoints = bezierPoints;
        dimSimplex = 3;
        subPoints = generateSubPointsTetrahedron(order);
        break;
      }
      case TYPE_PRI : {
        dim = 3;
        numLagPts = 6;
        exponents = generateExponentsPrism(order);
        bezierPoints = generatePointsPrism(order, false);
        lagPoints = bez2LagPoints(false,false,true,bezierPoints);
        dimSimplex = 2;
        subPoints = generateSubPointsPrism(order);
        break;
      }
      case TYPE_HEX : {
        dim = 3;
        numLagPts = 8;
        exponents = generateExponentsHex(order);
        bezierPoints = generatePointsHex(order, false);
        lagPoints = bez2LagPoints(true,true,true,bezierPoints);
        dimSimplex = 0;
        subPoints = generateSubPointsHex(order);
        break;
      }
      default : {
        Msg::Error("Unknown function space %d: reverting to TET_1", tag);
        dim = 3;
        numLagPts = 4;
        exponents = generateExponentsTetrahedron(0);
        bezierPoints = gmshGeneratePointsTetrahedron(0, false);
        lagPoints = bezierPoints;
        dimSimplex = 3;
        subPoints = generateSubPointsTetrahedron(0);
        break;
      }
    }
    Msg::Info("%d %d %d %d %d %d", exponents.size1(), exponents.size2(), bezierPoints.size1(), bezierPoints.size2(), order, dimSimplex);
    matrixBez2Lag = generateBez2LagMatrix(exponents, bezierPoints, order, dimSimplex);
    Msg::Info("%d %d %d %d", matrixBez2Lag.size1(), matrixBez2Lag.size2(), matrixLag2Bez.size1(), matrixLag2Bez.size2());
    Msg::Info("%f %f %f %f", bezierPoints(0, 0), bezierPoints(0, 1), exponents(0, 0), exponents(0, 1));
    matrixBez2Lag.invert(matrixLag2Bez);
    subDivisor = generateSubDivisor(exponents, subPoints, matrixLag2Bez, order, dimSimplex);
    numDivisions = (int) pow(2., (int) bezierPoints.size2());
  }
}
