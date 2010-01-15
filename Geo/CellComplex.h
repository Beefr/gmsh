// Gmsh - Copyright (C) 1997-2009 C. Geuzaine, J.-F. Remacle
//
// See the LICENSE.txt file for license information. Please report all
// bugs and problems to <gmsh@geuz.org>.
//
// Contributed by Matti Pellikka <matti.pellikka@tut.fi>.

#ifndef _CELLCOMPLEX_H_
#define _CELLCOMPLEX_H_

#include "GmshConfig.h"

#if defined(HAVE_KBIPACK)

#include <stdio.h>
#include <string>
#include <algorithm>
#include <set>
#include <queue>
#include "MElement.h"
#include "MPoint.h"
#include "MLine.h"
#include "MTriangle.h"
#include "MQuadrangle.h"
#include "MHexahedron.h"
#include "MPrism.h"
#include "MPyramid.h"
#include "MTetrahedron.h"
#include "GModel.h"
#include "GEntity.h"
#include "GRegion.h"
#include "GFace.h"
#include "GVertex.h"

class Cell;

// Ordering of the cells
class Less_Cell{
  public:
   bool operator()(const Cell* c1, const Cell* c2) const;
};

// Abstract class representing an elementary cell of a cell complex.
class Cell
{  
  protected:
   
   // cell dimension
   int _dim;
   
   // whether this cell belongs to a subdomain, immutable
   // used in relative homology computation
   bool _inSubdomain;
   
   // whether this cell belongs to the boundary of a cell complex
   bool _onDomainBoundary;
   
   // whether this cell a combinded cell of elemetary cells
   bool _combined; 
   
   // unused
   int _tag;
   
   // mutable index for each cell
   int _index; 
   
   // for some algorithms to omit this cell
   bool _immune;
   
   
  
   // mutable list of cells on the boundary and on the coboundary of this cell
   std::map< Cell*, int, Less_Cell > _boundary;
   std::map< Cell*, int, Less_Cell > _coboundary;
   int _bdSize;
   int _cbdSize;
   
   // immutable original boundary and coboundary before the reduction of the cell complex
   std::map<Cell*, int, Less_Cell > _obd;
   std::map<Cell*, int, Less_Cell > _ocbd;
   
  public:
   Cell(){
     _bdSize = 0;
     _cbdSize = 0;
     _combined = false;
     _index = 0;
     _immune = false;
   }
   virtual ~Cell(){}
   
   virtual int getDim() const { return _dim; };
   virtual int getTag() const { return _tag; };
   virtual void setTag(int tag) { _tag = tag; };
   virtual int getIndex() const { return _index; };
   virtual void setIndex(int index) { _index = index; };
   virtual int getNum() { return -1; }
   virtual int getType() { return -1; }
   virtual int getTypeForMSH() { return -1; }
   virtual int getPartition() { return -1; }
   virtual void setPartition(int num){}
   virtual void setImmune(bool immune) { _immune = immune; };
   virtual bool getImmune() const { return _immune; };

   // get the number of vertices this cell has
   virtual int getNumVertices() const = 0;
   virtual MVertex* getVertex(int vertex) const = 0; //{return _vertices.at(vertex);}
   virtual int getSortedVertex(int vertex) const = 0; 
   virtual std::vector<MVertex*> getVertexVector() const = 0;
   
   // restores the cell information to its original state before reduction
   virtual void restoreCell();
   
   // true if this cell has given vertex
   virtual bool hasVertex(int vertex) const = 0;

   // (co)boundary cell iterator
   typedef std::map<Cell*, int, Less_Cell>::iterator biter;
   
   virtual int getBoundarySize() { return _boundary.size(); }
   virtual int getCoboundarySize() { return _coboundary.size(); }
   
   // get the cell boundary
   virtual std::map<Cell*, int, Less_Cell > getOrientedBoundary() { return _boundary; }
   virtual std::list< Cell* > getBoundary();
   virtual std::map<Cell*, int, Less_Cell > getOrientedCoboundary() { return _coboundary; }
   virtual std::list< Cell* > getCoboundary();
   
   // add (co)boundary cell
   virtual bool addBoundaryCell(int orientation, Cell* cell); 
   virtual bool addCoboundaryCell(int orientation, Cell* cell);
   
   // remove (co)boundary cell
   virtual int removeBoundaryCell(Cell* cell, bool other=true);
   virtual int removeCoboundaryCell(Cell* cell, bool other=true);
   
   // true if has given cell on (original) (co)boundary
   virtual bool hasBoundary(Cell* cell, bool org=false);
   virtual bool hasCoboundary(Cell* cell, bool org=false);
   
   virtual void clearBoundary() { _boundary.clear(); }
   virtual void clearCoboundary() { _coboundary.clear(); }
   
   // algebraic dual of the cell
   virtual void makeDualCell();
   
   virtual void printBoundary();
   virtual void printCoboundary();
   
   // original (co)boundary
   virtual void addOrgBdCell(int orientation, Cell* cell) { _obd.insert( std::make_pair(cell, orientation ) ); };
   virtual void addOrgCbdCell(int orientation, Cell* cell) { _ocbd.insert( std::make_pair(cell, orientation ) ); };
   virtual std::map<Cell*, int, Less_Cell > getOrgBd() { return _obd; }
   virtual std::map<Cell*, int, Less_Cell > getOrgCbd() { return _ocbd; }
   virtual void printOrgBd();
   virtual void printOrgCbd(); 
   
   virtual bool inSubdomain() const { return _inSubdomain; }
   //virtual void setInSubdomain(bool subdomain)  { _inSubdomain = subdomain; }
   
   virtual bool onDomainBoundary() const { return _onDomainBoundary; }
   virtual void setOnDomainBoundary(bool domainboundary)  { _onDomainBoundary = domainboundary; }
   
   // print cell vertices
   virtual void printCell() const = 0;
   
   virtual int getNumFacets() const { return 0; }
   virtual void getFacetVertices(const int num, std::vector<MVertex*> &v) const {};
   
   // get boundary cell orientation
   virtual int getFacetOri(Cell* cell) { std::vector<MVertex*> v = cell->getVertexVector(); return getFacetOri(v); } 
   virtual int getFacetOri(std::vector<MVertex*> &v) { return 0; }   

   // tools for combined cells
   virtual bool isCombined() { return _combined; }
   virtual std::list< std::pair<int, Cell*> > getCells() {  std::list< std::pair<int, Cell*> >cells; cells.push_back( std::make_pair(1, this)); return cells; }
   virtual int getNumCells() {return 1;}
      
   bool operator==(const Cell& c2){  
     if(this->getNumVertices() != c2.getNumVertices()){
       return false;
     }
     for(int i=0; i < this->getNumVertices();i++){
       if(this->getSortedVertex(i) != c2.getSortedVertex(i)){
         return false;
       }
     }
     return true;
   }
   
   bool operator<(const Cell& c2) const {
     
     if(this->getNumVertices() != c2.getNumVertices()){
       return (this->getNumVertices() < c2.getNumVertices());
     }
     for(int i=0; i < this->getNumVertices();i++){
       if(this->getSortedVertex(i) < c2.getSortedVertex(i)) return true;
       else if (this->getSortedVertex(i) > c2.getSortedVertex(i)) return false;
     }
     return false;
   }
   
   //virtual void writeMSH(FILE *fp, double version=1.0, bool binary=false,  int num=0, int elementary=1, int physical=1, int parentNum=0) {}
};


// Simplicial cell type.
class Simplex : public Cell
{    
 public:
   Simplex() : Cell() {}
   ~Simplex(){}    
};

// Zero simplex cell type.
class ZeroSimplex : public Simplex, public MPoint
{
 private:

 public:
   
   ZeroSimplex(std::vector<MVertex*> v, int tag=0, bool subdomain=false, bool boundary=false, int num=0, int part=0)
     : MPoint(v, num, part){
       _tag = tag;
       _inSubdomain = subdomain;
       _onDomainBoundary = boundary;
     }
   ~ZeroSimplex(){}
   
   int getDim() const { return 0; }
   int getNum() { return MPoint::getNum(); }
   int getType() { return MPoint::getType(); }
   int getTypeForMSH() { return MPoint::getTypeForMSH(); }
   int getPartition() { return MPoint::getPartition(); }
   int getNumVertices() const { return 1; }
   MVertex* getVertex(int vertex) const {return _v[0]; }
   int getSortedVertex(int vertex) const {return _v[0]->getNum(); }
   bool hasVertex(int vertex) const {return (_v[0]->getNum() == vertex); }
   
   std::vector<MVertex*> getVertexVector() const { std::vector<MVertex*> v; v.push_back(_v[0]); return v; }
   std::vector<int> getSortedVertexVector() const { return std::vector<int>(1,_v[0]->getNum()); }
   
   void printCell() const { printf("Cell dimension: %d, Vertices: %d, in subdomain: %d \n", getDim(), _v[0]->getNum(), _inSubdomain); }
   
};

// One simplex cell type.
class OneSimplex : public Simplex, public MLine
{
  private:
   // sorted list of vertices
   int _vs[2];
  public: 
   OneSimplex(std::vector<MVertex*> v, int tag=0, bool subdomain=false, bool boundary=false, int num=0, int part=0)
     : MLine(v, num, part){
       _tag = tag;
       _inSubdomain = subdomain;
       _onDomainBoundary = boundary;
       _vs[0] = v.at(0)->getNum();
       _vs[1] = v.at(1)->getNum();
       std::sort(_vs, _vs+2);
     }
   
   ~OneSimplex(){}
   
   int getDim() const { return 1; }
   int getNum() { return MLine::getNum(); }
   int getType() { return MLine::getType(); }
   int getTypeForMSH() { return MLine::getTypeForMSH(); }
   int getPartition() { return MLine::getPartition(); }
   int getNumVertices() const { return 2; }
   int getNumFacets() const {  return 2; }
   MVertex* getVertex(int vertex) const {return _v[vertex]; }
   int getSortedVertex(int vertex) const {return _vs[vertex]; }
   bool hasVertex(int vertex) const {return (_v[0]->getNum() == vertex || _v[1]->getNum() == vertex); }
   
   std::vector<MVertex*> getVertexVector() const { 
     return std::vector<MVertex*>(_v, _v + sizeof(_v)/sizeof(MVertex*) ); }
   std::vector<int> getSortedVertexVector() const {
     return std::vector<int>(_vs, _vs + sizeof(_vs)/sizeof(int) ); 
   }
   
   void getFacetVertices(const int num, std::vector<MVertex*> &v) const {
     v.resize(1);
     v[0] = _v[num];
   }
   int getFacetOri(std::vector<MVertex*> &v){
     if(v.size() != 1) return 0;
     else if(v[0] == _v[0]) return -1;
     else if(v[0] == _v[1]) return 1;
     else return 0;
   }
   
   void printCell() const { printf("Cell dimension: %d, Vertices: %d %d, in subdomain: %d \n", getDim(), _v[0]->getNum(), _v[1]->getNum(), _inSubdomain); }
   
};

// Two simplex cell type.
class TwoSimplex : public Simplex, public MTriangle
{
  private:
   // sorted list of vertices
   int _vs[3];
   
  public:

   TwoSimplex(std::vector<MVertex*> v, int tag=0, bool subdomain=false, bool boundary=false, int num=0, int part=0)
     : MTriangle(v, num, part){
       _tag = tag;
       _inSubdomain = subdomain;
       _onDomainBoundary = boundary;
       _vs[0] = v.at(0)->getNum();
       _vs[1] = v.at(1)->getNum();
       _vs[2] = v.at(2)->getNum();
       std::sort(_vs, _vs+3);
     }
   
   ~TwoSimplex(){}
   
   int getDim() const { return 2; }
   int getNum() { return MTriangle::getNum(); }
   int getType() { return MTriangle::getType(); }
   int getTypeForMSH() { return MTriangle::getTypeForMSH(); }
   int getPartition() { return MTriangle::getPartition(); }
   int getNumVertices() const { return 3; }
   int getNumFacets() const { return 3; }
   MVertex* getVertex(int vertex) const {return _v[vertex]; }
   int getSortedVertex(int vertex) const {return _vs[vertex]; }
   bool hasVertex(int vertex) const {return 
       (_v[0]->getNum() == vertex || _v[1]->getNum() == vertex || _v[2]->getNum() == vertex); }
   
   std::vector<MVertex*> getVertexVector() const { 
     return std::vector<MVertex*>(_v, _v + sizeof(_v)/sizeof(MVertex*) ); }
   std::vector<int> getSortedVertexVector() const {
     return std::vector<int>(_vs, _vs + sizeof(_vs)/sizeof(int) ); 
   }
   
   void getFacetVertices(const int num, std::vector<MVertex*> &v) const {
     MTriangle::getEdgeVertices(num, v);
   }
   int getFacetOri(std::vector<MVertex*> &v){
     if(v.size() != 2) return 0;
     MEdge facet = MEdge(v[0], v[1]);
     int ithFacet = 0;
     int sign = 0;
     MTriangle::getEdgeInfo(facet, ithFacet, sign);
     return sign;
   }    

   void printCell() const { printf("Cell dimension: %d, Vertices: %d %d %d, in subdomain: %d\n", getDim(), _v[0]->getNum(), _v[1]->getNum(), _v[2]->getNum(), _inSubdomain); }
};

// Quadrangle cell type.
class CQuadrangle : public Cell, public MQuadrangle
{
  private:
   // sorted list of vertices
   int _vs[4];
   
  public:

   CQuadrangle(std::vector<MVertex*> v, int tag=0, bool subdomain=false, bool boundary=false, int num=0, int part=0)
     : MQuadrangle(v, num, part){
       _tag = tag;
       _inSubdomain = subdomain;
       _onDomainBoundary = boundary;
       _vs[0] = v.at(0)->getNum();
       _vs[1] = v.at(1)->getNum();
       _vs[2] = v.at(2)->getNum();
       _vs[3] = v.at(3)->getNum();
       std::sort(_vs, _vs+3);
     }
   
   ~CQuadrangle(){}
   
   int getDim() const { return 2; }
   int getNum() { return MQuadrangle::getNum(); }
   int getType() { return MQuadrangle::getType(); }
   int getTypeForMSH() { return MQuadrangle::getTypeForMSH(); }
   int getPartition() { return MQuadrangle::getPartition(); }
   int getNumVertices() const { return 4; }
   int getNumFacets() const { return 4; }
   MVertex* getVertex(int vertex) const {return _v[vertex]; }
   int getSortedVertex(int vertex) const {return _vs[vertex]; }
   bool hasVertex(int vertex) const {return 
       (_v[0]->getNum() == vertex || _v[1]->getNum() == vertex || _v[2]->getNum() == vertex || _v[3]->getNum() == vertex); } 
   
   std::vector<MVertex*> getVertexVector() const { 
     return std::vector<MVertex*>(_v, _v + sizeof(_v)/sizeof(MVertex*) ); }
   std::vector<int> getSortedVertexVector() const {
     return std::vector<int>(_vs, _vs + sizeof(_vs)/sizeof(int) ); 
   }
   
   void getFacetVertices(const int num, std::vector<MVertex*> &v) const {
     MQuadrangle::getEdgeVertices(num, v);
   }
   int getFacetOri(std::vector<MVertex*> &v){
     if(v.size() != 2) return 0;
     MEdge facet = MEdge(v[0], v[1]);
     int ithFacet = 0;
     int sign = 0;
     MQuadrangle::getEdgeInfo(facet, ithFacet, sign);
     return sign;
   }  
   
   void printCell() const { printf("Cell dimension: %d, Vertices: %d %d %d %d, in subdomain: %d\n", 
                                    getDim(), _v[0]->getNum(), _v[1]->getNum(), _v[2]->getNum(), 
                                    _v[3]->getNum(), _inSubdomain); }
};


// ThreeSimplex cell type.
class ThreeSimplex : public Simplex, public MTetrahedron
{
  private:
   // sorted list of vertices
   int _vs[4];
      
  public:
   ThreeSimplex(std::vector<MVertex*> v, int tag=0, bool subdomain=false, bool boundary=false, int num=0, int part=0)
     : MTetrahedron(v, num, part){
       _tag = tag;
       _inSubdomain = subdomain;
       _onDomainBoundary = boundary;
       _vs[0] = v.at(0)->getNum();
       _vs[1] = v.at(1)->getNum();
       _vs[2] = v.at(2)->getNum();
       _vs[3] = v.at(3)->getNum();
       std::sort(_vs, _vs+4);
     }
   
   ~ThreeSimplex(){}
   
   int getDim() const { return 3; }
   int getNum() { return MTetrahedron::getNum(); }
   int getType() { return MTetrahedron::getType(); }
   int getTypeForMSH() { return MTetrahedron::getTypeForMSH(); }
   int getPartition() { return MTetrahedron::getPartition(); }
   int getNumVertices() const { return 4; }
   int getNumFacets() const { return 4; }
   MVertex* getVertex(int vertex) const {return _v[vertex]; }
   int getSortedVertex(int vertex) const {return _vs[vertex]; }
   bool hasVertex(int vertex) const {return 
       (_v[0]->getNum() == vertex || _v[1]->getNum() == vertex || _v[2]->getNum() == vertex || _v[3]->getNum() == vertex); }
   std::vector<MVertex*> getVertexVector() const { 
     return std::vector<MVertex*>(_v, _v + sizeof(_v)/sizeof(MVertex*) ); }
   std::vector<int> getSortedVertexVector() const {
     return std::vector<int>(_vs, _vs + sizeof(_vs)/sizeof(int) ); }
   
   void getFacetVertices(const int num, std::vector<MVertex*> &v) const {
     MTetrahedron::getFaceVertices(num, v);
   }
   int getFacetOri(std::vector<MVertex*> &v){
     if(v.size() != 3) return 0;
     MFace facet = MFace(v);
     int ithFacet = 0;
     int sign = 0;
     int rot = 0;
     MTetrahedron::getFaceInfo(facet, ithFacet, sign, rot);
     return sign;
   }

   virtual void printCell() const { printf("Cell dimension: %d, Vertices: %d %d %d %d, in subdomain: %d \n", getDim(), _v[0]->getNum(), _v[1]->getNum(), _v[2]->getNum(), _v[3]->getNum(), _inSubdomain); }
};

// Hexahedron cell type.
class CHexahedron : public Cell, public MHexahedron
{
  private:
   // sorted list of vertices
   int _vs[8];
      
  public:
   CHexahedron(std::vector<MVertex*> v, int tag=0, bool subdomain=false, bool boundary=false, int num=0, int part=0)
     : MHexahedron(v, num, part){
       _tag = tag;
       _inSubdomain = subdomain;
       _onDomainBoundary = boundary;
       _vs[0] = v.at(0)->getNum();
       _vs[1] = v.at(1)->getNum();
       _vs[2] = v.at(2)->getNum();
       _vs[3] = v.at(3)->getNum();
       _vs[4] = v.at(4)->getNum();
       _vs[5] = v.at(5)->getNum();
       _vs[6] = v.at(6)->getNum();
       _vs[7] = v.at(7)->getNum();
       std::sort(_vs, _vs+8);
     }
   
   ~CHexahedron(){}
   
   int getDim() const { return 3; }
   int getNum() { return MHexahedron::getNum(); }
   int getType() { return MHexahedron::getType(); }
   int getTypeForMSH() { return MHexahedron::getTypeForMSH(); }
   int getPartition() { return MHexahedron::getPartition(); }
   int getNumVertices() const { return 8; }
   int getNumFacets() const { return 6; }
   MVertex* getVertex(int vertex) const {return _v[vertex]; }
   int getSortedVertex(int vertex) const {return _vs[vertex]; }
   bool hasVertex(int vertex) const {return 
       (_v[0]->getNum() == vertex || _v[1]->getNum() == vertex || _v[2]->getNum() == vertex || _v[3]->getNum() == vertex ||
        _v[4]->getNum() == vertex || _v[5]->getNum() == vertex || _v[6]->getNum() == vertex || _v[7]->getNum() == vertex ); }
   std::vector<MVertex*> getVertexVector() const { 
     return std::vector<MVertex*>(_v, _v + sizeof(_v)/sizeof(MVertex*) ); }
   std::vector<int> getSortedVertexVector() const {
     return std::vector<int>(_vs, _vs + sizeof(_vs)/sizeof(int) ); }
   
   void getFacetVertices(const int num, std::vector<MVertex*> &v) const {
     MHexahedron::getFaceVertices(num, v);
   }
   int getFacetOri(std::vector<MVertex*> &v){
     if(v.size() != 4) return 0;
     MFace facet = MFace(v);
     int ithFacet = 0;
     int sign = 0;
     int rot = 0;
     MHexahedron::getFaceInfo(facet, ithFacet, sign, rot);
     return sign;
   }
   
   virtual void printCell() const { printf("Cell dimension: %d, Vertices: %d %d %d %d %d %d %d %d, in subdomain: %d \n", getDim(), _v[0]->getNum(), _v[1]->getNum(), _v[2]->getNum(), _v[3]->getNum(), _v[4]->getNum(), _v[5]->getNum(), _v[6]->getNum(), _v[7]->getNum(), _inSubdomain); }
};


// Prism cell type.
class CPrism : public Cell, public MPrism
{
  private:
   // sorted list of vertices
   int _vs[6];
      
  public:
   CPrism(std::vector<MVertex*> v, int tag=0, bool subdomain=false, bool boundary=false, int num=0, int part=0)
     : MPrism(v, num, part){
       _tag = tag;
       _inSubdomain = subdomain;
       _onDomainBoundary = boundary;
       _vs[0] = v.at(0)->getNum();
       _vs[1] = v.at(1)->getNum();
       _vs[2] = v.at(2)->getNum();
       _vs[3] = v.at(3)->getNum();
       _vs[4] = v.at(4)->getNum();
       _vs[5] = v.at(5)->getNum();
       std::sort(_vs, _vs+6);
     }
   
   ~CPrism(){}
   
   int getDim() const { return 3; }
   int getNum() { return MPrism::getNum(); }
   int getType() { return MPrism::getType(); }
   int getTypeForMSH() { return MPrism::getTypeForMSH(); }
   int getPartition() { return MPrism::getPartition(); }
   int getNumVertices() const { return 6; }
   int getNumFacets() const { return 5; }
   MVertex* getVertex(int vertex) const {return _v[vertex]; }
   int getSortedVertex(int vertex) const {return _vs[vertex]; }
   bool hasVertex(int vertex) const {return 
       (_v[0]->getNum() == vertex || _v[1]->getNum() == vertex || _v[2]->getNum() == vertex || _v[3]->getNum() == vertex ||
       _v[4]->getNum() == vertex || _v[5]->getNum() == vertex); }
   std::vector<MVertex*> getVertexVector() const { 
     return std::vector<MVertex*>(_v, _v + sizeof(_v)/sizeof(MVertex*) ); }
   std::vector<int> getSortedVertexVector() const {
     return std::vector<int>(_vs, _vs + sizeof(_vs)/sizeof(int) ); }
   
   void getFacetVertices(const int num, std::vector<MVertex*> &v) const {
     MPrism::getFaceVertices(num, v);
   }
   int getFacetOri(std::vector<MVertex*> &v){
     if(v.size() != 4 && v.size() != 3) return 0;
     MFace facet = MFace(v);
     int ithFacet = 0;
     int sign = 0;
     int rot = 0;
     MPrism::getFaceInfo(facet, ithFacet, sign, rot);
     return sign;
   }
   
   virtual void printCell() const { printf("Cell dimension: %d, Vertices: %d %d %d %d %d %d, in subdomain: %d \n", getDim(), _v[0]->getNum(), _v[1]->getNum(), _v[2]->getNum(), _v[3]->getNum(), _v[4]->getNum(), _v[5]->getNum(),  _inSubdomain); }
};


// Pyramid cell type.
class CPyramid : public Cell, public MPyramid
{
  private:
   // sorted list of vertices
   int _vs[5];
      
  public:
   CPyramid(std::vector<MVertex*> v, int tag=0, bool subdomain=false, bool boundary=false, int num=0, int part=0)
     : MPyramid(v, num, part){
       _tag = tag;
       _inSubdomain = subdomain;
       _onDomainBoundary = boundary;
       _vs[0] = v.at(0)->getNum();
       _vs[1] = v.at(1)->getNum();
       _vs[2] = v.at(2)->getNum();
       _vs[3] = v.at(3)->getNum();
       _vs[4] = v.at(4)->getNum();
       std::sort(_vs, _vs+5);
     }
   
   ~CPyramid(){}
   
   int getDim() const { return 3; }
   int getNum() { return MPyramid::getNum(); }
   int getType() { return MPyramid::getType(); }
   int getTypeForMSH() { return MPyramid::getTypeForMSH(); }
   int getPartition() { return MPyramid::getPartition(); }
   int getNumVertices() const { return 5; }
   int getNumFacets() const { return 5; }
   MVertex* getVertex(int vertex) const {return _v[vertex]; }
   int getSortedVertex(int vertex) const {return _vs[vertex]; }
   bool hasVertex(int vertex) const {return 
       (_v[0]->getNum() == vertex || _v[1]->getNum() == vertex || _v[2]->getNum() == vertex || _v[3]->getNum() == vertex ||
        _v[4]->getNum() == vertex); }
   std::vector<MVertex*> getVertexVector() const { 
     return std::vector<MVertex*>(_v, _v + sizeof(_v)/sizeof(MVertex*) ); }
   std::vector<int> getSortedVertexVector() const {
     return std::vector<int>(_vs, _vs + sizeof(_vs)/sizeof(int) ); }
   
   void getFacetVertices(const int num, std::vector<MVertex*> &v) const {
     MPyramid::getFaceVertices(num, v);
   }
   int getFacetOri(std::vector<MVertex*> &v){
     if(v.size() != 4 && v.size() != 3) return 0;
     MFace facet = MFace(v);
     int ithFacet = 0;
     int sign = 0;
     int rot = 0;
     MPyramid::getFaceInfo(facet, ithFacet, sign, rot);
     return sign;
   }
   
   virtual void printCell() const { printf("Cell dimension: %d, Vertices: %d %d %d %d %d, in subdomain: %d \n", getDim(), _v[0]->getNum(), _v[1]->getNum(), _v[2]->getNum(), _v[3]->getNum(), _v[4]->getNum(), _inSubdomain); }
};

// Ordering for the finite element mesh vertices.
class Less_MVertex{
  public:
   bool operator()(const MVertex* v1, const MVertex* v2) const {
     return (v1->getNum() < v2->getNum());
   }
};

// A cell that is a combination of cells of same dimension
class CombinedCell : public Cell{
 
  private:
   // vertices
   std::vector<MVertex*> _v;
   // sorted list of all vertices
   std::vector<int> _vs;
   // list of cells this cell is a combination of
   std::list< std::pair<int, Cell*> > _cells;
   int _num;
   
  public:
   
   CombinedCell(Cell* c1, Cell* c2, bool orMatch, bool co=false);
   ~CombinedCell();
   
   int getNum() { return _num; }
   int getNumVertices() const { return _v.size(); } 
   MVertex* getVertex(int vertex) const { return _v.at(vertex); }
   int getSortedVertex(int vertex) const { return _vs.at(vertex); }
   std::vector<MVertex*> getVertexVector() const { return _v; }
   
   // true if this cell has given vertex
   bool hasVertex(int vertex) const;
   
   virtual void printCell() const;
   
   std::list< std::pair<int, Cell*> > getCells() { return _cells; }
   int getNumCells() {return _cells.size();}
   
};

// A class representing a cell complex made out of a finite element mesh.
class CellComplex
{
 private:
   
   // the domain in the model which this cell complex covers
   std::vector<GEntity*> _domain;
   // a subdomain of the given domain
   // used in relative homology computation, may be empty
   std::vector<GEntity*> _subdomain;
   
   std::vector<GEntity*> _boundary;
   
   // mesh vertices in this domain
   std::set<MVertex*, Less_MVertex> _domainVertices;
   
   // sorted containers of unique cells in this cell complex 
   // one for each dimension
   std::set<Cell*, Less_Cell>  _cells[4];
   
   // storage for cell pointers to delete them
   std::list<Cell*> _trash;  
   
   std::vector< std::set<Cell*, Less_Cell> > _store;
   std::set<Cell*, Less_Cell> _ecells;
   
   // original cells of this cell complex
   std::set<Cell*, Less_Cell>  _ocells[4];
   
   // Betti numbers of this cell complex (ranks of homology groups)
   int _betti[4];
   
   int _dim;
   
   // Is the cell complex simplicial
   bool _simplicial;
   
   // enqueue cells in queue if they are not there already
   void enqueueCells(std::list<Cell*>& cells, 
                             std::queue<Cell*>& Q, std::set<Cell*, Less_Cell>& Qset);
   // remove cell from the queue set
   void removeCellQset(Cell*& cell, std::set<Cell*, Less_Cell>& Qset);
      
   // for constructor 
   void insert_cells(bool subdomain, bool boundary);
   void find_boundary(std::vector<GEntity*>& domain, std::vector<GEntity*>& subdomain);
   void find_vertices(std::vector<GEntity*>& domain, std::vector<GEntity*>& subdomain);
   
   // remove a cell from this cell complex
   void removeCell(Cell* cell, bool other=true);
   
   // queued coreduction presented in Mrozek's paper
   int coreduction(Cell* generator, int omitted=0);
   
  public: 
   
   CellComplex( std::vector<GEntity*> domain, std::vector<GEntity*> subdomain );
   //CellComplex(CellComplex* cellComplex)
   CellComplex(){}
   ~CellComplex();

   void emptyTrash();
   
   // restore this cell complex to its original state
   void restoreComplex();
     
   // get the number of certain dimensional cells
   int getSize(int dim){ return _cells[dim].size(); }
   int getOrgSize(int dim){ return _ocells[dim].size(); }
   
   int getDim() {return _dim; } 
   
   bool simplicial() { return _simplicial; }
   
   std::set<Cell*, Less_Cell> getCells(int dim){ return _cells[dim]; }
   std::set<Cell*, Less_Cell> getOrgCells(int dim){ return _ocells[dim]; }
   
   // iterator for the cells of same dimension
   typedef std::set<Cell*, Less_Cell>::iterator citer;
   
   // iterators to the first and last cells of certain dimension
   citer firstCell(int dim) {return _cells[dim].begin(); }
   citer lastCell(int dim) {return _cells[dim].end(); }
   citer firstOrgCell(int dim) {return _ocells[dim].begin(); }
   citer lastOrgCell(int dim) {return _ocells[dim].end(); }
   
   
   // true if cell complex has given cell
   bool hasCell(Cell* cell, bool org=false); 
   
   // check whether two cells both belong to subdomain or if neither one does
   bool inSameDomain(Cell* c1, Cell* c2) const { return 
       ( (!c1->inSubdomain() && !c2->inSubdomain()) || (c1->inSubdomain() && c2->inSubdomain()) ); }
     
   void insertCell(Cell* cell);
   
   // reduction of this cell complex
   // removes reduction pairs of cell of dimension dim and dim-1
   int reduction(int dim, int omitted=0);
     
   // useful functions for (co)reduction of cell complex
   int reduceComplex();
   int coreduceComplex();
   
   // remove cells in subdomain from this cell complex
   void removeSubdomain();
      
   // print the vertices of cells of certain dimension
   void printComplex(int dim);
   
   // write this cell complex in 2.0 MSH ASCII format
   // for debugging only
   int writeComplexMSH(const std::string &name); 
   
   // Cell combining for reduction and coreduction
   int combine(int dim);
   int cocombine(int dim);
   
   void computeBettiNumbers();
   int getBettiNumber(int i) { if(i > -1 && i < 4) return _betti[i]; else return 0; }
   
   // check whether all boundary cells of a cell has this cell as coboundary cell and vice versa
   // also check whether all (co)boundary cells of a cell belong to this cell complex
   bool checkCoherence();
   
   int eulerCharacteristic(){ return getSize(0) - getSize(1) + getSize(2) - getSize(3);}
   void printEuler(){ printf("Euler characteristic: %d. \n", eulerCharacteristic()); }
   
   int getNumOmitted() { return _store.size(); };
   std::set<Cell*, Less_Cell> getOmitted(int i) { return _store.at(i); }  
   
   // change roles of boundaries and coboundaries of the cells in this cell complex
   // equivalent to transposing boundary operator matrices
   void makeDualComplex();
};

#endif

#endif
