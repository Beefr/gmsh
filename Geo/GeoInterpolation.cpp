// $Id: GeoInterpolation.cpp,v 1.1 2006-11-25 16:52:43 geuzaine Exp $
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
#include "Geo.h"
#include "GeoInterpolation.h"
#include "GeoStringInterface.h"
#include "GeoUtils.h"
#include "Utils.h"
#include "Numeric.h"

extern Mesh *THEM;

// X = X(u), Y = Y(u), Z = Z(u)

// Curves
Vertex InterpolateCurve(Curve * Curve, double u, int derivee)
{
  int N, i, j;
  Vertex D[2], V;
  Vertex *v[5];
  double eps = 1.e-3, T[4], W, teta, t1, t2, t;
  double vec[4];
  Vertex temp1, temp2;

  V.u = u;

  if(derivee) {
    D[0] = InterpolateCurve(Curve, u, 0);
    D[1] = InterpolateCurve(Curve, u + eps, 0);
    V.Pos.X = (D[1].Pos.X - D[0].Pos.X) / eps;
    V.Pos.Y = (D[1].Pos.Y - D[0].Pos.Y) / eps;
    V.Pos.Z = (D[1].Pos.Z - D[0].Pos.Z) / eps;
    return V;
  }

  switch (Curve->Typ) {

  case MSH_SEGM_LINE:
    N = List_Nbr(Curve->Control_Points);
    i = (int)((double)(N - 1) * u);
    while(i >= N - 1)
      i--;
    while(i < 0)
      i++;
    t1 = (double)(i) / (double)(N - 1);
    t2 = (double)(i + 1) / (double)(N - 1);
    t = (u - t1) / (t2 - t1);
    List_Read(Curve->Control_Points, i, &v[1]);
    List_Read(Curve->Control_Points, i + 1, &v[2]);
    V.Pos.X = v[1]->Pos.X + t * (v[2]->Pos.X - v[1]->Pos.X);
    V.Pos.Y = v[1]->Pos.Y + t * (v[2]->Pos.Y - v[1]->Pos.Y);
    V.Pos.Z = v[1]->Pos.Z + t * (v[2]->Pos.Z - v[1]->Pos.Z);
    V.w = (1. - t) * v[1]->w + t * v[2]->w;
    V.lc = (1. - t) * v[1]->lc + t * v[2]->lc;
    return V;

  case MSH_SEGM_PARAMETRIC:
    V.Pos.X = evaluate_scalarfunction("t", u, Curve->functu);
    V.Pos.Y = evaluate_scalarfunction("t", u, Curve->functv);
    V.Pos.Z = evaluate_scalarfunction("t", u, Curve->functw);
    V.w = (1. - u) * Curve->beg->w + u * Curve->end->w;
    V.lc = (1. - u) * Curve->beg->lc + u * Curve->end->lc;
    return V;

  case MSH_SEGM_CIRC:
  case MSH_SEGM_CIRC_INV:
  case MSH_SEGM_ELLI:
  case MSH_SEGM_ELLI_INV:
    if(Curve->Typ == MSH_SEGM_CIRC_INV || Curve->Typ == MSH_SEGM_ELLI_INV) {
      V.u = 1. - u;
      u = V.u;
    }

    teta = Curve->Circle.t1 - (Curve->Circle.t1 - Curve->Circle.t2) * u;
    /* pour les ellipses */
    teta -= Curve->Circle.incl;

    V.Pos.X =
      Curve->Circle.f1 * cos(teta) * cos(Curve->Circle.incl) -
      Curve->Circle.f2 * sin(teta) * sin(Curve->Circle.incl);
    V.Pos.Y =
      Curve->Circle.f1 * cos(teta) * sin(Curve->Circle.incl) +
      Curve->Circle.f2 * sin(teta) * cos(Curve->Circle.incl);
    V.Pos.Z = 0.0;
    Projette(&V, Curve->Circle.invmat);
    V.Pos.X += Curve->Circle.v[1]->Pos.X;
    V.Pos.Y += Curve->Circle.v[1]->Pos.Y;
    V.Pos.Z += Curve->Circle.v[1]->Pos.Z;
    V.w = (1. - u) * Curve->beg->w + u * Curve->end->w;
    V.lc = (1. - u) * Curve->beg->lc + u * Curve->end->lc;
    return V;

  case MSH_SEGM_BSPLN:
  case MSH_SEGM_BEZIER:
    return InterpolateUBS(Curve, u, derivee);

  case MSH_SEGM_NURBS:
    return InterpolateNurbs(Curve, u, derivee);

  case MSH_SEGM_SPLN:
    N = List_Nbr(Curve->Control_Points);

    /* 
       0                   i    P     i+1                  N-1
       vfirst*---------*---------*----X-----*----------*----------* vlast
       0                  t1   absc   t2                    1
       0    t     1

       Splines uniformes -> Le point se trouve entre v[1] et v[2] 
       -> Calcul de l'abcisse curviligne locale t ( entre 0 et 1 )

       0           -> t1 
       1           -> t2
       u -> t

       Splines Lineiques -> Multilines
     */

    i = (int)((double)(N - 1) * u);
    if(i < 0)
      i = 0;
    if(i >= N - 1)
      i = N - 2;

    t1 = (double)(i) / (double)(N - 1);
    t2 = (double)(i + 1) / (double)(N - 1);

    t = (u - t1) / (t2 - t1);

    List_Read(Curve->Control_Points, i, &v[1]);
    List_Read(Curve->Control_Points, i + 1, &v[2]);

    V.lc = (1. - t) * v[1]->lc + t * v[2]->lc;
    V.w = (1. - t) * v[1]->w + t * v[2]->w;

    if(!i) {
      v[0] = &temp1;
      v[0]->Pos.X = 2. * v[1]->Pos.X - v[2]->Pos.X;
      v[0]->Pos.Y = 2. * v[1]->Pos.Y - v[2]->Pos.Y;
      v[0]->Pos.Z = 2. * v[1]->Pos.Z - v[2]->Pos.Z;
    }
    else {
      List_Read(Curve->Control_Points, i - 1, &v[0]);
    }

    if(i == N - 2) {
      v[3] = &temp2;
      v[3]->Pos.X = 2. * v[2]->Pos.X - v[1]->Pos.X;
      v[3]->Pos.Y = 2. * v[2]->Pos.Y - v[1]->Pos.Y;
      v[3]->Pos.Z = 2. * v[2]->Pos.Z - v[1]->Pos.Z;
    }
    else {
      List_Read(Curve->Control_Points, i + 2, &v[3]);
    }

    if(derivee) {
      T[3] = 0.;
      T[2] = 1.;
      T[1] = 2. * t;
      T[0] = 3. * t * t;
    }
    else {
      T[3] = 1.;
      T[2] = t;
      T[1] = t * t;
      T[0] = t * t * t;
    }

    V.Pos.X = V.Pos.Y = V.Pos.Z = W = 0.0;
    for(i = 0; i < 4; i++) {
      vec[i] = 0.0;
    }

    /* X */
    for(i = 0; i < 4; i++) {
      for(j = 0; j < 4; j++) {
        vec[i] += Curve->mat[i][j] * v[j]->Pos.X;
      }
    }

    for(j = 0; j < 4; j++) {
      V.Pos.X += T[j] * vec[j];
      vec[j] = 0.0;
    }

    /* Y */
    for(i = 0; i < 4; i++) {
      for(j = 0; j < 4; j++) {
        vec[i] += Curve->mat[i][j] * v[j]->Pos.Y;
      }
    }

    for(j = 0; j < 4; j++) {
      V.Pos.Y += T[j] * vec[j];
      vec[j] = 0.0;
    }

    /* Z */
    for(i = 0; i < 4; i++) {
      for(j = 0; j < 4; j++) {
        vec[i] += Curve->mat[i][j] * v[j]->Pos.Z;
      }
    }
    for(j = 0; j < 4; j++) {
      V.Pos.Z += T[j] * vec[j];
      vec[j] = 0.0;
    }

    /* W */
    for(i = 0; i < 4; i++) {
      for(j = 0; j < 4; j++) {
        vec[i] += Curve->mat[i][j] * v[j]->lc;
      }
    }
    for(j = 0; j < 4; j++) {
      W += T[j] * vec[j];
    }

    if(derivee) {
      V.Pos.X /= (t2 - t1);
      V.Pos.Y /= (t2 - t1);
      V.Pos.Z /= (t2 - t1);
    }
    else {
      // V.Pos.X /= W;
      // V.Pos.Y /= W;
      // V.Pos.Z /= W;
    }
    return V;

  default:
    Msg(GERROR, "Unknown curve type in interpolation");
    V.Pos.X = V.Pos.Y = V.Pos.Z = 0.0;
    V.w = V.lc = 1.0;
    return V;
  }

}

// Surfaces

/* Interpolation transfinie sur un quadrangle :
   f(u,v) = (1-u)c4(v) + u c2(v) + (1-v)c1(u) + v c3(u)
            - [ (1-u)(1-v)s1 + u(1-v)s2 + uv s3 + (1-u)v s4 ] */

#define TRAN_QUA(c1,c2,c3,c4,s1,s2,s3,s4,u,v) \
   (1.-u)*c4+u*c2+(1.-v)*c1+v*c3-((1.-u)*(1.-v)*s1+u*(1.-v)*s2+u*v*s3+(1.-u)*v*s4)

Vertex TransfiniteQua(Vertex c1, Vertex c2, Vertex c3, Vertex c4,
                      Vertex s1, Vertex s2, Vertex s3, Vertex s4,
                      double u, double v)
{
  Vertex V;

  V.lc = TRAN_QUA(c1.lc, c2.lc, c3.lc, c4.lc,
                  s1.lc, s2.lc, s3.lc, s4.lc, u, v);
  V.w = TRAN_QUA(c1.w, c2.w, c3.w, c4.w, s1.w, s2.w, s3.w, s4.w, u, v);
  V.Pos.X = TRAN_QUA(c1.Pos.X, c2.Pos.X, c3.Pos.X, c4.Pos.X,
                     s1.Pos.X, s2.Pos.X, s3.Pos.X, s4.Pos.X, u, v);
  V.Pos.Y = TRAN_QUA(c1.Pos.Y, c2.Pos.Y, c3.Pos.Y, c4.Pos.Y,
                     s1.Pos.Y, s2.Pos.Y, s3.Pos.Y, s4.Pos.Y, u, v);
  V.Pos.Z = TRAN_QUA(c1.Pos.Z, c2.Pos.Z, c3.Pos.Z, c4.Pos.Z,
                     s1.Pos.Z, s2.Pos.Z, s3.Pos.Z, s4.Pos.Z, u, v);
  return (V);
}

/* Interpolation transfinie sur un triangle : TRAN_QUA avec s1=s4=c4
   f(u,v) = u c2 (v) + (1-v) c1(u) + v c3(u) - u(1-v) s2 - uv s3 */

#define TRAN_TRI(c1,c2,c3,s1,s2,s3,u,v) u*c2+(1.-v)*c1+v*c3-(u*(1.-v)*s2+u*v*s3);

Vertex TransfiniteTri(Vertex c1, Vertex c2, Vertex c3,
                      Vertex s1, Vertex s2, Vertex s3, double u, double v)
{
  Vertex V;

  V.lc = TRAN_TRI(c1.lc, c2.lc, c3.lc, s1.lc, s2.lc, s3.lc, u, v);
  V.w = TRAN_TRI(c1.w, c2.w, c3.w, s1.w, s2.w, s3.w, u, v);
  V.Pos.X = TRAN_TRI(c1.Pos.X, c2.Pos.X, c3.Pos.X,
                     s1.Pos.X, s2.Pos.X, s3.Pos.X, u, v);
  V.Pos.Y = TRAN_TRI(c1.Pos.Y, c2.Pos.Y, c3.Pos.Y,
                     s1.Pos.Y, s2.Pos.Y, s3.Pos.Y, u, v);
  V.Pos.Z = TRAN_TRI(c1.Pos.Z, c2.Pos.Z, c3.Pos.Z,
                     s1.Pos.Z, s2.Pos.Z, s3.Pos.Z, u, v);
  return (V);
}

void TransfiniteSph(Vertex S, Vertex center, Vertex * T)
{
  double r, s, dirx, diry, dirz;

  r = sqrt(DSQR(S.Pos.X - center.Pos.X) + DSQR(S.Pos.Y - center.Pos.Y)
           + DSQR(S.Pos.Z - center.Pos.Z));

  s = sqrt(DSQR(T->Pos.X - center.Pos.X) + DSQR(T->Pos.Y - center.Pos.Y)
           + DSQR(T->Pos.Z - center.Pos.Z));

  dirx = (T->Pos.X - center.Pos.X) / s;
  diry = (T->Pos.Y - center.Pos.Y) / s;
  dirz = (T->Pos.Z - center.Pos.Z) / s;

  T->Pos.X = center.Pos.X + r * dirx;
  T->Pos.Y = center.Pos.Y + r * diry;
  T->Pos.Z = center.Pos.Z + r * dirz;
}

void Calcule_Z_Plan(void *data, Surface *THESURFACE)
{
  Vertex **pv, *v;
  Vertex V;

  V.Pos.X = THESURFACE->a;
  V.Pos.Y = THESURFACE->b;
  V.Pos.Z = THESURFACE->c;

  Projette(&V, THESURFACE->plan);

  pv = (Vertex **) data;
  v = *pv;
  if(V.Pos.Z != 0.0)
    v->Pos.Z = (THESURFACE->d - V.Pos.X * v->Pos.X - V.Pos.Y * v->Pos.Y)
      / V.Pos.Z;
  else
    v->Pos.Z = 0.0;
}


Vertex InterpolateSurface(Surface * s, double u, double v,
                          int derivee, int u_v)
{
  Vertex *c1, *c2, T, D[4], V[4], *S[4];
  Curve *C[4];
  int i, issphere;
  double eps = 1.e-6;

  if(derivee) {
    if(u_v == 1) {
      if(u - eps < 0.0) {
        D[0] = InterpolateSurface(s, u, v, 0, 0);
        D[1] = InterpolateSurface(s, u + eps, v, 0, 0);
      }
      else {
        D[0] = InterpolateSurface(s, u - eps, v, 0, 0);
        D[1] = InterpolateSurface(s, u, v, 0, 0);
      }
    }
    else if(u_v == 2) {
      if(v - eps < 0.0) {
        D[0] = InterpolateSurface(s, u, v, 0, 0);
        D[1] = InterpolateSurface(s, u, v + eps, 0, 0);
      }
      else {
        D[0] = InterpolateSurface(s, u, v - eps, 0, 0);
        D[1] = InterpolateSurface(s, u, v, 0, 0);
      }
    }
    else {
      Msg(WARNING, "Arbitrary InterpolateSurface for derivative not done");
      /*
         double epsc = eps * cos (t);
         double epss = eps * sin (t);
         if (v - epss < 0.0 && u - epsc < 0.0){
         D[0] = InterpolateSurface (s, u, v, 0, 0);
         D[1] = InterpolateSurface (s, u + epsc, v + epss, 0, 0);
         }
         else if (v - epss < 0.0){
         D[0] = InterpolateSurface (s, u - epsc, v, 0, 0);
         D[1] = InterpolateSurface (s, u, v + epss, 0, 0);
         }
         else if (u - epsc < 0.0){
         D[0] = InterpolateSurface (s, u, v - epss, 0, 0);
         D[1] = InterpolateSurface (s, u + epsc, v, 0, 0);
         }
         else{
         D[0] = InterpolateSurface (s, u - epsc, v - epss, 0, 0);
         D[1] = InterpolateSurface (s, u, v, 0, 0);
         }
       */
    }
    T.Pos.X = (D[1].Pos.X - D[0].Pos.X) / eps;
    T.Pos.Y = (D[1].Pos.Y - D[0].Pos.Y) / eps;
    T.Pos.Z = (D[1].Pos.Z - D[0].Pos.Z) / eps;
    return T;
  }

  Vertex x(u, v, .0);
  Vertex *xx = &x, *dum;

  if(s->Extrude && s->Extrude->geo.Mode == EXTRUDED_ENTITY &&
     s->Typ != MSH_SURF_PLAN) {
    Curve *c = FindCurve(s->Extrude->geo.Source);
    Vertex v1 = InterpolateCurve(c, u, 0);
    s->Extrude->Extrude(v, v1.Pos.X, v1.Pos.Y, v1.Pos.Z);
    return v1;
  }

  switch (s->Typ) {

  case MSH_SURF_PLAN:

    Calcule_Z_Plan(&xx, s);
    //Projette_Inverse(&xx, &dum);
    return x;

  case MSH_SURF_REGL:
    issphere = 1;
    for(i = 0; i < 4; i++) {
      List_Read(s->Generatrices, i, &C[i]);
      if(C[i]->Typ != MSH_SEGM_CIRC && C[i]->Typ != MSH_SEGM_CIRC_INV) {
        issphere = 0;
      }
      else if(issphere) {
        if(!i) {
          List_Read(C[i]->Control_Points, 1, &c1);
        }
        else {
          List_Read(C[i]->Control_Points, 1, &c2);
          if(compareVertex(&c1, &c2))
            issphere = 0;
        }
      }
    }

    S[0] = C[0]->beg;
    S[1] = C[1]->beg;
    S[2] = C[2]->beg;
    S[3] = C[3]->beg;
    V[0] = InterpolateCurve(C[0], C[0]->ubeg + (C[0]->uend - C[0]->ubeg) * u, 0);
    V[1] = InterpolateCurve(C[1], C[1]->ubeg + (C[1]->uend - C[1]->ubeg) * v, 0);
    V[2] = InterpolateCurve(C[2], C[2]->ubeg + (C[2]->uend - C[2]->ubeg) * (1. - u), 0);
    V[3] = InterpolateCurve(C[3], C[3]->ubeg + (C[3]->uend - C[3]->ubeg) * (1. - v), 0);
    T = TransfiniteQua(V[0], V[1], V[2], V[3], *S[0], *S[1], *S[2], *S[3], u, v);
    if(issphere)
      TransfiniteSph(*S[0], *c1, &T);

    return (T);

  case MSH_SURF_NURBS:
    return InterpolateNurbsSurface(s, u, v);

  case MSH_SURF_TRIC:
    issphere = 1;
    for(i = 0; i < 3; i++) {
      List_Read(s->Generatrices, i, &C[i]);
      if(C[i]->Typ != MSH_SEGM_CIRC && C[i]->Typ != MSH_SEGM_CIRC_INV) {
        issphere = 0;
      }
      else if(issphere) {
        if(!i) {
          List_Read(C[i]->Control_Points, 1, &c1);
        }
        else {
          List_Read(C[i]->Control_Points, 1, &c2);
          if(compareVertex(&c1, &c2))
            issphere = 0;
        }
      }
    }

    S[0] = C[0]->beg;
    S[1] = C[1]->beg;
    S[2] = C[2]->beg;
    V[0] = InterpolateCurve(C[0], u, 0);
    V[1] = InterpolateCurve(C[1], v, 0);
    V[2] = InterpolateCurve(C[2], 1. - u, 0);

    T = TransfiniteTri(V[0], V[1], V[2], *S[0], *S[1], *S[2], u, v);
    if(issphere)
      TransfiniteSph(*S[0], *c1, &T);

    return (T);

  default:
    Msg(GERROR, "Unknown surface type in interpolation");
    T.Pos.X = T.Pos.Y = T.Pos.Z = 0.0;
    T.w = T.lc = 1.0;
    return T;
  }

}

// Volumes

/* Interpolation transfinie sur un hexaedre 
                                   prisme (avec s1=s4=a4, s5=s8=a8, a9=a12=f4) 
   f(u,v) = (1-u) f4(v,w) + u f2(v,w) 
          + (1-v) f1(u,w) + v f3(u,w) 
          + (1-w) f5(u,v) + w f6(u,v) 
          - [ (1-u)(1-v) c9(w) + (1-u)v c12(w) + u(1-v) c10(w) + uv c11(w) ]
          - [ (1-v)(1-w) c1(u) + (1-v)w c5(u)  + v(1-w) c3(u)  + vw c7(u)  ]
          - [ (1-u)(1-w) c4(v) + (1-w)u c2(v)  + w(1-u) c8(v)  + uw c6(v)  ]
          + [ (1-u)(1-v)(1-w) s1 + u(1-v)(1-w) s2 + uv(1-w) s3 + (1-u)v(1-w) s4 + 
	      (1-u)(1-v)w     s5 + u(1-v)w     s6 + uvw     s7 + (1-u)vw     s8 ]
*/

#define TRAN_HEX(f1,f2,f3,f4,f5,f6,c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,c11,c12,s1,s2,s3,s4,s5,s6,s7,s8,u,v,w) (1-u)*f4+u*f2+(1-v)*f1+v*f3+(1-w)*f5+w*f6-((1-u)*(1-v)*c9+(1-u)*v*c12+u*(1-v)*c10+u*v*c11)-((1-v)*(1-w)*c1+(1-v)*w*c5+v*(1-w)*c3+v*w*c7)-((1-u)*(1-w)*c4+(1-w)*u*c2+w*(1-u)*c8+u*w*c6)+(1-u)*(1-v)*(1-w)*s1+u*(1-v)*(1-w)*s2+u*v*(1-w)*s3+(1-u)*v*(1-w)*s4+(1-u)*(1-v)*w*s5+u*(1-v)*w*s6+u*v*w*s7+(1-u)*v*w*s8

Vertex TransfiniteHex(Vertex f1, Vertex f2, Vertex f3, Vertex f4, Vertex f5,
                      Vertex f6, Vertex c1, Vertex c2, Vertex c3, Vertex c4,
                      Vertex c5, Vertex c6, Vertex c7, Vertex c8, Vertex c9,
                      Vertex c10, Vertex c11, Vertex c12, Vertex s1,
                      Vertex s2, Vertex s3, Vertex s4, Vertex s5, Vertex s6,
                      Vertex s7, Vertex s8, double u, double v, double w)
{
  Vertex V;

  V.lc = TRAN_HEX(f1.lc, f2.lc, f3.lc, f4.lc, f5.lc, f6.lc,
                  c1.lc, c2.lc, c3.lc, c4.lc, c5.lc, c6.lc,
                  c7.lc, c8.lc, c9.lc, c10.lc, c11.lc, c12.lc,
                  s1.lc, s2.lc, s3.lc, s4.lc, s5.lc, s6.lc, s7.lc, s8.lc,
                  u, v, w);

  V.Pos.X =
    TRAN_HEX(f1.Pos.X, f2.Pos.X, f3.Pos.X, f4.Pos.X, f5.Pos.X, f6.Pos.X,
             c1.Pos.X, c2.Pos.X, c3.Pos.X, c4.Pos.X, c5.Pos.X, c6.Pos.X,
             c7.Pos.X, c8.Pos.X, c9.Pos.X, c10.Pos.X, c11.Pos.X, c12.Pos.X,
             s1.Pos.X, s2.Pos.X, s3.Pos.X, s4.Pos.X, s5.Pos.X, s6.Pos.X,
             s7.Pos.X, s8.Pos.X, u, v, w);

  V.Pos.Y =
    TRAN_HEX(f1.Pos.Y, f2.Pos.Y, f3.Pos.Y, f4.Pos.Y, f5.Pos.Y, f6.Pos.Y,
             c1.Pos.Y, c2.Pos.Y, c3.Pos.Y, c4.Pos.Y, c5.Pos.Y, c6.Pos.Y,
             c7.Pos.Y, c8.Pos.Y, c9.Pos.Y, c10.Pos.Y, c11.Pos.Y, c12.Pos.Y,
             s1.Pos.Y, s2.Pos.Y, s3.Pos.Y, s4.Pos.Y, s5.Pos.Y, s6.Pos.Y,
             s7.Pos.Y, s8.Pos.Y, u, v, w);

  V.Pos.Z =
    TRAN_HEX(f1.Pos.Z, f2.Pos.Z, f3.Pos.Z, f4.Pos.Z, f5.Pos.Z, f6.Pos.Z,
             c1.Pos.Z, c2.Pos.Z, c3.Pos.Z, c4.Pos.Z, c5.Pos.Z, c6.Pos.Z,
             c7.Pos.Z, c8.Pos.Z, c9.Pos.Z, c10.Pos.Z, c11.Pos.Z, c12.Pos.Z,
             s1.Pos.Z, s2.Pos.Z, s3.Pos.Z, s4.Pos.Z, s5.Pos.Z, s6.Pos.Z,
             s7.Pos.Z, s8.Pos.Z, u, v, w);

  return (V);
}

void Normal2Surface(Surface * s, double u, double v, double n[3])
{
  Vertex du, dv;
  double t1[3], t2[3];
  du = InterpolateSurface(s, u, v, 1, 1);
  dv = InterpolateSurface(s, u, v, 1, 2);
  t1[0] = du.Pos.X;
  t1[1] = du.Pos.Y;
  t1[2] = du.Pos.Z;
  t2[0] = dv.Pos.X;
  t2[1] = dv.Pos.Y;
  t2[2] = dv.Pos.Z;
  prodve(t1, t2, n);
  norme(n);
}

void HessianNormal2Surface(Surface * s, double u, double v, double n[3])
{
  Vertex du, dv;
  double t1[3], t2[3];
  du = InterpolateSurface(s, u, v, 1, 1);
  dv = InterpolateSurface(s, u, v, 1, 2);
  t1[0] = du.Pos.X;
  t1[1] = du.Pos.Y;
  t1[2] = du.Pos.Z;
  t2[0] = dv.Pos.X;
  t2[1] = dv.Pos.Y;
  t2[2] = dv.Pos.Z;
  prodve(t1, t2, n);
  norme(n);
}

// Cubic spline

Vertex InterpolateCubicSpline(Vertex * v[4], double t, double mat[4][4],
                              int derivee, double t1, double t2)
{
  Vertex V;
  int i, j;
  double vec[4], T[4];

  V.Pos.X = V.Pos.Y = V.Pos.Z = 0.0;
  V.lc = (1 - t) * v[1]->lc + t * v[2]->lc;
  V.w = (1 - t) * v[1]->w + t * v[2]->w;

  if(derivee) {
    T[3] = 0.;
    T[2] = 1.;
    T[1] = 2. * t;
    T[0] = 3. * t * t;
  }
  else {
    T[3] = 1.;
    T[2] = t;
    T[1] = t * t;
    T[0] = t * t * t;
  }

  for(i = 0; i < 4; i++) {
    vec[i] = 0.0;
  }

  /* X */
  for(i = 0; i < 4; i++) {
    for(j = 0; j < 4; j++) {
      vec[i] += mat[i][j] * v[j]->Pos.X;
    }
  }

  for(j = 0; j < 4; j++) {
    V.Pos.X += T[j] * vec[j];
    vec[j] = 0.0;
  }

  /* Y */
  for(i = 0; i < 4; i++) {
    for(j = 0; j < 4; j++) {
      vec[i] += mat[i][j] * v[j]->Pos.Y;
    }
  }

  for(j = 0; j < 4; j++) {
    V.Pos.Y += T[j] * vec[j];
    vec[j] = 0.0;
  }

  /* Z */
  for(i = 0; i < 4; i++) {
    for(j = 0; j < 4; j++) {
      vec[i] += mat[i][j] * v[j]->Pos.Z;
    }
  }
  for(j = 0; j < 4; j++) {
    V.Pos.Z += T[j] * vec[j];
    vec[j] = 0.0;
  }

  if(derivee) {
    V.Pos.X /= ((t2 - t1));
    V.Pos.Y /= ((t2 - t1));
    V.Pos.Z /= ((t2 - t1));
  }

  return V;
}


// Uniform BSplines

Vertex InterpolateUBS(Curve * Curve, double u, int derivee)
{
  int NbControlPoints, NbCurves, iCurve;
  double t, t1, t2;
  Vertex *v[4];

  NbControlPoints = List_Nbr(Curve->Control_Points);
  NbCurves = NbControlPoints - 3;

  iCurve = (int)(u * (double)NbCurves) + 1;

  if(iCurve > NbCurves)
    iCurve = NbCurves;
  else if (iCurve < 1)
    iCurve = 1;

  t1 = (double)(iCurve - 1) / (double)(NbCurves);
  t2 = (double)(iCurve) / (double)(NbCurves);

  t = (u - t1) / (t2 - t1);

  List_Read(Curve->Control_Points, iCurve - 1, &v[0]);
  List_Read(Curve->Control_Points, iCurve, &v[1]);
  List_Read(Curve->Control_Points, iCurve + 1, &v[2]);
  List_Read(Curve->Control_Points, iCurve + 2, &v[3]);

  return InterpolateCubicSpline(v, t, Curve->mat, derivee, t1, t2);
}

// Non Uniform BSplines

int findSpan(double u, int deg, int n, float *U)
{
  if(u >= U[n])
    return n - 1;
  if(u <= U[0])
    return deg;

  int low = deg;
  int high = n + 1;
  int mid = (low + high) / 2;

  while(u < U[mid] || u >= U[mid + 1]) {
    if(u < U[mid])
      high = mid;
    else
      low = mid;
    mid = (low + high) / 2;
  }
  return mid;
}

void basisFuns(double u, int i, int deg, float *U, double *N)
{
  double left[1000];
  double *right = &left[deg + 1];

  double temp, saved;

  //N.resize(deg+1) ;

  N[0] = 1.0;
  for(int j = 1; j <= deg; j++) {
    left[j] = u - U[i + 1 - j];
    right[j] = U[i + j] - u;
    saved = 0.0;
    for(int r = 0; r < j; r++) {
      temp = N[r] / (right[r + 1] + left[j - r]);
      N[r] = saved + right[r + 1] * temp;
      saved = left[j - r] * temp;
    }
    N[j] = saved;
  }
}

Vertex InterpolateNurbs(Curve * Curve, double u, int derivee)
{
  static double Nb[1000];
  int span =
    findSpan(u, Curve->degre, List_Nbr(Curve->Control_Points), Curve->k);
  Vertex p, *v;

  basisFuns(u, span, Curve->degre, Curve->k, Nb);
  p.Pos.X = p.Pos.Y = p.Pos.Z = p.w = p.lc = 0.0;
  for(int i = Curve->degre; i >= 0; --i) {
    List_Read(Curve->Control_Points, span - Curve->degre + i, &v);
    p.Pos.X += Nb[i] * v->Pos.X;
    p.Pos.Y += Nb[i] * v->Pos.Y;
    p.Pos.Z += Nb[i] * v->Pos.Z;
    p.w += Nb[i] * v->w;
    p.lc += Nb[i] * v->lc;
  }
  return p;
}

Vertex InterpolateNurbsSurface(Surface * s, double u, double v)
{
  int uspan = findSpan(u, s->OrderU, s->Nu, s->ku);
  int vspan = findSpan(v, s->OrderV, s->Nv, s->kv);
  double Nu[1000], Nv[1000];
  Vertex sp, temp[1000], *pv;

  basisFuns(u, uspan, s->OrderU, s->ku, Nu);
  basisFuns(v, vspan, s->OrderV, s->kv, Nv);

  int l, ll, kk;
  for(l = 0; l <= s->OrderV; l++) {
    temp[l].Pos.X = temp[l].Pos.Y = temp[l].Pos.Z = temp[l].w = temp[l].lc =
      0.0;
    for(int k = 0; k <= s->OrderU; k++) {
      kk = uspan - s->OrderU + k;
      ll = vspan - s->OrderV + l;
      List_Read(s->Control_Points, kk + s->Nu * ll, &pv);
      temp[l].Pos.X += Nu[k] * pv->Pos.X;
      temp[l].Pos.Y += Nu[k] * pv->Pos.Y;
      temp[l].Pos.Z += Nu[k] * pv->Pos.Z;
      temp[l].w += Nu[k] * pv->w;
      temp[l].lc += Nu[k] * pv->lc;
    }
  }
  sp.Pos.X = sp.Pos.Y = sp.Pos.Z = sp.w = sp.lc = 0.0;
  for(l = 0; l <= s->OrderV; l++) {
    sp.Pos.X += Nv[l] * temp[l].Pos.X;
    sp.Pos.Y += Nv[l] * temp[l].Pos.Y;
    sp.Pos.Z += Nv[l] * temp[l].Pos.Z;
    sp.w += Nv[l] * temp[l].w;
    sp.lc += Nv[l] * temp[l].lc;
  }
  return sp;
}


// Surface creation helpers

void CreateNurbsSurfaceSupport(int Num, int Order1, int Order2,
                               List_T * List, List_T * ku, List_T * kv)
{
  // This routine has been heavily modified to fit the new interfaces,
  // but has not been tested since then. It's probably full of bugs
  // now.
  List_T *ListOfDouble_L;
  List_T *ListCP = List_Create(2, 2, sizeof(int));

  for(int j = 0; j < List_Nbr(List); j++) {
    List_Read(List, j, &ListOfDouble_L);
    for(int i = 0; i < List_Nbr(ListOfDouble_L); i++) {
      double d;
      List_Read(ListOfDouble_L, i, &d);
      int N = (int)d;
      List_Add(ListCP, &N);
    }
  }
  List_Read(List, 0, &ListOfDouble_L);
  int Nu = List_Nbr(List);
  int Nv = List_Nbr(ListOfDouble_L);

  Surface *s = Create_Surface(Num, MSH_SURF_NURBS);
  s->Support = NULL;
  s->Control_Points = List_Create(4, 1, sizeof(Vertex *));
  s->OrderU = Order1;
  s->OrderV = Order2;
  s->Nu = Nu;
  s->Nv = Nv;
  for(int i = 0; i < List_Nbr(ListCP); i++) {
    int j;
    List_Read(ListCP, i, &j);
    Vertex *v = FindPoint(j);
    if(v){
      List_Add(s->Control_Points, &v);
    }
    else{
      Msg(GERROR, "Unknown control point %d in nurbs surface", j);
    }
  }

  s->ku = (float *)malloc(List_Nbr(ku) * sizeof(float));
  s->kv = (float *)malloc(List_Nbr(kv) * sizeof(float));

  double kumin = 0., kumax = 1.;
  double kvmin = 0., kvmax = 1.;

  for(int i = 0; i < List_Nbr(ku); i++) {
    double d;
    List_Read(ku, i, &d);
    float f = (float)((d - kumin) / (kumax - kumin));
    s->ku[i] = f;
  }
  for(int i = 0; i < List_Nbr(kv); i++) {
    double d;
    List_Read(kv, i, &d);
    float f = (float)((d - kvmin) / (kvmax - kvmin));
    s->kv[i] = f;
  }

  List_Delete(ListCP);

  End_Surface(s);
  Tree_Add(THEM->Surfaces, &s);
}

void CreateNurbsSurface(int Num, int Order1, int Order2, List_T * List,
                        List_T * ku, List_T * kv)
{
  // This routine has been heavily modified to fit the new interfaces,
  // but has not been tested since then. It's probably full of bugs
  // now.

  List_T *ListOfDouble_L, *Listint, *ListCP;
  int Loop[4];

  ListCP = List_Create(2, 2, sizeof(int));

  double kumin, kumax;
  List_Read(ku, 0, &kumin);
  List_Read(ku, List_Nbr(ku) - 1, &kumax);
  double kvmin, kvmax;
  List_Read(kv, 0, &kvmin);
  List_Read(kv, List_Nbr(kv) - 1, &kvmax);
  for(int j = 0; j < List_Nbr(List); j++) {
    List_Read(List, j, &ListOfDouble_L);
    for(int i = 0; i < List_Nbr(ListOfDouble_L); i++) {
      double d;
      List_Read(ListOfDouble_L, i, &d);
      int N = (int)d;
      List_Add(ListCP, &N);
    }
  }

  // 1st and 3rd gen
  List_Read(List, 0, &ListOfDouble_L);
  Listint = ListOfDouble2ListOfInt(ListOfDouble_L);
  if(recognize_seg(MSH_SEGM_NURBS, Listint, &Loop[0])) {
  }
  else {
    Loop[0] = NEWREG();
    Curve *c = Create_Curve(Loop[0], MSH_SEGM_NURBS, Order1, Listint, NULL, 
			    -1, -1, kumin, kumax);
    Tree_Add(THEM->Curves, &c);
    CreateReversedCurve(c);
    c->k = (float *)malloc(4 * List_Nbr(ku) * sizeof(float));
    for(int i = 0; i < List_Nbr(ku); i++) {
      double d;
      List_Read(ku, i, &d);
      c->k[i] = (float)d /*((d-kumin)/(kumax-kumin)) */ ;
    }
  }
  List_Delete(Listint);

  List_Read(List, List_Nbr(List) - 1, &ListOfDouble_L);
  Listint = ListOfDouble2ListOfInt(ListOfDouble_L);
  if(recognize_seg(MSH_SEGM_NURBS, Listint, &Loop[2])) {
  }
  else {
    Loop[2] = NEWREG();
    Curve *c = Create_Curve(Loop[2], MSH_SEGM_NURBS, Order1, Listint, NULL, 
			    -1, -1, kumin, kumax);
    Tree_Add(THEM->Curves, &c);
    CreateReversedCurve(c);
    c->k = (float *)malloc(4 * List_Nbr(ku) * sizeof(float));
    for(int i = 0; i < List_Nbr(ku); i++) {
      double d;
      List_Read(ku, i, &d);
      c->k[i] = (float)d /*((d-kumin)/(kumax-kumin)) */ ;
    }
  }
  List_Delete(Listint);

  // 2nd and 4th gen
  List_T *List1 = List_Create(List_Nbr(List), 1, sizeof(double));
  List_T *List2 = List_Create(List_Nbr(List), 1, sizeof(double));
  for(int i = 0; i < List_Nbr(List); i++) {
    List_Read(List, i, &ListOfDouble_L);
    List_Add(List1, List_Pointer(ListOfDouble_L, 0));
    List_Add(List2, List_Pointer(ListOfDouble_L, List_Nbr(ListOfDouble_L) - 1));
  }

  Listint = ListOfDouble2ListOfInt(List1);
  if(recognize_seg(MSH_SEGM_NURBS, Listint, &Loop[1])) {
  }
  else {
    Loop[1] = NEWREG();
    Curve *c = Create_Curve(Loop[1], MSH_SEGM_NURBS, Order2, Listint, NULL, 
			    -1, -1, kumin, kumax);
    Tree_Add(THEM->Curves, &c);
    CreateReversedCurve(c);
    c->k = (float *)malloc(4 * List_Nbr(kv) * sizeof(float));
    for(int i = 0; i < List_Nbr(kv); i++) {
      double d;
      List_Read(kv, i, &d);
      c->k[i] = (float)d /*((d-kvmin)/(kvmax-kvmin)) */ ;
    }
  }
  List_Delete(Listint);

  Listint = ListOfDouble2ListOfInt(List2);
  if(recognize_seg(MSH_SEGM_NURBS, Listint, &Loop[3])) {
  }
  else {
    Loop[3] = NEWREG();
    Curve *c = Create_Curve(Loop[3], MSH_SEGM_NURBS, Order2, Listint, NULL,
			    -1, -1, kumin, kumax);
    Tree_Add(THEM->Curves, &c);
    CreateReversedCurve(c);
    c->k = (float *)malloc(4 * List_Nbr(kv) * sizeof(float));
    for(int i = 0; i < List_Nbr(kv); i++) {
      double d;
      List_Read(kv, i, &d);
      c->k[i] = (float)d /*((d-kvmin)/(kvmax-kvmin)) */ ;
    }
  }
  List_Delete(Listint);
  List_Delete(List1);
  List_Delete(List2);

  Listint = List_Create(10, 10, sizeof(int));
  int l0 = -Loop[0];
  List_Add(Listint, &l0);
  List_Add(Listint, &Loop[1]);
  List_Add(Listint, &Loop[2]);
  int l3 = -Loop[3];
  List_Add(Listint, &l3);

  int topnew = NEWREG();
  CreateNurbsSurfaceSupport(topnew, Order1, Order2, List, ku, kv);

  int il = NEWREG();
  SurfaceLoop *l = Create_SurfaceLoop(il, Listint);
  Tree_Add(THEM->SurfaceLoops, &l);
  List_Reset(Listint);
  List_Add(Listint, &il);

  Surface *s = Create_Surface(NEWREG(), MSH_SURF_TRIMMED);
  setSurfaceGeneratrices(s, Listint);
  s->Support = s;
  End_Surface(s);
  Tree_Add(THEM->Surfaces, &s);

  List_Delete(Listint);
  List_Delete(ListCP);
}

