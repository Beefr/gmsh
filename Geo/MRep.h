#ifndef _MREP_H_
#define _MREP_H_

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

#include <set>
#include <map>
#include <algorithm>
#include "GEdge.h"
#include "GFace.h"
#include "GRegion.h"
#include "MVertex.h"
#include "MEdge.h"
#include "MElement.h"
#include "VertexArray.h"
#include "Message.h"
#include "OS.h"

// #define HAVE_HASH_MAP

#if defined(HAVE_HASH_MAP)
#include <ext/hash_map>
#endif

struct equalEdge {
  bool operator()(const std::pair<MVertex*, MVertex*> &p1, 
		  const std::pair<MVertex*, MVertex*> &p2) const
  {
    return (p1.first == p2.first && p1.second == p2.second);
  }
};

struct hashEdge {
  size_t operator() (const std::pair<MVertex*, MVertex*> &p) const 
  { 
    return p.first->getNum() + p.second->getNum();
  }
};

// A mesh representation.
class MRep {
 protected:

  // container for the edge representation
#if defined(HAVE_HASH_MAP)
  typedef __gnu_cxx::hash_map<std::pair<MVertex*, MVertex*>, MElement*, 
			      hashEdge, equalEdge> ermap;
#else
  typedef std::map<std::pair<MVertex*, MVertex*>, MElement*> ermap;
#endif
  typedef std::map<MFace, MElement*, Equal_Face> frmap;

  ermap edges;
  frmap faces;

  // generates the edges from a bunch of elements
  template<class T>
  void generateEdgeRep(std::vector<T*> &elements)
  {
    for(unsigned int i = 0; i < elements.size(); i++){
      for(int j = 0; j < elements[i]->getNumEdgesRep(); j++){
	MEdge e = elements[i]->getEdgeRep(j);
	std::pair<MVertex*, MVertex*> p(e.getMinVertex(), e.getMaxVertex());
	if(!edges.count(p)) edges[p] = elements[i];
      }
    }
  }

  // generates the boundary faces from a bunch of elements
  template<class T>
  void generateBoundaryFaceRep(std::vector<T*> &elements)
  {
    // FIXME: TODO
    
    for(unsigned int i = 0; i < elements.size(); i++){
      for(int j = 0; j < elements[i]->getNumFacesRep(); j++){
	MFace f = elements[i]->getFaceRep(j);
	frmap::iterator it = faces.find(f);
	if(it == faces.end()) 
	  faces[f] = elements[i];
	else
	  faces.erase(it);
      }
    }
  }

 public:
  // the vertex arrays
  VertexArray *va_lines, *va_triangles, *va_quads;

  // a flag telling if all the elements in the entity are visible
  bool allElementsVisible;

 public:
  MRep() : va_lines(0), va_triangles(0), va_quads(0), allElementsVisible(true) {}
  virtual ~MRep(){ destroy(); }

  // destroys everything
  void destroy(){
    resetArrays();
    edges.clear();
    faces.clear();
    allElementsVisible = true;
  }

  // destroys all the vertex arrays
  void resetArrays(){
    if(va_lines) delete va_lines;
    va_lines = 0;
    if(va_triangles) delete va_triangles;
    va_triangles = 0;
    if(va_quads) delete va_quads;
    va_quads = 0;
  }

  // generates the edge representation
  virtual void generateEdgeRep() = 0;

  // accesses the edge representation
  typedef ermap::const_iterator eriter;
  eriter firstEdgeRep() { return edges.begin(); }
  eriter lastEdgeRep() { return edges.end(); }
  int getNumEdgeRep() { return edges.size(); }

  // generates the boundary face representation
  virtual void generateBoundaryFaceRep(){}

  // accesses the face representation
  typedef frmap::const_iterator friter;
  friter firstFaceRep() { return faces.begin(); }
  friter lastFaceRep() { return faces.end(); }
  int getNumFaceRep() { return faces.size(); }

  // returns the element at a given position in a vertex array
  // (element pointers are not always stored: returning 0 is not an
  // error)
  MElement *getElement(int va_type, int index)
  {
    switch(va_type){
    case 2: 
      if(va_lines && index < va_lines->getNumElementPointers())
	return *va_lines->getElementPointerArray(index);
      break;
    case 3:
      if(va_triangles && index < va_triangles->getNumElementPointers())
	return *va_triangles->getElementPointerArray(index);
      break;
    case 4:
      if(va_quads && index < va_quads->getNumElementPointers())
	return *va_quads->getElementPointerArray(index);
      break;
    }
    return 0;
  }
};

class MRepEdge : public MRep {
 private:
  GEdge *_e;

 public:
  MRepEdge(GEdge *e) : _e(e) {}
  virtual ~MRepEdge(){}
  virtual void generateEdgeRep()
  {
    if(edges.size()) return;
    MRep::generateEdgeRep(_e->lines);
  }
};

class MRepFace : public MRep {
 private:
  GFace *_f;

 public:
  MRepFace(GFace *f) : _f(f) {}
  virtual ~MRepFace(){}
  virtual void generateEdgeRep()
  {
    if(edges.size()) return;
    double t = Cpu();    
    MRep::generateEdgeRep(_f->triangles);
    MRep::generateEdgeRep(_f->quadrangles);
    Msg(DEBUG, "Created %d edges in surface %d (%gs)",
	(int)edges.size(), _f->tag(), Cpu()-t);
  }
};

class MRepRegion : public MRep {
 private:
  GRegion *_r;

 public:
  MRepRegion(GRegion *r) : _r(r) {}
  virtual ~MRepRegion(){}
  virtual void generateEdgeRep()
  {
    if(edges.size()) return;
    double t = Cpu();    
    MRep::generateEdgeRep(_r->tetrahedra);
    MRep::generateEdgeRep(_r->hexahedra);
    MRep::generateEdgeRep(_r->prisms);
    MRep::generateEdgeRep(_r->pyramids);
    Msg(DEBUG, "Created %d edges in volume %d (%gs)",
	(int)edges.size(), _r->tag(), Cpu()-t);
  }
  virtual void generateBoundaryFaceRep()
  {
    if(faces.size()) return;
    double t = Cpu();    
    MRep::generateBoundaryFaceRep(_r->tetrahedra);
    MRep::generateBoundaryFaceRep(_r->hexahedra);
    MRep::generateBoundaryFaceRep(_r->prisms);
    MRep::generateBoundaryFaceRep(_r->pyramids);
    Msg(DEBUG, "Created %d boundary faces in volume %d (%gs)",
	(int)faces.size(), _r->tag(), Cpu()-t);
  }
};

#endif
