// Gmsh - Copyright (C) 1997-2010 C. Geuzaine, J.-F. Remacle
//
// See the LICENSE.txt file for license information. Please report all
// bugs and problems to <gmsh@geuz.org>.

#ifndef _MELEMENT_OCTREE_
#define _MELEMENT_OCTREE_

#include <vector>

class Octree;
class GModel;
class MElement;

Octree *buildMElementOctree(GModel *);
Octree *buildMElementOctree(std::vector<MElement*> &);

#endif
