// $Id: OCCEdge.cpp,v 1.13 2006-11-27 22:22:14 geuzaine Exp $
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

#include "GModel.h"
#include "Message.h"
#include "OCCEdge.h"
#include "OCCFace.h"

#if defined(HAVE_OCC)
#include "Geom_BSplineCurve.hxx"
#include "Geom_BezierCurve.hxx"
#include "Geom_Ellipse.hxx"
#include "Geom_Circle.hxx"
#include "Geom_Line.hxx"

OCCEdge::OCCEdge(GModel *model, TopoDS_Edge edge, int num, GVertex *v1, GVertex *v2)
  : GEdge(model, num, v1, v2), c(edge), trimmed(0)
{
  curve = BRep_Tool::Curve(c, s0, s1);
}

Range<double> OCCEdge::parBounds(int i) const
{ 
  //  double a,b;
  //  BRep_Tool::Range (c,a,b); 
  return(Range<double>(s0,s1));
}

void OCCEdge::setTrimmed (OCCFace *f)
{
  if (!trimmed)
    {
      trimmed = f;
      const TopoDS_Face *s = (TopoDS_Face*) trimmed->getNativePtr();
      curve2d = BRep_Tool::CurveOnSurface(c, *s, s0, s1);
      if (curve2d.IsNull())  trimmed = 0;
    }
}

SPoint2 OCCEdge::reparamOnFace(GFace *face, double epar,int dir) const
{
  const TopoDS_Face *s = (TopoDS_Face*) face->getNativePtr();
  double t0,t1;
  Handle(Geom2d_Curve) c2d = BRep_Tool::CurveOnSurface(c, *s, t0, t1);
  if (c2d.IsNull())
    return GEdge::reparamOnFace(face, epar,dir);
  double u,v;
  c2d->Value(epar).Coord(u,v);
  if (! isSeam ( face ) )
    {
      return SPoint2 (u,v);
    }
  else
    {
      BRepAdaptor_Surface surface( *s );

//       printf ("surface %d (%d %d) firstu %g lastu %g firstv %g lastv %g\n",
// 	      surface.IsUPeriodic() ,
// 	      surface.IsVPeriodic() ,
// 	      face->tag(),
// 	      surface.FirstUParameter(),
// 	      surface.LastUParameter(),
// 	      surface.FirstVParameter(),
// 	      surface.LastVParameter());

      if ( surface.IsUPeriodic() )
	{
	  if (dir == -1) 
	    return SPoint2(surface.FirstUParameter(), v);
	  else
	    return SPoint2(surface.LastUParameter(), v);
	}
      else {
	if (dir == -1) 
	  return SPoint2(u , surface.FirstVParameter());
	else
	  return SPoint2(u , surface.LastVParameter());
      }
    }
}

/** True if the edge is a seam for the given face. */
int OCCEdge::isSeam(GFace *face) const
{
  const TopoDS_Face *s = (TopoDS_Face*) face->getNativePtr();
  BRepAdaptor_Surface surface( *s );
  if ( surface.IsUPeriodic() || surface.IsVPeriodic() )
    {
      return BRep_Tool::IsClosed( c, *s );
    }
  return 0;
}


GPoint OCCEdge::point(double par) const
{
  if (!curve.IsNull())
    {
      gp_Pnt pnt = curve->Value (par);
      return GPoint(pnt.X(),pnt.Y(),pnt.Z());
    }
  else if (trimmed)
    {
      double u,v;
      curve2d->Value(par).Coord(u,v);
      return trimmed->point(u,v);
    }
  else
    {
      Msg(WARNING,"OCC Curve %d is neither a 3D curve not a trimmed curve",tag());
      return GPoint (0,0,0);
    }
}

GPoint OCCEdge::closestPoint(const SPoint3 & qp)
{
  throw;
}

int OCCEdge::containsParam(double pt) const
{
  Range<double> rg = parBounds(0);
  return (pt >= rg.low() && pt <= rg.high());
}

SVector3 OCCEdge::firstDer(double par) const
{  
  BRepAdaptor_Curve brepc(c);
  BRepLProp_CLProps prop(brepc, 1, 1e-5);
  prop.SetParameter (par);
  gp_Vec d1 = prop.D1();
  return SVector3(d1.X(), d1.Y(), d1.Z());
}

double OCCEdge::parFromPoint(const SPoint3 &pt) const
{
  throw;
}

GEntity::GeomType OCCEdge::geomType() const
{
  if (curve.IsNull())
    {
      if (curve2d->DynamicType() == STANDARD_TYPE(Geom_Circle))
	return Circle;
      else if (curve2d->DynamicType() == STANDARD_TYPE(Geom_Line))
	return Line;
      else if (curve2d->DynamicType() == STANDARD_TYPE(Geom_Ellipse))
	return Ellipse;
      else if (curve2d->DynamicType() == STANDARD_TYPE(Geom_BSplineCurve))
	return BSpline;
      else if (curve2d->DynamicType() == STANDARD_TYPE(Geom_BezierCurve))
	return Bezier;
      //   else if (occface->DynamicType() == STANDARD_TYPE(Geom_ConicalSurface))
      //     return Cone;
      return Unknown;
    }
  else
    {
      if (curve->DynamicType() == STANDARD_TYPE(Geom_Circle))
	return Circle;
      else if (curve->DynamicType() == STANDARD_TYPE(Geom_Line))
	return Line;
      else if (curve->DynamicType() == STANDARD_TYPE(Geom_Ellipse))
	return Ellipse;
      else if (curve->DynamicType() == STANDARD_TYPE(Geom_BSplineCurve))
	return BSpline;
      else if (curve->DynamicType() == STANDARD_TYPE(Geom_BezierCurve))
	return Bezier;
      //   else if (occface->DynamicType() == STANDARD_TYPE(Geom_ConicalSurface))
      //     return Cone;
      return Unknown;
    }
}

int OCCEdge::minimumMeshSegments () const
{
  if(geomType() == Circle || geomType() == Ellipse)
    return (int)(fabs(s1 - s0) *
		 (double)CTX.mesh.min_circ_points / Pi) - 1;
  else
    return GEdge::minimumMeshSegments () ;
}

int OCCEdge::minimumDrawSegments () const
{
  int n = GEdge::minimumDrawSegments();

  if(geomType() == Line)
    return n;
  else if(geomType() == Circle || geomType() == Ellipse)
    return CTX.geom.circle_points;
  else
    return 20 * n;
}


double OCCEdge::curvature(double par) const 
{
  return GEdge::curvature(par);
  BRepAdaptor_Curve brepc(c);
  BRepLProp_CLProps prop(brepc, 1, 1e-5);
  prop.SetParameter (par); 
  printf("curvature = %12.5E\n",prop.Curvature()); 
  return fabs(prop.Curvature());
}
#endif
