// Gmsh - Copyright (C) 1997-2010 C. Geuzaine, J.-F. Remacle
//
// See the LICENSE.txt file for license information. Please report all
// bugs and problems to <gmsh@geuz.org>.
//
// Contributed by Matti Pellikka <matti.pellikka@tut.fi>.

#ifndef _CELL_H_
#define _CELL_H_

#include "GmshConfig.h"

#if defined(HAVE_KBIPACK)

#include <stdio.h>
#include <string>
#include <algorithm>
#include <set>
#include <list>
#include <map>
#include <queue>
#include "CellComplex.h"
#include "MElement.h"
//#include "GModel.h"

class Cell;

// Ordering of the cells
class Less_Cell{
  public:
   bool operator()(const Cell* c1, const Cell* c2) const;
};

// Class representing an elementary cell of a cell complex.
class Cell
{  
 protected:
  
  // cell dimension
  int _dim;
   
  // whether this cell belongs to a subdomain
  // used in relative homology computation
  bool _inSubdomain;
  
  // whether this cell a combinded cell of elemetary cells
  bool _combined; 
  
  // mutable index for each cell (used to create boundary operator matrices)
  int _index; 
  
  // for some algorithms to omit this cell
  bool _immune;
  
  // mutable list of cells on the boundary and on the coboundary of this cell
  std::map< Cell*, int, Less_Cell > _boundary;
  std::map< Cell*, int, Less_Cell > _coboundary;
  
  // immutable original boundary and coboundary before the reduction of the
  // cell complex
  std::map<Cell*, int, Less_Cell > _obd;
  std::map<Cell*, int, Less_Cell > _ocbd;
  
  // The mesh element that is the image of this cell
  MElement* _image;
 
  // Whether to delete the mesh element when done
  // (created for homology computation only)
  bool _deleteImage;
  
  // sorted vertices of this cell (used for ordering of the cells)
  std::vector<int> _vs;

  virtual MVertex* getVertex(int vertex) const { 
    return _image->getVertex(vertex); }  
 
 public:

 Cell() : _combined(false), _index(0), _immune(false), _image(NULL), 
    _deleteImage(false), _inSubdomain(false) {}
  Cell(MElement* image);
  
  virtual ~Cell();
  
  virtual MElement* getImageMElement() const { return _image; };
  

  // get the number of vertices this cell has
  virtual int getNumVertices() const { return _image->getNumVertices(); }
  // get the number of facets of this cell
  virtual int getNumFacets() const;
  // get the vertices on a facet of this cell
  virtual void getFacetVertices(const int num, std::vector<MVertex*> &v) const;
  // get boundary cell orientation
  virtual int getFacetOri(std::vector<MVertex*> &v); 
  virtual int getFacetOri(Cell* cell) { 
    std::vector<MVertex*> v; 
    for(int i = 0; i < cell->getNumVertices(); i++) {
      v.push_back(cell->getVertex(i));
    }
    return getFacetOri(v);
  }

  virtual int getDim() const { return _dim; };
  virtual int getIndex() const { return _index; };
  virtual void setIndex(int index) { _index = index; };
  virtual void setImmune(bool immune) { _immune = immune; };
  virtual bool getImmune() const { return _immune; };
  virtual bool inSubdomain() const { return _inSubdomain; }
  virtual void setInSubdomain(bool subdomain)  { _inSubdomain = subdomain; }
  virtual void setDeleteImage(bool deleteImage) { 
    _deleteImage = deleteImage; };
  virtual bool getDeleteImage() const { return _deleteImage; };
  virtual int getSortedVertex(int vertex) const { return _vs.at(vertex); }
  
  // restores the cell information to its original state before reduction
  virtual void restoreCell();
  
  // true if this cell has given vertex
  virtual bool hasVertex(int vertex) const;
  
  // (co)boundary cell iterator
  typedef std::map<Cell*, int, Less_Cell>::iterator biter;
  biter firstBoundary(bool org=false){ 
    return org ? _obd.begin() : _boundary.begin(); }
  biter lastBoundary(bool org=false){ 
    return org ? _obd.end() : _boundary.end(); }
  biter firstCoboundary(bool org=false){ 
    return org ? _ocbd.begin() : _coboundary.begin(); }
  biter lastCoboundary(bool org=false){ 
    return org ? _ocbd.end() : _coboundary.end(); }

  virtual int getBoundarySize() { return _boundary.size(); }
  virtual int getCoboundarySize() { return _coboundary.size(); }
   
  // get the cell boundary
  virtual void getBoundary(std::map<Cell*, int, Less_Cell >& boundary, 
			   bool org=false){
    org ? boundary = _obd : boundary =  _boundary; }
  virtual void getCoboundary(std::map<Cell*, int, Less_Cell >& coboundary,
			     bool org=false){
    org ? coboundary = _ocbd : coboundary = _coboundary; }
  
  // add (co)boundary cell
  virtual bool addBoundaryCell(int orientation, Cell* cell, bool org=false); 
  virtual bool addCoboundaryCell(int orientation, Cell* cell, bool org=false);
  
  // remove (co)boundary cell
  virtual int removeBoundaryCell(Cell* cell, bool other=true);
  virtual int removeCoboundaryCell(Cell* cell, bool other=true);
  
  // true if has given cell on (original) (co)boundary
  virtual bool hasBoundary(Cell* cell, bool org=false);
  virtual bool hasCoboundary(Cell* cell, bool org=false);
  
  virtual void clearBoundary() { _boundary.clear(); }
  virtual void clearCoboundary() { _coboundary.clear(); }
  
  // print cell debug info
  virtual void printCell();
  virtual void printBoundary(bool org=false);
  virtual void printCoboundary(bool org=false);   
  
  // tools for combined cells
  virtual bool isCombined() { return _combined; }
  virtual void getCells( std::list< std::pair<int, Cell*> >& cells) {  
    cells.clear();
    cells.push_back( std::make_pair(1, this)); 
    return;
  }
  virtual int getNumCells() {return 1;}
  
  // equivalence
  bool operator==(const Cell& c2) const {  
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
  /*
  Cell operator=(const Cell& c2) {
    Cell cell;
    cell._ocbd = c2._ocbd;

    return cell;
  }
  Cell(const Cell& c2){ *this = c2; }*/
};

// A cell that is a combination of cells of same dimension
class CombinedCell : public Cell{
  
 private:
  // sorted list of vertices
  std::vector<int> _vs;
  // list of cells this cell is a combination of
  std::list< std::pair<int, Cell*> > _cells;
  
  MVertex* getVertex(int vertex) const {
    printf("ERROR: No mesh vertices for combined cell."); } 
  
 public:
  
  CombinedCell(Cell* c1, Cell* c2, bool orMatch, bool co=false);
  ~CombinedCell() {}
  
  MElement* getImageMElement() const { 
    printf("ERROR: No image mesh element for combined cell."); }
  int getNumFacets() const { return 0; }
  void getFacetVertices(const int num, std::vector<MVertex*> &v) const {}
  int getFacetOri(std::vector<MVertex*> &v) { return 0; }
  int getFacetOri(Cell* cell) { return 0; }

  int getNumVertices() const { return _vs.size(); } 
  int getSortedVertex(int vertex) const { return _vs.at(vertex); }

  void getCells(std::list< std::pair<int, Cell*> >& cells) { cells = _cells; }
  int getNumCells() {return _cells.size();}
  
};

#endif

#endif
