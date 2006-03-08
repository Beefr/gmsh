#include "gmshModel.h"
#include "gmshEdge.h"
#include "gmshFace.h"
#include "Interpolation.h"
#include "CAD.h"
#include "Geo.h"

Range<double> gmshFace::parBounds(int i) const
{ 
/*  SPAinterval range;
  if(i ==0)
    range = acisSurface()->equation().param_range_u();
  if(i==1)
    range = acisSurface()->equation().param_range_v();

  if(range.finite() )
    return( Range<double>(range.start_pt(),range.end_pt()) );
  else{
    //printf("*calcParBounds()* is called\n");
    SBoundingBox2d b = calcParBounds();
    SPoint2 min = b.min();
    SPoint2 max = b.max();
    return Range<double>(min[i],max[i]);
  }
*/
  return Range<double>(0, 1);
}

int gmshFace::paramDegeneracies(int dir, double *par)
{
  return 0;
}

SBoundingBox3d gmshFace::bounds() const
{
  throw;
}

SVector3 gmshFace::normal(const SPoint2 &param) const
{
  Vertex vu = InterpolateSurface( s, param[0], param[1],1,1);
  Vertex vv = InterpolateSurface( s, param[0], param[1],1,2);
  Vertex n = vu % vv;
  n.norme();
  return SVector3(n.Pos.X,n.Pos.Y,n.Pos.Z);
}

Pair<SVector3,SVector3> gmshFace::firstDer(const SPoint2 &param) const
{
  Vertex vu = InterpolateSurface( s, param[0], param[1],1,1);
  Vertex vv = InterpolateSurface( s, param[0], param[1],1,2);
  return Pair<SVector3,SVector3>( SVector3(vu.Pos.X,vu.Pos.Y,vu.Pos.Z),
				  SVector3(vv.Pos.X,vv.Pos.Y,vv.Pos.Z));
}

double * gmshFace::nthDerivative(const SPoint2 &param, int n, double *array) const
{
  throw;
}

GFPoint gmshFace::point(const SPoint2 &pt) const
{   
    return point(pt.x(),pt.y()); 
}

GFPoint gmshFace::point(double par1,double par2) const
{
  Vertex v = InterpolateSurface( s, par1, par2,0,0);
  return GFPoint(v.Pos.X,v.Pos.Y,v.Pos.Z,this,SPoint2(par1,par2));
}

GFPoint gmshFace::closestPoint(const SPoint3 & qp)
{
  Vertex v;

  v.Pos.X = qp.x();
  v.Pos.Y = qp.y();
  v.Pos.Z = qp.z();

  if ( s->Typ != MSH_SURF_PLAN )
    ProjectPointOnSurface(s, v);
  return GFPoint(v.Pos.X,v.Pos.Y,v.Pos.Z,
		 this,SPoint2(v.us[0],v.us[1]));
}

int gmshFace::containsParam(const SPoint2 &pt) const
{
  Range<double> uu = parBounds(0);
  Range<double> vv = parBounds(1);
  if ((pt.x()>=uu.low() && pt.x()<=uu.high()) && (pt.y()>=vv.low() && pt.y()<=vv.high()))
     return 1;
  else 
     return 0;
}


SPoint2 gmshFace::parFromPoint(const SPoint3 &qp) const
{
  Vertex v;
  v.Pos.X = qp.x();
  v.Pos.Y = qp.y();
  v.Pos.Z = qp.z();
  ProjectPointOnSurface(s, v);
  SPoint2 pt2(v.us[0],v.us[1]); 
  return pt2;
}
/*

GeoRep * gmshFace::geometry()
{ return new gmshGeoRep(this,2); }
*/
Logical::Value gmshFace::continuous(int dim) const
{ 
  return Logical::True;
}

Logical::Value gmshFace::periodic(int dim) const
{ 
  return Logical::False;
}

Logical::Value gmshFace::degenerate(int dim) const
{ 
  return Logical::False;
}

GeomType::Value gmshFace::geomType() const
{
  int type;
  type = s->Typ;
  //if(type == CONE_TYPE)
  //return GeomType::Cone;
  if(type == MSH_SURF_NURBS)
    return GeomType::Nurb;
  if(type == MSH_SURF_PLAN)
    return GeomType::Plane;
  return GeomType::Unknown;

}

int gmshFace::geomDirection() const
{
  return 1;
}

double gmshFace::tolerance() const
{ return 1.e-14; }



void * gmshFace::getNativePtr() const
{ return s; }


// added 200306
int gmshFace::containsPoint(const SPoint3 &pt) const
{ 
  throw;
}

// added 200306
double gmshFace::period(int dir) const
{
  return 0.;
}
