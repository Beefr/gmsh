// $Id: 1D_Mesh.cpp,v 1.16 2001-05-29 13:26:54 geuzaine Exp $

#include "Gmsh.h"
#include "Const.h"
#include "Geo.h"
#include "Mesh.h"
#include "Context.h"
#include "Interpolation.h"
#include "Numeric.h"

extern Mesh      *THEM;
extern Context_T  CTX;
extern int        CurrentNodeNumber;

Curve *THEC;

// ipar[0] = nbpoints
// abs(ipar[1]) = method
// sign(ipar[1]) = orientation
// dpar[0] = parameter

double F_One (double t){
  Vertex der;
  double d;
  der = InterpolateCurve (THEC, t, 1);
  d = sqrt (der.Pos.X * der.Pos.X + der.Pos.Y * der.Pos.Y + der.Pos.Z * der.Pos.Z);
  return (d);
}


double F_Transfini (double t){
  Vertex der;
  double d, a, b, val, ZePauwer;

  der = InterpolateCurve (THEC, t, 1);
  d = sqrt (der.Pos.X * der.Pos.X + der.Pos.Y * der.Pos.Y +
            der.Pos.Z * der.Pos.Z);

  if (THEC->dpar[0] == 0.0 || THEC->dpar[0] == 1.0){
    val = d * (double) THEC->ipar[0] / THEC->l ;
  }
  else{
    switch (abs (THEC->ipar[1])){

    case 1: // progression
      if (sign (THEC->ipar[1]) == -1)
	ZePauwer = 1. / THEC->dpar[0];
      else
	ZePauwer = THEC->dpar[0];
      b = log (1. / ZePauwer) / THEC->l;
      a = (1. - exp (-b * THEC->l)) / (b * (double) THEC->ipar[0]);
      val = d / (a * exp (b * (t * THEC->l))) ;
      break ;

    case 2: //bump
      if (THEC->dpar[0] > 1.0){
        a = -4. * sqrt (THEC->dpar[0] - 1.) * 
          atan2 (1., sqrt (THEC->dpar[0] - 1.)) / 
          ((double) THEC->ipar[0] * THEC->l);
      }
      else{
        a = 2. * sqrt (1. - THEC->dpar[0]) * 
          log (fabs ((1. + 1. / sqrt (1. - THEC->dpar[0])) 
                     / (1. - 1. / sqrt (1. - THEC->dpar[0]))))
          / ((double) THEC->ipar[0] * THEC->l);
      }
      b = -a * THEC->l * THEC->l / (4. * (THEC->dpar[0] - 1.)) ;
      val = d / (-a * DSQR (t * THEC->l - (THEC->l) * 0.5) + b) ;
      break ;

    default:
      Msg(WARNING, "Unknown case in Transfinite Line mesh");
      val = 1. ;
    }
  }

  return val ;
}

double F_Lc (double t){
  Vertex  der, point;
  double  Lc, d;

  if (CTX.mesh.algo == DELAUNAY_OLDALGO && THEM->BGM.Typ == ONFILE){
    der = InterpolateCurve(THEC, t, 1);
    point = InterpolateCurve(THEC, t, 0);  
    Lc = Lc_XYZ(point.Pos.X, point.Pos.Y, point.Pos.Z, THEM);
    d = sqrt(DSQR(der.Pos.X)+DSQR(der.Pos.Y)+DSQR(der.Pos.Z));
    if(!Lc){
      Msg(GERROR, "Null characteristic length in background mesh");
      return d;
    }
    if(CTX.mesh.constrained_bgmesh)
      return MAX(d/Lc,THEM->Metric->getLc(t, THEC));
    else
      return d/Lc;
  }
  else
    return THEM->Metric->getLc(t, THEC);
}

void Maillage_Curve (void *data, void *dummy){
  Curve **pc, *c;
  Simplex *s;
  double b, a, d, dt, dp, t;
  int i, N, count, NUMP;
  Vertex **v, **vexist, *pV, V, *v1, *v2;
  List_T *Points;
  IntPoint P1, P2;

  pc = (Curve **) data;
  c = *pc;
  THEC = c;

  if (c->Num < 0)
    return;

  Msg(STATUS3, "Meshing Curve %d", c->Num);

  Points = List_Create (10, 10, sizeof (IntPoint));
  c->l = Integration (c->ubeg, c->uend, F_One, Points, 1.e-4);
  List_Delete (Points);

  if (c->Method == TRANSFINI || !Extrude_Mesh (c)){
    if (c->Method == TRANSFINI){
      Points = List_Create (10, 10, sizeof (IntPoint));
      a = Integration (c->ubeg, c->uend, F_Transfini, Points, 1.e-7);
      N = c->ipar[0];
    }
    else{
      Points = List_Create (10, 10, sizeof (IntPoint));
      a = Integration (c->ubeg, c->uend, F_Lc, Points, 1.e-4);
      N = IMAX (2, (int) (a + 1.));

      if (c->Typ == MSH_SEGM_CIRC ||
          c->Typ == MSH_SEGM_CIRC_INV ||
          c->Typ == MSH_SEGM_ELLI ||
          c->Typ == MSH_SEGM_ELLI_INV){
        N = IMAX (N, (int) (fabs (c->Circle.t1 - c->Circle.t2) *
                            (double)CTX.mesh.min_circ_points / Pi));
      }
      else if (c->Typ == MSH_SEGM_NURBS){
        N = IMAX (N, 2);
      }
    }
    b = a / (double) (N - 1);
    c->Vertices = List_Create (N, 2, sizeof (Vertex *));
    
    v = &c->beg;
    if ((vexist = (Vertex **) Tree_PQuery (THEM->Vertices, v))){
      (*vexist)->u = c->ubeg;
      Tree_Insert (THEM->Vertices, vexist);
      if ((*vexist)->ListCurves)
        List_Add ((*vexist)->ListCurves, &c);
      List_Add (c->Vertices, vexist);
    }
    else{
      pV = Create_Vertex ((*v)->Num, (*v)->Pos.X, (*v)->Pos.Y,
                          (*v)->Pos.Z, (*v)->lc, 0.0);
      pV->ListCurves = List_Create (1, 1, sizeof (Curve *));
      List_Add (pV->ListCurves, &c);
      Tree_Insert (THEM->Vertices, &pV);
      List_Add (c->Vertices, &pV);
    }

    count = NUMP = 1;
    while (NUMP < N - 1){
      List_Read (Points, count - 1, &P1);
      List_Read (Points, count, &P2);
      d = (double) NUMP *b;

      if ((fabs (P2.p) >= fabs (d)) && (fabs (P1.p) < fabs (d))){
        dt = P2.t - P1.t;
        dp = P2.p - P1.p;
        t = P1.t + dt / dp * (d - P1.p);
        V = InterpolateCurve (c, t, 0);
        pV = Create_Vertex (++CurrentNodeNumber, 
                            V.Pos.X, V.Pos.Y, V.Pos.Z, V.lc, t);
        pV->w = V.w;
        pV->ListCurves = List_Create (1, 1, sizeof (Curve *));
        List_Add (pV->ListCurves, &c);
        Tree_Insert (THEM->Vertices, &pV);
        List_Add (c->Vertices, &pV);
        NUMP++;
      }
      else{
        count++;
      }
    }

    List_Delete(Points);

    v = &c->end;
    if ((vexist = (Vertex **) Tree_PQuery (THEM->Vertices, v))){
      (*vexist)->u = c->uend;
      Tree_Insert (THEM->Vertices, vexist);
      if ((*vexist)->ListCurves)
        List_Add ((*vexist)->ListCurves, &c);
      List_Add (c->Vertices, vexist);
    }
    else{
      pV = Create_Vertex ((*v)->Num, (*v)->Pos.X, (*v)->Pos.Y, 
                          (*v)->Pos.Z, (*v)->lc, 0.0);
      pV->ListCurves = List_Create (1, 1, sizeof (Curve *));
      List_Add (pV->ListCurves, &c);
      Tree_Insert (THEM->Vertices, &pV);
      List_Add (c->Vertices, &pV);
    }
  }

  for (i = 0; i < List_Nbr (c->Vertices) - 1; i++){
    List_Read (c->Vertices, i, &v1);
    List_Read (c->Vertices, i + 1, &v2);
    s = Create_Simplex (v1, v2, NULL, NULL);
    s->iEnt = c->Num;
    Tree_Add (c->Simplexes, &s);
    List_Add (c->TrsfSimplexes, &s);
  }

  if (CTX.mesh.degree == 2)
    Degre2 (THEM->Vertices, THEM->VertexEdges, c->Simplexes, c, NULL);

  THEM->Statistics[4] += List_Nbr (c->Vertices);

}
