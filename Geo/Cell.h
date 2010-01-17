// Gmsh - Copyright (C) 1997-2009 C. Geuzaine, J.-F. Remacle
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
#include <queue>
#include "CellComplex.h"
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
   
   // whether this cell belongs to a subdomain, immutable
   // used in relative homology computation
   bool _inSubdomain;
   
   // whether this cell belongs to the boundary of a cell complex
   bool _onDomainBoundary;
   
   // whether this cell a combinded cell of elemetary cells
   bool _combined; 
   
   // mutable index for each cell (used to create boundary operator matrices)
   int _index; 
   
   // for some algorithms to omit this cell
   bool _immune;
  
   // mutable list of cells on the boundary and on the coboundary of this cell
   std::map< Cell*, int, Less_Cell > _boundary;
   std::map< Cell*, int, Less_Cell > _coboundary;
   
   // immutable original boundary and coboundary before the reduction of the cell complex
   std::map<Cell*, int, Less_Cell > _obd;
   std::map<Cell*, int, Less_Cell > _ocbd;
   
   // The mesh element that is the image of this cell
   MElement* _image;
   // Whether to delete the mesh element when done (created for homology computation only)
   bool _deleteImage;
   
   // sorted vertices of this cell (used for ordering of the cells)
   std::vector<int> _vs;
   
  public:
   Cell(){
     _combined = false;
     _index = 0;
     _immune = false;
     _image = NULL;
     _deleteImage = false;
   }
  
   Cell(MElement* image, bool subdomain, bool boundary){
     _combined = false;
     _index = 0;
     _immune = false;
     _image = image;
     _onDomainBoundary = boundary;
     _inSubdomain = subdomain;
     _dim = image->getDim();
     _deleteImage = false;
     for(int i = 0; i < image->getNumVertices(); i++) _vs.push_back(image->getVertex(i)->getNum()); 
     std::sort(_vs.begin(), _vs.end());
   }
   virtual ~Cell() {if(_deleteImage) delete _image; }
  
   virtual MElement* getImageMElement() const { return _image; };
   
   virtual int getDim() const { return _dim; };
   virtual int getIndex() const { return _index; };
   virtual void setIndex(int index) { _index = index; };
   virtual int getNum() { return _image->getNum(); }
   virtual int getType() { return _image->getType(); }
   virtual int getTypeForMSH() { return _image->getTypeForMSH(); }
   virtual int getPartition() { return _image->getPartition(); }
   virtual void setImmune(bool immune) { _immune = immune; };
   virtual bool getImmune() const { return _immune; };
   virtual void setDeleteImage(bool deleteImage) { _deleteImage = deleteImage; };
   virtual bool getDeleteImage() const { return _deleteImage; };

   // get the number of vertices this cell has
   virtual int getNumVertices() const { return _image->getNumVertices(); }
   virtual MVertex* getVertex(int vertex) const { return _image->getVertex(vertex); }
   virtual int getSortedVertex(int vertex) const { return _vs.at(vertex); }
   
   // restores the cell information to its original state before reduction
   virtual void restoreCell();
   
   // true if this cell has given vertex
   virtual bool hasVertex(int vertex) const;

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
   
   // print cell info
   virtual void printCell();
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
   
   // get the number of facets of this cell
   virtual int getNumFacets() const;
   // get the vertices on a facet of this cell
   virtual void getFacetVertices(const int num, std::vector<MVertex*> &v) const;
   
   // get boundary cell orientation
   virtual int getFacetOri(Cell* cell) { 
    std::vector<MVertex*> v; 
    for(int i = 0; i < cell->getNumVertices(); i++) v.push_back(cell->getVertex(i));
    return getFacetOri(v);
   } 
   virtual int getFacetOri(std::vector<MVertex*> &v); 

   // tools for combined cells
   virtual bool isCombined() { return _combined; }
   virtual std::list< std::pair<int, Cell*> > getCells() {  
     std::list< std::pair<int, Cell*> >cells;
     cells.push_back( std::make_pair(1, this)); 
     return cells;
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
   int getType() { return 0; }
   int getTypeForMSH() { return 0; }
   int getPartition() { return 0; }
   
   int getNumVertices() const { return _v.size(); } 
   MVertex* getVertex(int vertex) const { return _v.at(vertex); }
   int getSortedVertex(int vertex) const { return _vs.at(vertex); }
   
   std::list< std::pair<int, Cell*> > getCells() { return _cells; }
   int getNumCells() {return _cells.size();}
   
};


#endif

#endif
