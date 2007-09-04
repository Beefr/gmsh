#ifndef _MESHGFACEOPTIMIZE_H_
#define _MESHGFACEOPTIMIZE_H_

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
#include "MElement.h"
#include "MEdge.h"
#include <map>
#include <vector>
class GFace;
class MVertex;
typedef std::map<MVertex*,std::vector<MTriangle*> > v2t_cont ;
typedef std::map<MEdge, std::pair<MTriangle*,MTriangle*> , Less_Edge> e2t_cont ;
void buildVertexToTriangle ( std::vector<MTriangle*> & ,  v2t_cont &adj );
void buildEdgeToTriangle ( std::vector<MTriangle*> & ,  e2t_cont &adj );
void laplaceSmoothing   (GFace *gf);
void edgeSwappingLawson (GFace *gf);
#endif
