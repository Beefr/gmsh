// Gmsh - Copyright (C) 1997-2009 C. Geuzaine, J.-F. Remacle
//
// See the LICENSE.txt file for license information. Please report all
// bugs and problems to <gmsh@geuz.org>.
//
// Contributed by Matti Pellikka <matti.pellikka@tut.fi>.

#include "CellComplex.h"

#if defined(HAVE_KBIPACK)

bool Less_Cell::operator()(const Cell* c1, const Cell* c2) const {
  
  //cells with fever vertices first
  
  if(c1->getNumVertices() != c2->getNumVertices()){
    return (c1->getNumVertices() < c2->getNumVertices());
  }
  
  //"natural number" -like ordering (the number of a vertice corresponds a digit)
  
  for(int i=0; i < c1->getNumVertices();i++){
    if(c1->getSortedVertex(i) < c2->getSortedVertex(i)) return true;
    else if (c1->getSortedVertex(i) > c2->getSortedVertex(i)) return false;
  }
  
  return false;
}


void Cell::restoreCell(){
  _boundary = _obd;
  _coboundary = _ocbd;
  _bdSize = _boundary.size();
  _cbdSize = _coboundary.size();
  _combined = false;
  _index = 0;
  _immune = false;   
}

std::list< Cell* > Cell::getBoundary() {
  std::list<Cell*> boundary;
  for( biter it= _boundary.begin(); it != _boundary.end();it++){
    Cell* cell = (*it).first;
    boundary.push_back(cell);
  }
  return boundary;
}

std::list< Cell* > Cell::getCoboundary() {
  std::list<Cell*> coboundary;
  for( biter it= _coboundary.begin();it!= _coboundary.end();it++){
    Cell* cell = (*it).first;
    coboundary.push_back(cell);
  }
  return coboundary;
}


bool Cell::addBoundaryCell(int orientation, Cell* cell) {
  _bdSize++;
  biter it = _boundary.find(cell);
  if(it != _boundary.end()){
    (*it).second = (*it).second + orientation;
    if((*it).second == 0) {
      _boundary.erase(it); _bdSize--;
      (*it).first->removeCoboundaryCell(this,false);
      return false;
    }
    return true;
  }
  _boundary.insert( std::make_pair(cell, orientation ) );
  return true;
}

bool Cell::addCoboundaryCell(int orientation, Cell* cell) {
  _cbdSize++;
  biter it = _coboundary.find(cell);
  if(it != _coboundary.end()){
    (*it).second = (*it).second + orientation;
    if((*it).second == 0) {
      _coboundary.erase(it); _cbdSize--;
      (*it).first->removeBoundaryCell(this,false);
      return false;
    }
    return true;
  }
  _coboundary.insert( std::make_pair(cell, orientation ) );
  return true;
}

int Cell::removeBoundaryCell(Cell* cell, bool other) {
  biter it = _boundary.find(cell);
  if(it != _boundary.end()){
    _boundary.erase(it);
    if(other) (*it).first->removeCoboundaryCell(this, false);
    _bdSize--;
    return (*it).second;
  }
  
  return 0;
} 
int Cell::removeCoboundaryCell(Cell* cell, bool other) {
  biter it = _coboundary.find(cell);
  if(it != _coboundary.end()){
    _coboundary.erase(it);
    if(other) (*it).first->removeBoundaryCell(this, false);
    _cbdSize--;
    return (*it).second;
  }
  return 0;
}
   
bool Cell::hasBoundary(Cell* cell, bool org){
  if(!org){
    biter it = _boundary.find(cell);
    if(it != _boundary.end()) return true;
    return false;
  }
  else{
    biter it = _obd.find(cell);
    if(it != _obd.end()) return true;
    return false;
  }
}

bool Cell::hasCoboundary(Cell* cell, bool org){
  if(!org){
    biter it = _coboundary.find(cell);
    if(it != _coboundary.end()) return true;
    return false;
  }
  else{
    biter it = _ocbd.find(cell);
    if(it != _ocbd.end()) return true;
    return false;
  } 
}

void Cell::makeDualCell(){ 
  std::map<Cell*, int, Less_Cell > temp = _boundary;
  _boundary = _coboundary;
  _coboundary = temp;
  _dim = 3-_dim;     
}

void Cell::printBoundary() {  
  for(biter it = _boundary.begin(); it != _boundary.end(); it++){
    printf("Boundary cell orientation: %d ", (*it).second);
    Cell* cell2 = (*it).first;
    cell2->printCell();
  }
  if(_boundary.empty()) printf("Cell boundary is empty. \n");
}
void Cell::printCoboundary() {
  for(biter it = _coboundary.begin(); it != _coboundary.end(); it++){
    printf("Coboundary cell orientation: %d, ", (*it).second);
    Cell* cell2 = (*it).first;
    cell2->printCell();
    if(_coboundary.empty()) printf("Cell coboundary is empty. \n");
  }
}
   
void Cell::printOrgBd() {
  for(biter it = _obd.begin(); it != _obd.end(); it++){
    printf("Boundary cell orientation: %d ", (*it).second);
    Cell* cell2 = (*it).first;
    cell2->printCell();
  }
  if(_obd.empty()) printf("Cell boundary is empty. \n");
}
void Cell::printOrgCbd() {
  for(biter it = _ocbd.begin(); it != _ocbd.end(); it++){
    printf("Coboundary cell orientation: %d, ", (*it).second);
    Cell* cell2 = (*it).first;
    cell2->printCell();
    if(_ocbd.empty()) printf("Cell coboundary is empty. \n");
  }
}  


CombinedCell::CombinedCell(Cell* c1, Cell* c2, bool orMatch, bool co) : Cell() {
  
  // use "smaller" cell as c2
  if(c1->getNumVertices() < c2->getNumVertices()){
    Cell* temp = c1;
    c1 = c2;
    c2 = temp;
  }
  
  _index = c1->getIndex();
  _tag = c1->getTag();
  _dim = c1->getDim();
  _num = c1->getNum();
  _inSubdomain = c1->inSubdomain();
  _onDomainBoundary = c1->onDomainBoundary();
  _combined = true;
  
  _v.reserve(c1->getNumVertices() + c2->getNumVertices());
  _vs.reserve(c1->getNumVertices() + c2->getNumVertices());
  
     
  _v = c1->getVertexVector();
  for(int i = 0; i < c2->getNumVertices(); i++){
    if(!this->hasVertex(c2->getVertex(i)->getNum())) _v.push_back(c2->getVertex(i));
  }
  
  // sorted vertices
  for(unsigned int i = 0; i < _v.size(); i++) _vs.push_back(_v.at(i)->getNum());
  std::sort(_vs.begin(), _vs.end());
  
  // cells
  _cells = c1->getCells();
  std::list< std::pair<int, Cell*> > c2Cells = c2->getCells();
  for(std::list< std::pair<int, Cell*> >::iterator it = c2Cells.begin(); it != c2Cells.end(); it++){
    if(!orMatch) (*it).first = -1*(*it).first;
    _cells.push_back(*it);
  }
  
  // boundary cells
  std::map< Cell*, int, Less_Cell > c1Boundary = c1->getOrientedBoundary();
  std::map< Cell*, int, Less_Cell > c2Boundary = c2->getOrientedBoundary();
  
  for(std::map<Cell*, int, Less_Cell>::iterator it = c1Boundary.begin(); it != c1Boundary.end(); it++){
    Cell* cell = (*it).first;
    int ori = (*it).second;
    cell->removeCoboundaryCell(c1); 
    if(this->addBoundaryCell(ori, cell)) cell->addCoboundaryCell(ori, this);
  }
  for(std::map<Cell*, int, Less_Cell >::iterator it = c2Boundary.begin(); it != c2Boundary.end(); it++){
    Cell* cell = (*it).first;
    if(!orMatch) (*it).second = -1*(*it).second;
    int ori = (*it).second;
    cell->removeCoboundaryCell(c2);    
    if(co){
      std::map<Cell*, int, Less_Cell >::iterator it2 = c1Boundary.find(cell);
      bool old = false;
      if(it2 != c1Boundary.end()) old = true;
      if(!old){  if(this->addBoundaryCell(ori, cell)) cell->addCoboundaryCell(ori, this); }
    }
    else{
      if(this->addBoundaryCell(ori, cell)) cell->addCoboundaryCell(ori, this);
    }
  }
  
  // coboundary cells
  std::map<Cell*, int, Less_Cell > c1Coboundary = c1->getOrientedCoboundary();
  std::map<Cell*, int, Less_Cell > c2Coboundary = c2->getOrientedCoboundary();
  
  for(std::map<Cell*, int, Less_Cell>::iterator it = c1Coboundary.begin(); it != c1Coboundary.end(); it++){
    Cell* cell = (*it).first;
    int ori = (*it).second;
    cell->removeBoundaryCell(c1); 
    if(this->addCoboundaryCell(ori, cell)) cell->addBoundaryCell(ori, this);
  }
  for(std::map<Cell*, int, Less_Cell>::iterator it = c2Coboundary.begin(); it != c2Coboundary.end(); it++){
    Cell* cell = (*it).first;
    if(!orMatch) (*it).second = -1*(*it).second;
    int ori = (*it).second;
    cell->removeBoundaryCell(c2);    
    if(!co){
      std::map<Cell*, int, Less_Cell >::iterator it2 = c1Coboundary.find(cell);
      bool old = false;
      if(it2 != c1Coboundary.end()) old = true;
      if(!old) { if(this->addCoboundaryCell(ori, cell)) cell->addBoundaryCell(ori, this); }
    }
    else {
      if(this->addCoboundaryCell(ori, cell)) cell->addBoundaryCell(ori, this);
    }
  }
  
}


CombinedCell::~CombinedCell(){
  std::list< std::pair<int, Cell*> > cells = getCells();
  for(std::list< std::pair<int, Cell*> >::iterator it = cells.begin(); it != cells.end(); it++){
    Cell* cell2 = (*it).second;
    delete cell2;
  }
} 

bool CombinedCell::hasVertex(int vertex) const {
  /*std::vector<int>::const_iterator it = std::find(_vs.begin(), _vs.end(), vertex);
  if (it != _vs.end()) return true;
  else return false;*/
  for(unsigned int i = 0; i < _v.size(); i++){
    if(_v.at(i)->getNum() == vertex) return true;
  }
  return false;
}

void CombinedCell::printCell() const {
  printf("Cell dimension: %d, ", getDim() ); 
  printf("Vertices: ");
  for(int i = 0; i < this->getNumVertices(); i++){
    printf("%d ", this->getSortedVertex(i));
  }
  printf(", in subdomain: %d\n", _inSubdomain);
}

/*
 int Cell::incidence(Cell* tau) const{
 for(int i=0; i < tau->getNumVertices(); i++){
 if( !(this->hasVertex(tau->getVertex(i)->getNum())) ) return 0;
 }
  
 if(this->getDim() - tau->getDim() != 1) return 0;
  
 int value=1;
 
  for(int i = 0; i < this->getNumFacets(); i++){
    std::vector<MVertex*> vTau = tau->getVertexVector(); 
    std::vector<MVertex*> v;
    this->getFacetVertices(i, v);
    value = -1;
    
    if(v.size() != vTau.size()) printf("Error: invalid facet!");
    
    do {
      value = value*-1;
      if(v == vTau) return value;
    }
    while (std::next_permutation(vTau.begin(), vTau.end()) );
    
    vTau = tau->getVertexVector();
    value = -1;
    do {
      value = value*-1;
      if(v == vTau) return value;
    }
    while (std::prev_permutation(vTau.begin(), vTau.end()) );
    
    
  }
  
  return 0;
}*/

#endif
