// Gmsh - Copyright (C) 1997-2011 C. Geuzaine, J.-F. Remacle
//
// See the LICENSE.txt file for license information. Please report all
// bugs and problems to <gmsh@geuz.org>.
//
// Contributor(s):
//   Amaury Johnen (a.johnen@ulg.ac.be)
//

#ifndef _MESH_GFACE_RECOMBINE_H_
#define _MESH_GFACE_RECOMBINE_H_

#include "GFace.h"
#include "GModel.h"
#include "MEdge.h"
#include "BackgroundMesh.h"

class Rec2DEdge;
class Rec2DVertex;
class Rec2DAction;

typedef std::set<Rec2DAction*/*, lessRec2DAction*/> setofRec2DAction;
typedef std::map<MVertex*, Rec2DVertex*> mapofVertices;
typedef std::map<MEdge, Rec2DEdge*, Less_Edge> mapofEdges;
typedef std::map<MElement*, std::set<Rec2DAction*> > mapofElementActions;
typedef std::map<MQuadrangle*, std::set<MElement*> > mapofAdjacencies;

class Recombine2D {
  private :
    int _numEdge, _numVert, _numChange;
    double _valEdge, _valVert;
    GFace *_gf;
    backgroundMesh *_bgm;
    mapofEdges _edges;
    mapofVertices _vertices;
    setofRec2DAction _actions;
    mapofElementActions _mea;
    mapofAdjacencies _ma;
    std::map<int, std::set<Rec2DVertex*> > _parities;
    std::vector<Rec2DAction*> _obsoleteAction;
    static Recombine2D *_current;
    
  public :
    static bool bo;
    
    Recombine2D(GFace*);
    Recombine2D(GModel*);
    ~Recombine2D();
    
    void apply(bool){return;}
    
    static inline double edgeReward(double adimLength, double alignment)
      {return adimLength * (.5 + .5 * alignment);}
    static inline int getNumChange() {return _current->_numChange;}
    static inline backgroundMesh* bgm() {return _current->_bgm;}
    static inline double getGlobalValue(int numEdge, double valEdge,
                                        int numVert, double valVert );
    static Rec2DEdge* getREdge(MEdge);
    static Rec2DVertex* getRVertex(MVertex*);
    static inline GFace* getGFace() {return _current->_gf;}
    static inline void addObsleteAction(Rec2DAction *a) {
      _current->_obsoleteAction.push_back(a);
    }
    //static inline int* getPtrNumChange() {return &_numChange;}
};

class Rec2DAction {
  private :
    int _position;
    
  protected :
    int _lastUpdate;
    double _globValIfExecuted;
    
  public :
    inline int getVectPosition() {return _position;}
    inline void setVectPosition(int p) {_position = p;}
    //virtual double getReward() = 0;
    virtual bool isObsolete() = 0;
  
  private :
    virtual void _computeGlobVal() = 0;
};

class Rec2DTwoTri2Quad : public Rec2DAction {
  private :
    MTriangle *_triangles[2];
    Rec2DEdge *_edges[5]; // 1 embedded, 4 boundary
    Rec2DVertex *_vertices[4]; // 4 boundary (2 on embedded edge + 2)
    
  public :
    Rec2DTwoTri2Quad(Rec2DEdge*, std::list<MTriangle*>&);
    //double getReward();
    bool isObsolete();
  
  private :
    void _computeGlobVal();
};

class Rec2DFourTri2Quad : public Rec2DAction {
  private :
    MTriangle *_triangles[4];
    Rec2DEdge *_edges[8]; // 4 embedded, 4 boundary
    Rec2DVertex *_vertices[5]; // 4 boundary (2 non adjacent + 2), 1 embedded
    
  public :
    Rec2DFourTri2Quad(Rec2DVertex*, std::list<MTriangle*>&);
    //double getReward();
    bool isObsolete();
  
  private :
    void _computeGlobVal();
};

class Rec2DCollapseTri : public Rec2DAction {
  private :
    MTriangle *_triangles[4]; // either 2 or 4 triangles
    Rec2DEdge *_edge; // 1 embedded or NULL
    Rec2DVertex *_vertices[5]; // 4 boundary (2 on embedded edge + 2), 1 embedded or NULL
    double _globValIfExecuted2;
    
  public :
    Rec2DCollapseTri(Rec2DVertex*, std::list<MTriangle*>&);
    Rec2DCollapseTri(Rec2DEdge*, std::list<MTriangle*>&);
    //double getReward();
    bool isObsolete();
  
  private :
    void _computeGlobVal();
    void _qualCavity(double &valVert, double &valEdge, int &numEdge,
                     std::map<Rec2DVertex*, int> &vertices,
                     Rec2DVertex *imposedVert = NULL                );
    
};

class Rec2DListAction {
  private :
    
    
  public :
    
};

class Rec2DVertex {
  private :
    MVertex *_v;
    std::vector<Rec2DEdge*> _edges;
    int _onWhat, _numEl, _parity, _lastChange;
    // _onWhat={-1:corner,0:edge,1:face,(2:volume)}
    double _angle, _qual;
    SPoint2 _param;
    
    static double **_qualVSnum;
    static double **_gains;
    
  public :
    Rec2DVertex(MVertex*, std::list<MTriangle*>, int onWhat,
                std::map<MVertex*, std::set<GEdge*> >);
    Rec2DVertex(double u, double v);
    
    void add(Rec2DEdge *re) {_edges.push_back(re);}
    void setOnBoundary() {if (_onWhat > 0) _onWhat = -1;
      static bool a=1; if(a){Msg::Error("FIXME boundary");a=0;}}
    bool getIsOnBoundary() {return _onWhat < 1;}
    double getQual(int numEdge = -1);
    inline int getParity() {return _parity;}
    inline void setParity(int p) {_parity = p;}
    inline MVertex* getMVertex() {return _v;}
    void getxyz(double *xyz) {
      xyz[0] = _v->x(); xyz[1] = _v->y(); xyz[2] = _v->z();}
    inline double u() {return _param[0];}
    inline double v() {return _param[1];}
    inline void deleteMVertex() {delete _v;}
    
    double getGain(int);
    void getNeighbours(std::map<Rec2DVertex*, int>&, std::set<Rec2DEdge*>&);
    void getNeighbourEdges(std::set<Rec2DEdge*>&);
    
    static void initStaticTable();
    
  private :
    void _computeQual();
    double _computeAngle(MVertex*, std::list<MTriangle*>&, std::set<GEdge*>&);
};

class Rec2DEdge {
  private :
    Rec2DVertex *_rv1, *_rv2;
    double _qual;
    int _lastChange, _weight;
    
  public :
    Rec2DEdge(MEdge, mapofVertices&, std::list<MTriangle*>&);
    Rec2DEdge(Rec2DVertex*, Rec2DVertex*);
    
    double getQual();
    inline MEdge getMEdge() {return MEdge(_rv1->getMVertex(), _rv2->getMVertex());}
    inline Rec2DVertex* getRVertex(int i) {if (i) return _rv2; return _rv1;}
    int getWeight() {return _weight;}
    double getWeightedQual() {return _weight * _qual;}
    
  private :
    void _computeQual();
    double _straightAdimLength();
    double _straightAlignment();
};

#endif

//idee : lors du parcours de larbre avec un horizon : copier les Rec2dVertex et les Rec2DEdge.
//idee : faire carr�ment : arrete d'un triangle = 0, arrete d'un quad = 1/2
