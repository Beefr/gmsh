#ifndef H_GEntity
#define H_GEntity

#include "Range.h"
#include "SPoint3.h"
#include "SBoundingBox3d.h"
#include "MVertex.h"
#include "GmshDefines.h"
#include <list>
#include <vector>

class GModel;
class GVertex;
class GEdge;
class GFace;
class GRegion;
//class MeshRep;

/** A geometric model entity. All enitites are owned by a GModel. */
class GEntity {
  int _tag;
  GModel *_model;
  //  DiscreteRep *mesh, *modelMesh;
public:

  enum GeomType{
    Unknown,
    Discrete,
    Point,
    Line,
    Circle,
    Ellipse,
    ParametricCurve,
    Plane,
    Nurb,
    Cylinder,
    Sphere,
    Cone,
    Torus,
    ParametricSurface,
    ThreeDimVolume
    };

  GEntity(GModel *m, int t) : _model(m),_tag(t){
    drawAttributes.Visible = VIS_GEOM | VIS_MESH; 
    drawAttributes.Frozen = 0; 
  }

  virtual ~GEntity() {};

  /** Return a renderable representation of the entity.*/
  //  virtual MeshRep * getGeometry() ;
  /** Return a mesh of the entity */
  //  virtual MeshRep * getMesh()  ;

  /// Spatial dimension of the entity. 
  virtual int dim() const = 0;

  /** Returns true if ent is in the closure of this entity */
  virtual int inClosure(GEntity *ent) const {throw;} 

  /// Regions that bound this entity or that this entity bounds.
  virtual std::list<GRegion*> regions() const{throw;}

  /// Faces that bound this entity or that this entity bounds.
  virtual std::list<GFace*> faces() const{throw;}

  /// Edges that bound this entity or that this entity bounds.
  virtual std::list<GEdge*> edges() const{throw;}

  /// Vertices that bound this entity.
  virtual std::list<GVertex*> vertices() const{throw;}

  /// Underlying geometric representation of this entity.
  virtual GeomType geomType() const = 0;

  /// True if parametric space is continuous in the "dim" direction.
  virtual bool continuous(int dim) const {return true;}

  /// True if entity is periodic in the "dim" direction.
  virtual bool periodic(int dim) const {return false;}

  /// True if there are parametric degeneracies in the "dim" direction.
  virtual bool degenerate(int dim) const {return false;}

  /// Parametric bounds of the entity in the "i" direction.
  virtual Range<double> parBounds(int i) const{throw;}

  /// Modeler tolerance for the entity.
  virtual double tolerance() const {return 1.e-14;}

  /// True if the entity contains the given point to within tolerance.
  virtual int containsPoint(const SPoint3 &pt) const{throw;}

  /// Get the native pointer of the particular representation
  virtual void * getNativePtr() const= 0;

  /// The model owning this entity.
  GModel *model() const {return _model;}
  // The tag of the entity
  int tag () const {return _tag;}
  // The bounding box
  SBoundingBox3d bounds() const{throw;}

  std::vector<MVertex*> mesh_vertices;

  struct 
  {
    char Visible, Frozen;
  } drawAttributes ;

};


#endif


