#include "gmshModel.h"
#include "gmshEdge.h"
#include "Interpolation.h"
#include "CAD.h"
#include "Geo.h"
#include "Mesh.h"
#include "Create.h"
#include "Context.h"

extern Context_T CTX;
extern Mesh *THEM;

gmshEdge::gmshEdge(GModel *model, Curve *edge, GVertex *v1, GVertex *v2)
  : GEdge(model, edge->Num, v1, v2), c(edge)
{
}

gmshEdge::gmshEdge(GModel *model, int num)
  : GEdge(model, num, 0, 0)
{
  c = Create_Curve(num, MSH_SEGM_DISCRETE, 0, NULL, NULL, -1, -1, 0., 1.);
  Tree_Add(THEM->Curves, &c);
  CreateReversedCurve(THEM, c);
}

Range<double> gmshEdge::parBounds(int i) const
{ 
  return(Range<double>(c->ubeg, c->uend));
}

GPoint gmshEdge::point(double par) const
{
  Vertex a = InterpolateCurve(c, par, 0);
  return GPoint(a.Pos.X,a.Pos.Y,a.Pos.Z,this,par);
}

GPoint gmshEdge::closestPoint(const SPoint3 & qp)
{
  Vertex v;
  Vertex a;
  Vertex der;
  v.Pos.X = qp.x();
  v.Pos.Y = qp.y();
  v.Pos.Z = qp.z();
  ProjectPointOnCurve(c, &v, &a, &der);
  return GPoint(a.Pos.X, a.Pos.Y, a.Pos.Z, this, a.u);
}

int gmshEdge::containsParam(double pt) const
{
  Range<double> rg = parBounds(0);
  return (pt >= rg.low() && pt <= rg.high());
}

SVector3 gmshEdge::firstDer(double par) const
{
  Vertex a = InterpolateCurve(c, par, 1);
  return SVector3(a.Pos.X, a.Pos.Y, a.Pos.Z);
}

double gmshEdge::parFromPoint(const SPoint3 &pt) const
{
  Vertex v;
  Vertex a;
  Vertex der;
  v.Pos.X = pt.x();
  v.Pos.Y = pt.y();
  v.Pos.Z = pt.z();
  ProjectPointOnCurve(c, &v, &a, &der);
  return a.u;
}

GEntity::GeomType gmshEdge::geomType() const
{
  switch (c->Typ){
  case MSH_SEGM_LINE : return Line;
  case MSH_SEGM_PARAMETRIC : return ParametricCurve;
  case MSH_SEGM_CIRC :  
  case MSH_SEGM_CIRC_INV : return Circle;
  case MSH_SEGM_ELLI:
  case MSH_SEGM_ELLI_INV: return Ellipse;
  case MSH_SEGM_BSPLN:
  case MSH_SEGM_BEZIER: 
  case MSH_SEGM_NURBS:
  case MSH_SEGM_SPLN: return Nurb; 
  case MSH_SEGM_DISCRETE: return DiscreteCurve; 
  default : return Unknown;
  }
}

int gmshEdge::minimumMeshSegments () const
{
  if(geomType() == Circle || geomType() == Ellipse)
    return (int)(fabs(c->Circle.t1 - c->Circle.t2) *
		 (double)CTX.mesh.min_circ_points / Pi) - 1;
  else
    return GEdge::minimumMeshSegments () ;
}

int gmshEdge::minimumDrawSegments () const
{
  int n = List_Nbr(c->Control_Points) - 1;
  if(!n) n = GEdge::minimumDrawSegments();

  if(geomType() == Line)
    return n;
  else if(geomType() == Circle || geomType() == Ellipse)
    return CTX.geom.circle_points;
  else
    return 10 * n;
}
