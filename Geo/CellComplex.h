// Gmsh - Copyright (C) 1997-2009 C. Geuzaine, J.-F. Remacle
//
// See the LICENSE.txt file for license information. Please report all
// bugs and problems to <gmsh@geuz.org>.
//
// Contributed by Matti Pellikka.

#ifndef _CELLCOMPLEX_H_
#define _CELLCOMPLEX_H_

#include <stdio.h>
#include <string>
#include <algorithm>
#include <set>
#include <queue>
#include "GmshConfig.h"
#include "MElement.h"
#include "GModel.h"
#include "GEntity.h"
#include "GRegion.h"
#include "GFace.h"
#include "GVertex.h"

// Abstract class representing an elemtary cell of a cell complex.
class Cell
{  
  protected:
   
   // cell dimension
   int _dim;
   // maximum number of lower dimensional cells on the boundary of this cell
   int _bdMaxSize;
   // maximum number of higher dimensional cells on the coboundary of this cell
   int _cbdMaxSize;

   // whether this cell belongs to a subdomain
   // used in relative homology computation
   bool _inSubdomain;
   
   // whether this cell belongs to the boundary of a cell complex
   bool _onDomainBoundary;
   
   bool _combined; 
   
   // unique tag for each cell
   int _tag;
   int _index; 
   
   // cells on the boundary and on the coboundary of thhis cell
   std::list< std::pair<int, Cell*> > _boundary;
   std::list< std::pair<int, Cell*> > _coboundary;
   
   
  public:
   Cell(){}
   virtual ~Cell(){}
   
   virtual int getDim() const { return _dim; };
   virtual int getTag() const { return _tag; };
   virtual void setTag(int tag) { _tag = tag; };
   virtual int getIndex() const { return _index; };
   virtual void setIndex(int index) { _index = index; };
   
   
   // get the number of vertices this cell has
   virtual int getNumVertices() const = 0; //{return _vertices.size();}
   virtual int getVertex(int vertex) const = 0; //{return _vertices.at(vertex);}
   virtual int getSortedVertex(int vertex) const = 0; 
   virtual std::vector<int> getVertexVector() const = 0;
   
   // returns 1 or -1 if lower dimensional cell tau is on the boundary of this cell
   // otherwise returns 0
   // implementation will vary depending on cell type
   virtual int kappa(Cell* tau) const = 0;
   
   // true if this cell has given vertex
   virtual bool hasVertex(int vertex) const =0;
  
   virtual unsigned int getBdMaxSize() const { return _bdMaxSize; };
   virtual unsigned int getCbdMaxSize() const { return _cbdMaxSize; };

   virtual int getBoundarySize() { return _boundary.size(); }
   virtual int getCoboundarySize() { return _coboundary.size(); }
   
   virtual std::list< std::pair<int, Cell*> > getOrientedBoundary() { return _boundary; }
   virtual std::list< Cell* > getBoundary() {
     std::list<Cell*> boundary;
     for( std::list< std::pair<int, Cell*> >::iterator it= _boundary.begin();it!= _boundary.end();it++){
       Cell* cell = (*it).second;
       boundary.push_back(cell);
     }
     return boundary;
   }
   virtual std::list<std::pair<int, Cell*> >getOrientedCoboundary() { return _coboundary; }
   virtual std::list< Cell* > getCoboundary() {
     std::list<Cell*> coboundary;
     for( std::list< std::pair<int, Cell*> >::iterator it= _coboundary.begin();it!= _coboundary.end();it++){
       Cell* cell = (*it).second;
       coboundary.push_back(cell);
     }
     return coboundary;
   }
   
   virtual void addBoundaryCell(int orientation, Cell* cell) { _boundary.push_back( std::make_pair(orientation, cell)); }
   virtual void addCoboundaryCell(int orientation, Cell* cell) { _coboundary.push_back( std::make_pair(orientation, cell)); }
   
   virtual void removeBoundaryCell(Cell* cell) {
     for(std::list< std::pair<int, Cell*> >::iterator it = _boundary.begin(); it != _boundary.end(); it++){
       Cell* cell2 = (*it).second;
       if(*cell2 == *cell) { _boundary.erase(it); break; }
     }
   }
   virtual void removeCoboundaryCell(Cell* cell) { 
     for(std::list< std::pair<int, Cell*> >::iterator it = _coboundary.begin(); it != _coboundary.end(); it++){
       Cell* cell2 = (*it).second;
       if(*cell2 == *cell) { _coboundary.erase(it); break; }
     }
   }
      
   virtual void printBoundary() {  
     for(std::list< std::pair<int, Cell*> >::iterator it = _boundary.begin(); it != _boundary.end(); it++){
       printf("Boundary cell orientation: %d ", (*it).first);
       Cell* cell2 = (*it).second;
       cell2->printCell();
     }
   }
   
   
   virtual bool inSubdomain() const { return _inSubdomain; }
   virtual void setInSubdomain(bool subdomain)  { _inSubdomain = subdomain; }
   
   virtual bool onDomainBoundary() const { return _onDomainBoundary; }
   virtual void setOnDomainBoundary(bool domainboundary)  { _onDomainBoundary = domainboundary; }
   
   // print cell vertices
   virtual void printCell() const = 0;
   
   // return the coordinates of this cell (only used on zero-cells if anywhere)
   virtual inline double x() const { return 0; }
   virtual inline double y() const { return 0; }
   virtual inline double z() const { return 0; }
   
   virtual int getNumFacets() const { return 0; }
   virtual void getFacetVertices(const int num, std::vector<int> &v) const {};
   
   virtual bool combined() { return _combined; }
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
   
   
};


// Simplicial cell type.
class Simplex : public Cell
{ 
   
 public:
   Simplex() : Cell() {
     _combined = false;
   }
   ~Simplex(){}  
  
   // kappa for simplices
   // checks whether lower dimensional simplex tau is on the boundary of this cell
   virtual int kappa(Cell* tau) const; 
   
};

// Zero simplex cell type.
class ZeroSimplex : public Simplex
{
 private:
   // number of the vertex
   // same as the corresponding vertex in the finite element mesh
   int _v;
   // coordinates of this zero simplex, if needed
   double _x, _y, _z;
 public:
   
   ZeroSimplex(int vertex, int tag=0, bool subdomain=false, bool boundary=false, double x=0, double y=0, double z=0) : Simplex(){
     _v = vertex;
     _tag = tag;
     _dim = 0;
     _bdMaxSize = 0;
     _cbdMaxSize = 1000; // big number
     _x = x;
     _y = y;
     _z = z;
     _inSubdomain = subdomain;
     _onDomainBoundary = boundary;
   }
   ~ZeroSimplex(){}
   
   int getDim() const { return 0; }
   int getNumVertices() const { return 1; }
   int getVertex(int vertex) const {return _v; }
   int getSortedVertex(int vertex) const {return _v; }
   bool hasVertex(int vertex) const {return (_v == vertex); }
   
   std::vector<int> getVertexVector() const { return std::vector<int>(1,_v); }
   std::vector<int> getSortedVertexVector() const { return std::vector<int>(1,_v); }
   
   inline double x() const { return _x; }
   inline double y() const { return _y; }
   inline double z() const { return _z; }
   
   void printCell() const { printf("Vertices: %d, in subdomain: %d \n", _v, _inSubdomain); }
   
};

// One simplex cell type.
class OneSimplex : public Simplex
{
  private:
   // numbers of the vertices of this simplex
   // same as the corresponding vertices in the finite element mesh
   int _v[2];
   int _vs[2];
  public:
   
   OneSimplex(std::vector<int> vertices, int tag=0, bool subdomain=false, bool boundary=false) : Simplex(){
     _v[0] = vertices.at(0);
     _v[1] = vertices.at(1);
     sort(vertices.begin(), vertices.end());
     _vs[0] = vertices.at(0);
     _vs[1] = vertices.at(1);
     _tag = tag;
     _dim = 1;
     _bdMaxSize = 2;
     _cbdMaxSize = 1000;
     _inSubdomain = subdomain;
     _onDomainBoundary = boundary;
   }
     
   
   
   // constructor for the dummy one simplex
   // used to find another definite one simplex from the cell complex
   // first vertex gives the lower bound from where to look
   OneSimplex(int vertex, int dummy){
     _vs[0] = vertex;
     _vs[1] = dummy;
   }
   
   ~OneSimplex(){}
   
   int getDim() const { return 1; }
   int getNumVertices() const { return 2; }
   int getNumFacets() const {  return 2; }
   int getVertex(int vertex) const {return _v[vertex]; }
   int getSortedVertex(int vertex) const {return _vs[vertex]; }
   bool hasVertex(int vertex) const {return (_v[0] == vertex || _v[1] == vertex); }
   
   std::vector<int> getVertexVector() const { 
     return std::vector<int>(_v, _v + sizeof(_v)/sizeof(int) ); }
   std::vector<int> getSortedVertexVector() const {
     return std::vector<int>(_vs, _vs + sizeof(_vs)/sizeof(int) ); 
   }
   
   void getFacetVertices(const int num, std::vector<int> &v) const {
     v.resize(1);
     v[0] = _v[num];
   }
   
   //int kappa(Cell* tau) const;
   
   void printCell() const { printf("Vertices: %d %d, in subdomain: %d \n", _v[0], _v[1], _inSubdomain); }
};

// Two simplex cell type.
class TwoSimplex : public Simplex
{
  private:
   // numbers of the vertices of this simplex
   // same as the corresponding vertices in the finite element mesh
   int _v[3];
   int _vs[3];
   
   int edges_tri(const int edge, const int vert) const{
     static const int e[3][2] = {
       {0, 1},
       {1, 2},
       {2, 0}
     };
     return e[edge][vert];
   }
   
  public:
   
   TwoSimplex(std::vector<int> vertices, int tag=0, bool subdomain=false, bool boundary=false) : Simplex(){
     _v[0] = vertices.at(0);
     _v[1] = vertices.at(1);
     _v[2] = vertices.at(2);
     sort(vertices.begin(), vertices.end());
     _vs[0] = vertices.at(0);
     _vs[1] = vertices.at(1);
     _vs[2] = vertices.at(2);
     _tag = tag;
     _dim = 2;
     _bdMaxSize = 3;
     _cbdMaxSize = 2;
     _inSubdomain = subdomain;
     _onDomainBoundary = boundary;
   }
   // constructor for the dummy one simplex
   TwoSimplex(int vertex, int dummy){
     _v[0] = vertex;
     _v[1] = dummy;
     _v[2] = dummy;
   }
   
   ~TwoSimplex(){}
   
   int getDim() const { return 2; }
   int getNumVertices() const { return 3; }
   int getNumFacets() const { return 3; }
   int getVertex(int vertex) const {return _v[vertex]; }
   int getSortedVertex(int vertex) const {return _vs[vertex]; }
   bool hasVertex(int vertex) const {return 
       (_v[0] == vertex || _v[1] == vertex || _v[2] == vertex); }
   std::vector<int> getVertexVector() const { 
     return std::vector<int>(_v, _v + sizeof(_v)/sizeof(int) ); }
   std::vector<int> getSortedVertexVector() const {
     return std::vector<int>(_vs, _vs + sizeof(_vs)/sizeof(int) ); 
   }
   
   void getFacetVertices(const int num, std::vector<int> &v) const {
     v.resize(2);
     v[0] = _v[edges_tri(num, 0)];
     v[1] = _v[edges_tri(num, 1)];
   }
   
   void printCell() const { printf("Vertices: %d %d %d, in subdomain: %d\n", _v[0], _v[1], _v[2], _inSubdomain); }
};

// Three simplex cell type.
class ThreeSimplex : public Simplex
{
  private:
   // numbers of the vertices of this simplex
   // same as the corresponding vertices in the finite element mesh
   int _v[4];
   int _vs[4];
   
   int faces_tetra(const int face, const int vert) const{
     static const int f[4][3] = {
       {0, 2, 1},
       {0, 1, 3},
       {0, 3, 2},
       {3, 1, 2}
     };
     return f[face][vert];
   }
   
  public:
   
   ThreeSimplex(std::vector<int> vertices, int tag=0, bool subdomain=false, bool boundary=false) : Simplex(){
     _v[0] = vertices.at(0);
     _v[1] = vertices.at(1);
     _v[2] = vertices.at(2);
     _v[3] = vertices.at(3);
     sort(vertices.begin(), vertices.end());
     _vs[0] = vertices.at(0);
     _vs[1] = vertices.at(1);
     _vs[2] = vertices.at(2);
     _vs[3] = vertices.at(3);
     _tag = tag;
     _dim = 3;
     _bdMaxSize = 4;
     _cbdMaxSize = 0;
     _inSubdomain = subdomain;
     _onDomainBoundary = boundary;
   }
   // constructor for the dummy one simplex
   ThreeSimplex(int vertex, int dummy){
     _v[0] = vertex;
     _v[1] = dummy;
     _v[2] = dummy;
     _v[3] = dummy;
   }
   
   ~ThreeSimplex(){}
   
   int getDim() const { return 3; }
   int getNumVertices() const { return 4; }
   int getNumFacets() const { return 4; }
   int getVertex(int vertex) const {return _v[vertex]; }
   int getSortedVertex(int vertex) const {return _vs[vertex]; }
   bool hasVertex(int vertex) const {return 
       (_v[0] == vertex || _v[1] == vertex || _v[2] == vertex || _v[3] == vertex); }
   std::vector<int> getVertexVector() const { 
     return std::vector<int>(_v, _v + sizeof(_v)/sizeof(int) ); }
   std::vector<int> getSortedVertexVector() const {
     return std::vector<int>(_vs, _vs + sizeof(_vs)/sizeof(int) ); }
   
   void getFacetVertices(const int num, std::vector<int> &v) const {
     v.resize(3);
     v[0] = _v[faces_tetra(num, 0)];
     v[1] = _v[faces_tetra(num, 1)];
     v[2] = _v[faces_tetra(num, 2)];
   }
   
   virtual void printCell() const { printf("Vertices: %d %d %d %d, in subdomain: %d \n", _v[0], _v[1], _v[2], _v[3], _inSubdomain); }
};


class Quadrangle : public Cell
{
 private:
   
   int _v[4];
   int _vs[4];
   
   int edges_quad(const int edge, const int vert) const{
     static const int e[4][2] = {
       {0, 1},
       {1, 2},
       {2, 3},
       {3, 0}
     };
     return e[edge][vert];
   }
   
 public:
   
   Quadrangle(std::vector<int> vertices, int tag=0, bool subdomain=false, bool boundary=false) : Cell(){
     _v[0] = vertices.at(0);
     _v[1] = vertices.at(1);
     _v[2] = vertices.at(2);
     _v[3] = vertices.at(3);
     sort(vertices.begin(), vertices.end());
     _vs[0] = vertices.at(0);
     _vs[1] = vertices.at(1);
     _vs[2] = vertices.at(2);
     _vs[3] = vertices.at(3);
     _tag = tag;
     _dim = 2;
     _bdMaxSize = 4;
     _cbdMaxSize = 2;
     _inSubdomain = subdomain;
     _onDomainBoundary = boundary;
   }
   ~Quadrangle(){}
   
   int getDim() const { return 2; }
   int getNumVertices() const { return 4; }
   int getNumFacets() const { return 4; }
   int getVertex(int vertex) const {return _v[vertex]; }
   int getSortedVertex(int vertex) const { return _vs[vertex]; }
   int kappa(Cell* tau) const;
   
   bool hasVertex(int vertex) const {return
       (_v[0] == vertex || _v[1] == vertex || _v[2] == vertex || _v[3] == vertex); }
   std::vector<int> getVertexVector() const {
     return std::vector<int>(_v, _v + sizeof(_v)/sizeof(int) ); }
   std::vector<int> getSortedVertexVector() const {
     return std::vector<int>(_vs, _vs + sizeof(_vs)/sizeof(int) ); }
   
   void getFacetVertices(const int num, std::vector<int> &v) const {
     v.resize(2);
     v[0] = _v[edges_quad(num, 0)];
     v[1] = _v[edges_quad(num, 1)];
   }
   
};


// Ordering for the cells.
class Less_Cell{
  public:
   bool operator()(const Cell* c1, const Cell* c2) const {
     
     // cells with fever vertices first
     
     if(c1->getNumVertices() != c2->getNumVertices()){
       return (c1->getNumVertices() < c2->getNumVertices());
     }
     
     
     
     // "natural number" -like ordering (the number of a vertice corresponds a digit)
     
     for(int i=0; i < c1->getNumVertices();i++){
       if(c1->getSortedVertex(i) < c2->getSortedVertex(i)) return true;
       else if (c1->getSortedVertex(i) > c2->getSortedVertex(i)) return false;
     }
     
     /*
     std::vector<int> c1v = c1->getVertexVector();
     std::vector<int> c2v = c2->getVertexVector();
     std::sort(c1v.begin(), c1v.end());
     std::sort(c2v.begin(), c2v.end());
     
     for(int i=0; i < c1v.size();i++){
       if(c1v.at(i) < c2v.at(i)) return true;
       else if (c1v.at(i) > c2v.at(i)) return false;
     }
     */
     return false;
   }
};


class Equal_Cell{
  public:
   bool operator ()(const Cell* c1, const Cell* c2){
     if(c1->getNumVertices() != c2->getNumVertices()){
       return false;
     }
     for(int i=0; i < c1->getNumVertices();i++){
       if(c1->getSortedVertex(i) != c2->getSortedVertex(i)){
         return false;
       }
     }
     return true;
   }
};


// Ordering for the finite element mesh vertices.
class Less_MVertex{
  public:
   bool operator()(const MVertex* v1, const MVertex* v2) const {
     return (v1->getNum() < v2->getNum());
   }
};

class CombinedCell : public Cell{
 
  private:
   std::vector<int> _v;
   std::vector<int> _vs;
   std::list< std::pair<int, Cell*> > _cells;
   
  public:
   
   CombinedCell(Cell* c1, Cell* c2, bool orMatch, bool co=false) : Cell(){
     _tag = c1->getTag();
     _dim = c1->getDim();
     _bdMaxSize = 1000000;
     _cbdMaxSize = 1000000;
     _inSubdomain = c1->inSubdomain();
     _onDomainBoundary = c1->onDomainBoundary();
     _combined = true;
     
     _v = c1->getVertexVector();
     for(int i = 0; i < c2->getNumVertices(); i++){
       if(!this->hasVertex(c2->getVertex(i))) _v.push_back(c2->getVertex(i));
     }
     
     _vs = _v;
     std::sort(_vs.begin(), _vs.end());
     
     _cells = c1->getCells();
     std::list< std::pair<int, Cell*> > c2Cells = c2->getCells();
     for(std::list< std::pair<int, Cell*> >::iterator it = c2Cells.begin(); it != c2Cells.end(); it++){
       if(!orMatch) (*it).first = -1*(*it).first;
       _cells.push_back(*it);
     }
     
     _boundary = c1->getOrientedBoundary();
     std::list< std::pair<int, Cell*> > c2Boundary = c2->getOrientedBoundary();
     for(std::list< std::pair<int, Cell*> >::iterator it = c2Boundary.begin(); it != c2Boundary.end(); it++){
       if(!orMatch) (*it).first = -1*(*it).first;
       Cell* cell = (*it).second;
       if(co){
         bool old = false;
         for(std::list< std::pair<int, Cell* > >::iterator it2 = _boundary.begin(); it2 != _boundary.end(); it2++){
           Cell* cell2 = (*it2).second;
           if(*cell2 == *cell) old = true;
         }
         if(!old) _boundary.push_back(*it);
       }
       else _boundary.push_back(*it);
     }
     
     _coboundary = c1->getOrientedCoboundary();
     std::list< std::pair<int, Cell*> > c2Coboundary = c2->getOrientedCoboundary();
     for(std::list< std::pair<int, Cell* > >::iterator it = c2Coboundary.begin(); it != c2Coboundary.end(); it++){
       if(!orMatch) (*it).first = -1*(*it).first;
       Cell* cell = (*it).second;
       if(!co){
         bool old = false;
         for(std::list< std::pair<int, Cell* > >::iterator it2 = _coboundary.begin(); it2 != _coboundary.end(); it2++){
           Cell* cell2 = (*it2).second;
           if(*cell2 == *cell) old = true;
         }
         if(!old) _coboundary.push_back(*it);
       }
       else _coboundary.push_back(*it);
     }
     
   }
  
   ~CombinedCell(){} 
   
   int getNumVertices() const { return _v.size(); } 
   int getVertex(int vertex) const { return _v.at(vertex); }
   int getSortedVertex(int vertex) const { return _vs.at(vertex); }
   std::vector<int> getVertexVector() const { return _v; }
   
   int kappa(Cell* tau) const { return 0; }
   
   // true if this cell has given vertex
   bool hasVertex(int vertex) const {
     for(int i = 0; i < _v.size(); i++){
       if(_v.at(i) == vertex) return true;
     }
     return false;
   }
   
   virtual void printCell() const {
     printf("Vertices: ");
     for(int i = 0; i < this->getNumVertices(); i++){
       printf("%d ", this->getVertex(i));
     }
     printf(", in subdomain: %d\n", _inSubdomain);
   }
   
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
   
   std::set<MVertex*, Less_MVertex> _domainVertices;
   
   // sorted containers of unique cells in this cell complex 
   // one for each dimension
   std::set<Cell*, Less_Cell>  _cells[4];
   
   std::set<Cell*, Less_Cell>  _originalCells[4];
   
  public:
   // iterator for the cells of same dimension
   typedef std::set<Cell*, Less_Cell>::iterator citer;
   
  protected: 
   // enqueue cells in queue if they are not there already
   virtual void enqueueCells(std::list<Cell*>& cells, 
                             std::queue<Cell*>& Q, std::set<Cell*, Less_Cell>& Qset);
   // remove cell from the queue set
   virtual void removeCellQset(Cell*& cell, std::set<Cell*, Less_Cell>& Qset);
      
   // insert cells into this cell complex
   //virtual void insert_cells(bool subdomain, bool boundary);
   virtual void insert_cells(bool subdomain, bool boundary);
   
  public: 
   CellComplex(  std::vector<GEntity*> domain, std::vector<GEntity*> subdomain, std::set<Cell*, Less_Cell> cells ) {
     _domain = domain;
     _subdomain = subdomain;
     for(citer cit = cells.begin(); cit != cells.end(); cit++){
       Cell* cell = *cit;
       _cells[cell->getDim()].insert(cell);
     }
   }
   CellComplex( std::vector<GEntity*> domain, std::vector<GEntity*> subdomain );
   virtual ~CellComplex(){}
   
   
   // get the number of certain dimensional cells
   virtual int getSize(int dim){ return _cells[dim].size(); }
   
   virtual std::set<Cell*, Less_Cell> getCells(int dim){ return _cells[dim]; }
      
   // iterators to the first and last cells of certain dimension
   virtual citer firstCell(int dim) {return _cells[dim].begin(); }
   virtual citer lastCell(int dim) {return _cells[dim].end(); }
  
   // find a cell in this cell complex
   virtual std::set<Cell*, Less_Cell>::iterator findCell(int dim, std::vector<int>& vertices, bool original=false);
   virtual std::set<Cell*, Less_Cell>::iterator findCell(int dim, int vertex, int dummy=0);
   
   // kappa for two cells of this cell complex
   // implementation will vary depending on cell type
   virtual inline int kappa(Cell* sigma, Cell* tau) const { return sigma->kappa(tau); }
   
   // remove a cell from this cell complex
   virtual void removeCell(Cell* cell);
   virtual void replaceCells(Cell* c1, Cell* c2, Cell* newCell, bool orMatch, bool co=false);
   
   
   virtual void insertCell(Cell* cell);
   
   // check whether two cells both belong to subdomain or if neither one does
   virtual bool inSameDomain(Cell* c1, Cell* c2) const { return 
       ( (!c1->inSubdomain() && !c2->inSubdomain()) || (c1->inSubdomain() && c2->inSubdomain()) ); }
   
   // coreduction of this cell complex
   // removes corection pairs of cells of dimension dim and dim+1
   virtual int coreduction(int dim);
   
   // stores removed cells
   
   // reduction of this cell complex
   // removes reduction pairs of cell of dimension dim and dim-1
   virtual int reduction(int dim);
   
   // useful functions for (co)reduction of cell complex
   virtual void reduceComplex();
   // coreduction up to generators of dimension generatorDim
   virtual void coreduceComplex(int generatorDim=3);
   
   // queued coreduction presented in Mrozek's paper
   // slower, but produces cleaner result
   virtual int coreductionMrozek(Cell* generator);
      
   // add every volume, face and edge its missing boundary cells
   virtual void repairComplex(int i=3);
   // change non-subdomain cells to be in subdomain, subdomain cells to not to be in subdomain
   virtual void swapSubdomain();
   
   // print the vertices of cells of certain dimension
   virtual void printComplex(int dim);
   
   // write this cell complex in 2.0 MSH ASCII format
   virtual int writeComplexMSH(const std::string &name); 
   
   virtual int combine(int dim);
   virtual int cocombine(int dim);
   
};

#endif
