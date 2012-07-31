#ifndef _FUNCTIONSPACE_H_
#define _FUNCTIONSPACE_H_

#include <vector>

#include "Basis.h"
#include "GroupOfElement.h"
#include "MElement.h"
#include "Dof.h"

/** 
    @class FunctionSpace
    @brief A Function Space
    
    This class represents a Function Space

    @todo 
    Hybrid Mesh@n
*/

class ElementComparator;

class FunctionSpace{
 private:
  const Basis* basis;
  const GroupOfElement* goe;

  int fPerVertex;
  int fPerEdge;
  int fPerFace;
  int fPerCell;

 public:
   FunctionSpace(const GroupOfElement& goe, 
		 int basisType, int order);

  ~FunctionSpace(void);

  const GroupOfElement& getSupport(void) const;
  const Basis&          getBasis(const MElement& element) const;

  const std::vector<Dof*> getKeys(const MElement& element) const;

  int getNFunctionPerVertex(const MElement& element) const;
  int getNFunctionPerEdge(const MElement& element) const;
  int getNFunctionPerFace(const MElement& element) const;
  int getNFunctionPerCell(const MElement& element) const;
};

//////////////////////
// Inline Functions //
//////////////////////

inline const GroupOfElement& FunctionSpace::getSupport(void) const{
  return *goe;
}

inline const Basis& FunctionSpace::getBasis(const MElement& element) const{
  return *basis;
}

inline int FunctionSpace::getNFunctionPerVertex(const MElement& element) const{
  return fPerVertex;
}

inline int FunctionSpace::getNFunctionPerEdge(const MElement& element) const{
  return fPerEdge;
}

inline int FunctionSpace::getNFunctionPerFace(const MElement& element) const{
  return fPerFace;
}

inline int FunctionSpace::getNFunctionPerCell(const MElement& element) const{
  return fPerCell;
}

#endif
