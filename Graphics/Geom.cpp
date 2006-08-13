// $Id: Geom.cpp,v 1.108 2006-08-13 02:46:53 geuzaine Exp $
//
// Copyright (C) 1997-2006 C. Geuzaine, J.-F. Remacle
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

#include "Gmsh.h"
#include "GmshUI.h"
#include "Draw.h"
#include "Context.h"
#include "gl2ps.h"
#include "GModel.h"
#include "SBoundingBox3d.h"

extern Context_T CTX;
extern GModel *GMODEL;

class drawGVertex
{
public :
  void operator () (GVertex *v)
  {
    if(!v->getVisibility()) return;

    if(CTX.render_mode == GMSH_SELECT) {
      glPushName(0);
      glPushName(v->tag());
    }
    
    if(v->getFlag() > 0) {
      glPointSize(CTX.geom.point_sel_size);
      gl2psPointSize(CTX.geom.point_sel_size * CTX.print.eps_point_size_factor);
      glColor4ubv((GLubyte *) & CTX.color.geom.point_sel);
    }
    else {
      glPointSize(CTX.geom.point_size);
      gl2psPointSize(CTX.geom.point_size * CTX.print.eps_point_size_factor);
      glColor4ubv((GLubyte *) & CTX.color.geom.point);
    }
    
    if(CTX.geom.points) {
      if(CTX.geom.point_type > 0) {
	if(v->getFlag() > 0)
	  Draw_Sphere(CTX.geom.point_sel_size, v->x(), v->y(), v->z(), 
		      CTX.geom.light);
	else
	  Draw_Sphere(CTX.geom.point_size, v->x(), v->y(), v->z(),
		      CTX.geom.light);
      }
      else {
	glBegin(GL_POINTS);
	glVertex3d(v->x(), v->y(), v->z());
	glEnd();
      }
    }

    if(CTX.geom.points_num) {
      char Num[100];
      sprintf(Num, "%d", v->tag());
      double offset = (0.5 * CTX.geom.point_size + 0.3 * CTX.gl_fontsize) * 
	CTX.pixel_equiv_x;
      glRasterPos3d(v->x() + offset / CTX.s[0],
		    v->y() + offset / CTX.s[1],
		    v->z() + offset / CTX.s[2]);
      Draw_String(Num);
    }
    
    if(CTX.render_mode == GMSH_SELECT) {
      glPopName();
      glPopName();
    }
  }
};

class drawGEdge
{
public :
  void operator () (GEdge *e)
  {
    if(!e->getVisibility())
      return;
    
    if(CTX.render_mode == GMSH_SELECT) {
      glPushName(1);
      glPushName(e->tag());
    }
    
    if(e->getFlag() > 0) {
      glLineWidth(CTX.geom.line_sel_width);
      gl2psLineWidth(CTX.geom.line_sel_width * CTX.print.eps_line_width_factor);
      glColor4ubv((GLubyte *) & CTX.color.geom.line_sel);
    }
    else {
      glLineWidth(CTX.geom.line_width);
      gl2psLineWidth(CTX.geom.line_width * CTX.print.eps_line_width_factor);
      glColor4ubv((GLubyte *) & CTX.color.geom.line);
    }
    
    Range<double> t_bounds = e->parBounds(0);
    double t_min = t_bounds.low();
    double t_max = t_bounds.high();
    
    if(CTX.geom.lines) {
      if(e->geomType() == GEntity::DiscreteCurve){
	// do nothing: we draw the elements in the mesh drawing routines
      }
      else {
	int N = e->minimumDrawSegments() + 1;
	if(CTX.geom.line_type > 0) {
	  for(int i = 0; i < N - 1; i++) {
	    double t1 = t_min + (double)i / (double)(N - 1) * (t_max - t_min);
	    GPoint p1 = e->point(t1);
	    double t2 = t_min + (double)(i + 1) / (double)(N - 1) * (t_max - t_min);
	    GPoint p2 = e->point(t2);
	    double x[2] = {p1.x(), p2.x()};
	    double y[2] = {p1.y(), p2.y()};
	    double z[2] = {p1.z(), p2.z()};
	    Draw_Cylinder(e->getFlag() > 0 ? CTX.geom.line_sel_width : 
			  CTX.geom.line_width, x, y, z, CTX.geom.light);
	  }
	}
	else {
	  glBegin(GL_LINE_STRIP);
	  for(int i = 0; i < N; i++) {
	    double t = t_min + (double)i / (double)(N - 1) * (t_max - t_min);
	    GPoint p = e->point(t);
	    glVertex3d(p.x(), p.y(), p.z());
	  }
	  glEnd();
	}
      }
    }
    
    if(CTX.geom.lines_num) {
      GPoint p = e->point(0.5 * (t_max - t_min));
      char Num[100];
      sprintf(Num, "%d", e->tag());
      double offset = (0.5 * CTX.geom.line_width + 0.3 * CTX.gl_fontsize) * CTX.pixel_equiv_x;
      glRasterPos3d(p.x() + offset / CTX.s[0],
		    p.y() + offset / CTX.s[1],
		    p.z() + offset / CTX.s[2]);
      Draw_String(Num);
    }
    
    if(CTX.geom.tangents) {
      double t = 0.5 * (t_max - t_min);
      GPoint p = e->point(t);
      SVector3 der = e->firstDer(t) ;
      double mod = sqrt(der[0] * der[0] + der[1] * der[1] + der[2] * der[2]);
      for(int i = 0; i < 3; i++)
	der[i] = der[i] / mod * CTX.geom.tangents * CTX.pixel_equiv_x / CTX.s[i];
      glColor4ubv((GLubyte *) & CTX.color.geom.tangents);
      Draw_Vector(CTX.vector_type, 0, CTX.arrow_rel_head_radius, 
		  CTX.arrow_rel_stem_length, CTX.arrow_rel_stem_radius,
		  p.x(), p.y(), p.z(), der[0], der[1], der[2], CTX.geom.light);
    }
    
    if(CTX.render_mode == GMSH_SELECT) {
      glPopName();
      glPopName();
    }
  }
};

class drawGFace
{
private:
  void _drawNonPlaneGFace(GFace *f)
  {
    if(CTX.geom.surfaces) {
      glEnable(GL_LINE_STIPPLE);
      glLineStipple(1, 0x1F1F);
      gl2psEnable(GL2PS_LINE_STIPPLE);
      int N = 20;
      glBegin(GL_LINE_STRIP);
      for(int i = 0; i < N; i++) {
	GPoint coords = f->point((double)i / (double)(N - 1), 0.5);
	glVertex3d(coords.x(), coords.y(), coords.z());
      }
      glEnd();
      glBegin(GL_LINE_STRIP);
      for(int i = 0; i < N; i++) {
	GPoint coords = f->point(0.5, (double)i / (double)(N - 1));
	glVertex3d(coords.x(), coords.y(), coords.z());
      }
      glEnd();
      glDisable(GL_LINE_STIPPLE);
      gl2psDisable(GL2PS_LINE_STIPPLE);
    }
    
    if(CTX.geom.surfaces_num) {
      GPoint coords = f->point(0.5, 0.5);
      char Num[100];
      sprintf(Num, "%d", f->tag());
      double offset = 0.3 * CTX.gl_fontsize * CTX.pixel_equiv_x;
      glRasterPos3d(coords.x() + offset / CTX.s[0],
		    coords.y() + offset / CTX.s[1],
		    coords.z() + offset / CTX.s[2]);
      Draw_String(Num);
    }
    
    if(CTX.geom.normals) {
      SPoint2 p2 = SPoint2(0.5, 0.5);
      SVector3 nn = f->normal(p2);
      GPoint p = f->point(p2);
      double n[3] = {nn.x() * CTX.geom.normals * CTX.pixel_equiv_x / CTX.s[0],
		     nn.y() * CTX.geom.normals * CTX.pixel_equiv_x / CTX.s[1],
		     nn.z() * CTX.geom.normals * CTX.pixel_equiv_x / CTX.s[2]};
      glColor4ubv((GLubyte *) & CTX.color.geom.normals);
      Draw_Vector(CTX.vector_type, 0, CTX.arrow_rel_head_radius, 
		  CTX.arrow_rel_stem_length, CTX.arrow_rel_stem_radius,
		  p.x(), p.y(), p.z(), n[0], n[1], n[2], CTX.geom.light);
    }
  }
  
  void _drawPlaneGFace(GFace *f)
  {
    // we must lock this part to avoid it being called recursively
    // when redraw events are fired in rapid succession
    if(!f->cross.size() && !CTX.threads_lock) {
      CTX.threads_lock = 1; 
      std::list<GVertex*> pts = f->vertices();
      if(pts.size()){
	SBoundingBox3d bb;
	for(std::list<GVertex*>::iterator it = pts.begin(); it != pts.end(); it++){
	  SPoint3 p((*it)->x(), (*it)->y(), (*it)->z());
	  SPoint2 uv = f->parFromPoint(p);
	  bb += SPoint3(uv.x(), uv.y(), 0.);
	}
	GPoint v0 = f->point(bb.min().x(), bb.min().y());
	GPoint v1 = f->point(bb.max().x(), bb.min().y());
	GPoint v2 = f->point(bb.max().x(), bb.max().y());
	GPoint v3 = f->point(bb.min().x(), bb.max().y());
	const int N = 100;
	for(int dir = 0; dir < 2; dir++) {
	  int end_line = 0;
	  SPoint3 pt, pt_last_good;
	  for(int i = 0; i < N; i++) {
	    double t = (double)i / (double)(N - 1);
	    double x, y, z;
	    if(dir){
	      x = t * 0.5 * (v0.x() + v1.x()) + (1. - t) * 0.5 * (v2.x() + v3.x());
	      y = t * 0.5 * (v0.y() + v1.y()) + (1. - t) * 0.5 * (v2.y() + v3.y());
	      z = t * 0.5 * (v0.z() + v1.z()) + (1. - t) * 0.5 * (v2.z() + v3.z());
	    }
	    else{
	      x = t * 0.5 * (v0.x() + v3.x()) + (1. - t) * 0.5 * (v2.x() + v1.x());
	      y = t * 0.5 * (v0.y() + v3.y()) + (1. - t) * 0.5 * (v2.y() + v1.y());
	      z = t * 0.5 * (v0.z() + v3.z()) + (1. - t) * 0.5 * (v2.z() + v1.z());
	    }
	    pt.setPosition(x, y, z);
	    if(f->containsPoint(pt)){
	      pt_last_good.setPosition(pt.x(), pt.y(), pt.z());
	      if(!end_line) { f->cross.push_back(pt); end_line = 1; }
	    }
	    else {
	      if(end_line) { f->cross.push_back(pt_last_good); end_line = 0; }
	    }
	  }
	  if(end_line) f->cross.push_back(pt_last_good);
	}
      }
      // if we couldn't determine a cross, add dummy point so that
      // we won't try again
      if(!f->cross.size()) f->cross.push_back(SPoint3(0., 0., 0.));
      CTX.threads_lock = 0;
    }

    if(f->cross.size() < 2) return ;

    if(CTX.geom.surfaces) {
      glEnable(GL_LINE_STIPPLE);
      glLineStipple(1, 0x1F1F);
      gl2psEnable(GL2PS_LINE_STIPPLE);
      glBegin(GL_LINES);
      for(unsigned int i = 0; i < f->cross.size(); i++)
	glVertex3d(f->cross[i].x(), f->cross[i].y(), f->cross[i].z());
      glEnd();
      glDisable(GL_LINE_STIPPLE);
      gl2psDisable(GL2PS_LINE_STIPPLE);
    }

    if(CTX.geom.surfaces_num) {
      char Num[100];
      sprintf(Num, "%d", f->tag());
      double offset = 0.3 * CTX.gl_fontsize * CTX.pixel_equiv_x;
      glRasterPos3d(0.5 * (f->cross[0].x() + f->cross[1].x()) + offset / CTX.s[0],
		    0.5 * (f->cross[0].y() + f->cross[1].y()) + offset / CTX.s[0],
		    0.5 * (f->cross[0].z() + f->cross[1].z()) + offset / CTX.s[0]);
      Draw_String(Num);
    }

    if(CTX.geom.normals) {
      SPoint3 p(0.5 * (f->cross[0].x() + f->cross[1].x()),
		0.5 * (f->cross[0].y() + f->cross[1].y()),
		0.5 * (f->cross[0].z() + f->cross[1].z()));
      SPoint2 uv = f->parFromPoint(p);
      SVector3 nn = f->normal(uv);
      double n[3] = {nn.x() * CTX.geom.normals * CTX.pixel_equiv_x / CTX.s[0],
		     nn.y() * CTX.geom.normals * CTX.pixel_equiv_x / CTX.s[1],
		     nn.z() * CTX.geom.normals * CTX.pixel_equiv_x / CTX.s[2]};
      glColor4ubv((GLubyte *) & CTX.color.geom.normals);
      Draw_Vector(CTX.vector_type, 0, CTX.arrow_rel_head_radius, 
		  CTX.arrow_rel_stem_length, CTX.arrow_rel_stem_radius, 
		  p.x(), p.y(), p.z(), n[0], n[1], n[2], CTX.geom.light);
    }
  }
  
public :
  void operator () (GFace *f)
  {
    if(!f->getVisibility())
      return;
    
    if(CTX.render_mode == GMSH_SELECT) {
      glPushName(2);
      glPushName(f->tag());
    }
    
    if(f->getFlag() > 0) {
      glLineWidth(CTX.geom.line_sel_width / 2.);
      gl2psLineWidth(CTX.geom.line_sel_width / 2. *
		     CTX.print.eps_line_width_factor);
      glColor4ubv((GLubyte *) & CTX.color.geom.surface_sel);
    }
    else {
      glLineWidth(CTX.geom.line_width / 2.);
      gl2psLineWidth(CTX.geom.line_width / 2. * CTX.print.eps_line_width_factor);
      glColor4ubv((GLubyte *) & CTX.color.geom.surface);
    }
    
    if(f->geomType() == GEntity::DiscreteSurface){
      // do nothing: we draw the elements in the mesh drawing routines
    }
    else if(f->geomType() == GEntity::Plane){
      _drawPlaneGFace(f);
    }
    else{
      _drawNonPlaneGFace(f);
    }
    
    if(CTX.render_mode == GMSH_SELECT) {
      glPopName();
      glPopName();
    }
  }
};


class drawGRegion
{
public :
  void operator () (GRegion *r)
  {
  }
};

void Draw_Geom()
{
  glLightModelf(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
  
  for(int i = 0; i < 6; i++)
    if(CTX.clip[i] & 1) 
      glEnable((GLenum)(GL_CLIP_PLANE0 + i));
    else
      glDisable((GLenum)(GL_CLIP_PLANE0 + i));
  
  if(CTX.geom.points || CTX.geom.points_num)
    std::for_each(GMODEL->firstVertex(), GMODEL->lastVertex(), drawGVertex());

  if(CTX.geom.lines || CTX.geom.lines_num || CTX.geom.tangents)
    std::for_each(GMODEL->firstEdge(), GMODEL->lastEdge(), drawGEdge());

  if(CTX.geom.surfaces || CTX.geom.surfaces_num || CTX.geom.normals)
    std::for_each(GMODEL->firstFace(), GMODEL->lastFace(), drawGFace());

  if(CTX.geom.volumes || CTX.geom.volumes_num)
    std::for_each(GMODEL->firstRegion(), GMODEL->lastRegion(), drawGRegion());

  for(int i = 0; i < 6; i++)
    glDisable((GLenum)(GL_CLIP_PLANE0 + i));
}

void HighlightEntity(GEntity *e, int permanent)
{
  if(permanent)
    e->setFlag(2);
  else
    Msg(STATUS2N, "%s", e->getInfoString().c_str());
}

void HighlightEntity(GVertex *v, GEdge *c, GFace *s, int permanent)
{
  if(v) HighlightEntity(v, permanent);
  else if(c) HighlightEntity(c, permanent);
  else if(s) HighlightEntity(s, permanent);
  else if(!permanent) Msg(STATUS2N, " ");
}

void HighlightEntityNum(int v, int c, int s, int permanent)
{
  if(v) {
    GVertex *pv = GMODEL->vertexByTag(v);
    if(pv) HighlightEntity(pv, permanent);
  }
  if(c) {
    GEdge *pc = GMODEL->edgeByTag(c);
    if(pc) HighlightEntity(pc, permanent);
  }
  if(s) {
    GFace *ps = GMODEL->faceByTag(s);
    if(ps) HighlightEntity(ps, permanent);
  }
}

void ZeroHighlightEntity(GEntity *e)
{
  e->setFlag(-2);
}

void ZeroHighlightEntity(GVertex *v, GEdge *c, GFace *s)
{
  if(v) ZeroHighlightEntity(v);
  if(c) ZeroHighlightEntity(c);
  if(s) ZeroHighlightEntity(s);
}

void ZeroHighlight()
{
  for(GModel::viter it = GMODEL->firstVertex(); it != GMODEL->lastVertex(); it++)
    ZeroHighlightEntity(*it);
  for(GModel::eiter it = GMODEL->firstEdge(); it != GMODEL->lastEdge(); it++)
    ZeroHighlightEntity(*it);
  for(GModel::fiter it = GMODEL->firstFace(); it != GMODEL->lastFace(); it++)
    ZeroHighlightEntity(*it);
}

void ZeroHighlightEntityNum(int v, int c, int s)
{
  if(v) {
    GVertex *pv = GMODEL->vertexByTag(v);
    if(pv) ZeroHighlightEntity(pv);
  }
  if(c) {
    GEdge *pc = GMODEL->edgeByTag(c);
    if(pc) ZeroHighlightEntity(pc);
  }
  if(s) {
    GFace *ps = GMODEL->faceByTag(s);
    if(ps) ZeroHighlightEntity(ps);
  }
}
