#ifndef _GFACE_H_
#define _GFACE_H_

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

#include "GPoint.h"
#include "GEntity.h"
#include "MElement.h"
#include "SPoint2.h"
#include "SVector3.h"
#include "Pair.h"
#include "ExtrudeParams.h"

#include <set>

struct mean_plane
{
  double plan[3][3];
  double a, b, c, d;
  double x, y, z;
};

class GRegion;

// A model face. 
class GFace : public GEntity 
{
 protected: 
  // in a manifold representation, edges can
  // be taken twice in the topology in order 
  // to represent the periodicity of the surface
  std::set<GEdge *> edges_taken_twice;
  // list of al the edges of the face
  std::list<GEdge *> l_edges;
  std::list<int> l_dirs;
  GRegion *r1, *r2;
  mean_plane meanPlane;
  std::list<GEdge *> embedded_edges;
  std::list<GVertex *> embedded_vertices;
  void computeDirs ();

 public:
  GFace(GModel *model, int tag);
  virtual ~GFace();

  void addRegion(GRegion *r){ r1 ? r2 = r : r1 = r; }
  void delRegion(GRegion *r){ if(r1 == r) r1 = r2; r2=0; }

  // edge orientations.
  virtual std::list<int> orientations() const{return l_dirs;}
  // Edges that bound this entity or that this entity bounds.
  virtual std::list<GEdge*> edges() const{return l_edges;}
  // Edges that are embedded in this face.
  virtual std::list<GEdge*> emb_edges() const{return embedded_edges;}
  // Vertices that bound this entity or that this entity bounds.
  virtual std::list<GVertex*> vertices() const;

  virtual int dim() const {return 2;}
  virtual void setVisibility(char val, bool recursive=false);

  // compute XYZ from parametric UV
  void XYZtoUV(const double X, const double Y, const double Z, 
	       double &U, double &V,
	       const double relax) const;

  // The bounding box
  virtual SBoundingBox3d bounds() const; 

  // Get the location of any parametric degeneracies on the face in
  // the given parametric direction.
  virtual int paramDegeneracies(int dir, double *par) = 0;

  // Return the point on the face corresponding to the given parameter.
  virtual GPoint point(double par1, double par2) const = 0;
  virtual GPoint point(const SPoint2 &pt) const = 0;

  // Return the parmater location on the face given a point in space
  // that is on the face.
  virtual SPoint2            parFromPoint(const SPoint3 &) const;
  virtual void               parFromPoint(const SPoint3 &, std::list<double> &u, std::list<double> &v ) const;

  // True if the parameter value is interior to the face.
  virtual int containsParam(const SPoint2 &pt) const = 0;

  // Period of the face in the given direction.
  virtual double period(int dir) const = 0;

  // Return the point on the face closest to the given point.
  virtual GPoint closestPoint(const SPoint3 & queryPoint) const = 0;

  // Return the normal to the face at the given parameter location.
  virtual SVector3 normal(const SPoint2 &param) const = 0;

  // Return the first derivate of the face at the parameter location.
  virtual Pair<SVector3,SVector3> firstDer(const SPoint2 &param) const = 0;

  // Return the curvature i.e. the divergence of the normal
  virtual double curvature(const SPoint2 &param) const;
  
  // True if the surface underlying the face is periodic and we need
  // to worry about that.
  virtual bool surfPeriodic(int dim) const = 0;

  // Recompute the mesh partitions defined on this face.
  void recomputeMeshPartitions();

  // Delete the mesh partitions defined on this face.
  void deleteMeshPartitions();

  // Recompute the mean plane of the surface from a list of points
  void computeMeanPlane(const std::vector<MVertex*> &points);
  void computeMeanPlane(const std::vector<SPoint3> &points);

  // Recompute the mean plane of the surface from its bounding vertices
  void computeMeanPlane();

  // Get the mean plane info
  void getMeanPlaneData(double VX[3], double VY[3], 
			double &x, double &y, double &z) const;

  // a crude graphical representation using a "cross" defined by pairs
  // of start/end points
  std::vector<SPoint3> cross;

  std::vector<MTriangle*> triangles;
  std::vector<MQuadrangle*> quadrangles;

  struct {
    // do we recombine the triangles of the mesh ?
    int recombine;
    // what is the treshold angle for recombination
    double recombineAngle;
    // is this surface meshed using a transfinite interpolation
    int Method;
    // these are the 3 corners of the interpolation
    std::vector<GVertex*> corners;
    // all diagonals of the triangulation are left (1), right (2) or
    // alternated (3)
    int transfiniteArrangement;
    // the extrusion parameters (if any)
    ExtrudeParams *extrude;
  } meshAttributes ;

};

#endif
