// $Id: BoundaryLayer.cpp,v 1.3 2007-09-03 12:00:28 geuzaine Exp $
//
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

#include "BoundaryLayer.h"
#include "ExtrudeParams.h"
#include "meshGEdge.h"
#include "meshGFace.h"
#include "Message.h"
#include "Views.h"

template<class T>
static void addExtrudeNormals(std::vector<T*> &elements)
{
  for(unsigned int i = 0; i < elements.size(); i++){
    MElement *ele = elements[i];
    for(int j = 0; j < ele->getNumFaces(); j++){
      MFace fac = ele->getFace(j);
      SVector3 n = fac.normal();
      if(n[0] || n[1] || n[2]){
	double nn[3] = {n[0], n[1], n[2]};
	for(int k = 0; k < fac.getNumVertices(); k++){
	  MVertex *v = fac.getVertex(k);
	  SPoint3 p(v->x(), v->y(), v->z());
	  ExtrudeParams::normals->add(p[0], p[1], p[2], 3, nn);
	}
      }
    }
  }
}

int MeshBoundaryLayerFaces(GModel *m)
{
  bool haveBoundaryLayers = false;
  for(GModel::fiter it = m->firstFace(); it != m->lastFace(); it++){
    GFace *gf = *it;
    if(gf->geomType() == GEntity::BoundaryLayerSurface){
      haveBoundaryLayers = true;
      break;
    }
  }
  if(!haveBoundaryLayers) return 0;

  // make sure the surface mesh is oriented correctly (normally we do
  // this only after the 3D mesh is done; but here it's critical since
  // we use the normals for the extrusion)
  std::for_each(m->firstFace(), m->lastFace(), orientMeshGFace());

  // compute a normal field for the extrusion
  if(ExtrudeParams::normals) delete ExtrudeParams::normals;
  ExtrudeParams::normals = new smooth_data();
  ExtrudeParams *myep = 0;
  for(GModel::fiter it = m->firstFace(); it != m->lastFace(); it++){
    GFace *gf = *it;
    if(gf->geomType() == GEntity::BoundaryLayerSurface){
      ExtrudeParams *ep = myep = gf->meshAttributes.extrude;
      if(ep && ep->mesh.ExtrudeMesh && ep->geo.Mode == COPIED_ENTITY){
	if(ep->mesh.ViewIndex >= 0 && ep->mesh.ViewIndex < List_Nbr(CTX.post.list)){ 
	  // use external vector point post-pro view to get normals
	  // FIXME: should use an octree on a general view instead
	  xyzv::eps = 1.e-4;
	  Post_View *v = *(Post_View**)List_Pointer(CTX.post.list, ep->mesh.ViewIndex);
	  if(v->NbVP){
	    int nb = List_Nbr(v->VP) / v->NbVP;
	    for(int i = 0; i < List_Nbr(v->VP); i += nb){
	      double *data = (double*)List_Pointer_Fast(v->VP, i);
	      ExtrudeParams::normals->add(data[0], data[1], data[2], 3, &data[3]);
	    }
	  }
	}
	else{ 
	  // compute smooth normal field from surfaces
	  GFace *from = gf->model()->faceByTag(std::abs(ep->geo.Source));
	  if(!from){
	    Msg(GERROR, "Unknown source face %d for boundary layer", ep->geo.Source);
	    continue;
	  }
	  addExtrudeNormals(from->triangles);
	  addExtrudeNormals(from->quadrangles);
	}
      }
    }
  }
  ExtrudeParams::normals->normalize();
  //ExtrudeParams::normals->exportview("normals.pos");
  if(!myep) return 0;

  // set the position of bounding points (FIXME: should check
  // coherence of all extrude parameters)
  for(GModel::viter it = m->firstVertex(); it != m->lastVertex(); it++){
    GVertex *gv = *it;
    if(gv->geomType() == GEntity::BoundaryLayerPoint){
      GPoint p = gv->point();
      myep->Extrude(myep->mesh.NbLayer - 1, myep->mesh.NbElmLayer[myep->mesh.NbLayer - 1],
		    p.x(), p.y(), p.z());
      gv->setPosition(p);
    }
  }
  
  // mesh the curves bounding the boundary layers by extrusion using
  // the smooth normal field
  for(GModel::eiter it = m->firstEdge(); it != m->lastEdge(); it++){
    GEdge *ge = *it;
    if(ge->geomType() == GEntity::BoundaryLayerCurve){
      Msg(INFO, "Meshing curve %d", ge->tag());
      deMeshGEdge dem;
      dem(ge);
      MeshExtrudedCurve(ge);
    }
  }

  // mesh the surfaces bounding the boundary layers by extrusion using
  // the smooth normal field
  for(GModel::fiter it = m->firstFace(); it != m->lastFace(); it++){
    GFace *gf = *it;
    if(gf->geomType() == GEntity::BoundaryLayerSurface){
      Msg(STATUS2, "Meshing surface %d (%s)", gf->tag(), gf->getTypeString().c_str());
      deMeshGFace dem;
      dem(gf);
      MeshExtrudedSurface(gf);
    }
  }

  return 1;
}
