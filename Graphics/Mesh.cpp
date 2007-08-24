// $Id: Mesh.cpp,v 1.201 2007-08-24 20:14:18 geuzaine Exp $
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

#include "Gmsh.h"
#include "GmshUI.h"
#include "GModel.h"
#include "Draw.h"
#include "Context.h"
#include "MRep.h"
#include "OS.h"
#include "gl2ps.h"

extern Context_T CTX;

// General helper routines

static unsigned int getColorByEntity(GEntity *e)
{
  if(e->getSelection()){ // selection
    return CTX.color.geom.selection;
  }
  else if(e->useColor()){ // forced from a script
    return e->getColor();
  }
  else if(CTX.mesh.color_carousel == 1){ // by elementary entity
    return CTX.color.mesh.carousel[abs(e->tag() % 20)];
  }
  else if(CTX.mesh.color_carousel == 2){ // by physical entity
    int np = e->physicals.size();
    int p = np ? e->physicals[np - 1] : 0;
    return CTX.color.mesh.carousel[abs(p % 20)];
  }
  else{
    return CTX.color.fg;
  }
}

static unsigned int getColorByElement(MElement *ele)
{
  if(ele->getVisibility() > 1){ // selection
    return CTX.color.geom.selection;
  }
  else if(CTX.mesh.color_carousel == 0){ // by element type
    switch(ele->getNumEdges()){
    case 1: return CTX.color.mesh.line;
    case 3: return CTX.color.mesh.triangle;
    case 4: return CTX.color.mesh.quadrangle;
    case 6: return CTX.color.mesh.tetrahedron;
    case 12: return CTX.color.mesh.hexahedron;
    case 9: return CTX.color.mesh.prism;
    case 8: return CTX.color.mesh.pyramid;
    default: return CTX.color.mesh.vertex;
    }
  }
  else if(CTX.mesh.color_carousel == 3){ // by partition
    return CTX.color.mesh.carousel[abs(ele->getPartition() % 20)];
  }
  else{ // by elementary or physical entity
    // this is not perfect (e.g. a triangle can have no vertices
    // categorized on a surface), but it's the best we can do "fast"
    // since we don't store the associated entity in the element
    for(int i = 0; i < ele->getNumVertices(); i++){
      GEntity *e = ele->getVertex(i)->onWhat();
      if(e && (e->dim() == ele->getDim()))
	return getColorByEntity(e);
    }
  }
  return CTX.color.fg;
}

static double intersectCutPlane(MElement *ele)
{
  MVertex *v = ele->getVertex(0);
  double val = CTX.mesh.evalCutPlane(v->x(), v->y(), v->z());
  for(int i = 1; i < ele->getNumVertices(); i++){
    v = ele->getVertex(i);
    if(val * CTX.mesh.evalCutPlane(v->x(), v->y(), v->z()) <= 0)
      return 0.; // the element intersects the cut plane
  }
  return val;
}

static bool isElementVisible(MElement *ele)
{
  if(!ele->getVisibility()) return false;
  if(CTX.mesh.quality_sup) {
    double q;
    if(CTX.mesh.quality_type == 2)
      q = ele->rhoShapeMeasure();
    else if(CTX.mesh.quality_type == 1)
      q = ele->etaShapeMeasure();
    else
      q = ele->gammaShapeMeasure();
    if(q < CTX.mesh.quality_inf || q > CTX.mesh.quality_sup) return false;
  }
  if(CTX.mesh.radius_sup) {
    double r = ele->maxEdge();
    if(r < CTX.mesh.radius_inf || r > CTX.mesh.radius_sup) return false;
  }
  if(CTX.mesh.use_cut_plane){
    if(ele->getDim() < 3 && CTX.mesh.cut_plane_only_volume){
    }
    else if(intersectCutPlane(ele) < 0) return false;
  }
  return true;
}

template<class T>
static bool areAllElementsVisible(std::vector<T*> &elements)
{
  for(unsigned int i = 0; i < elements.size(); i++)
    if(!isElementVisible(elements[i])) return false;
  return true;
}

template<class T>
static bool areSomeElementsCurved(std::vector<T*> &elements)
{
  for(unsigned int i = 0; i < elements.size(); i++)
    if(elements[i]->getPolynomialOrder() > 1 ) return true;
  return false;
}

static int getLabelStep(int total)
{
  int step;
  if(CTX.mesh.label_frequency == 0.0) 
    step = total;
  else 
    step = (int)(100.0 / CTX.mesh.label_frequency);
  return (step > total) ? total : step;
}

template<class T>
static void drawElementLabels(GEntity *e, std::vector<T*> &elements,
			      int forceColor=0, unsigned int color=0)
{
  unsigned col = forceColor ? color : getColorByEntity(e);
  glColor4ubv((GLubyte *) & col);

  int labelStep = getLabelStep(elements.size());

  for(unsigned int i = 0; i < elements.size(); i++){
    MElement *ele = elements[i];
    if(!isElementVisible(ele)) continue;
    if(i % labelStep == 0) {
      SPoint3 pc = ele->barycenter();
      char str[256];
      if(CTX.mesh.label_type == 4)
	sprintf(str, "(%g,%g,%g)", pc.x(), pc.y(), pc.z());
      else if(CTX.mesh.label_type == 3)
	sprintf(str, "%d", ele->getPartition());
      else if(CTX.mesh.label_type == 2){
	int np = e->physicals.size();
	int p = np ? e->physicals[np - 1] : 0;
	sprintf(str, "%d", p);
      }
      else if(CTX.mesh.label_type == 1)
	sprintf(str, "%d", e->tag());
      else
	sprintf(str, "%d", ele->getNum());
      glRasterPos3d(pc.x(), pc.y(), pc.z());
      Draw_String(str);
    }
  }
}

template<class T>
static void drawNormals(std::vector<T*> &elements)
{
  glColor4ubv((GLubyte *) & CTX.color.mesh.normals);
  for(unsigned int i = 0; i < elements.size(); i++){
    MElement *ele = elements[i];
    if(!isElementVisible(ele)) continue;
    SVector3 n = ele->getFaceRep(0).normal();
    for(int j = 0; j < 3; j++)
      n[j] *= CTX.mesh.normals * CTX.pixel_equiv_x / CTX.s[j];
    SPoint3 pc = ele->barycenter();
    Draw_Vector(CTX.vector_type, 0, CTX.arrow_rel_head_radius, 
		CTX.arrow_rel_stem_length, CTX.arrow_rel_stem_radius,
		pc.x(), pc.y(), pc.z(), n[0], n[1], n[2], CTX.mesh.light);
  }
}

template<class T>
static void drawTangents(std::vector<T*> &elements)
{
  glColor4ubv((GLubyte *) & CTX.color.mesh.tangents);
  for(unsigned int i = 0; i < elements.size(); i++){
    MElement *ele = elements[i];
    if(!isElementVisible(ele)) continue;
    SVector3 t = ele->getEdgeRep(0).tangent();
    for(int j = 0; j < 3; j++)
      t[j] *= CTX.mesh.tangents * CTX.pixel_equiv_x / CTX.s[j];
    SPoint3 pc = ele->barycenter();
    Draw_Vector(CTX.vector_type, 0, CTX.arrow_rel_head_radius, 
		CTX.arrow_rel_stem_length, CTX.arrow_rel_stem_radius,
		pc.x(), pc.y(), pc.z(), t[0], t[1], t[2], CTX.mesh.light);
  }
}

static void drawVertexLabel(GEntity *e, MVertex *v, int partition=-1)
{
  int np = e->physicals.size();
  int physical = np ? e->physicals[np - 1] : 0;
  char str[256];
  if(CTX.mesh.label_type == 4)
    sprintf(str, "(%.16g,%.16g,%.16g)", v->x(), v->y(), v->z());
  else if(CTX.mesh.label_type == 3){
    if(partition < 0)
      sprintf(str, "NA");
    else
      sprintf(str, "%d", partition);
  }
  else if(CTX.mesh.label_type == 2)
    sprintf(str, "%d", physical);
  else if(CTX.mesh.label_type == 1)
    sprintf(str, "%d", e->tag());
  else
    sprintf(str, "%d", v->getNum());

  if(v->getPolynomialOrder() > 1)
    glColor4ubv((GLubyte *) & CTX.color.mesh.vertex_sup);
  else
    glColor4ubv((GLubyte *) & CTX.color.mesh.vertex);	
  glRasterPos3d(v->x(), v->y(), v->z());
  Draw_String(str);
}

static void drawVerticesPerEntity(GEntity *e)
{
  if(CTX.mesh.points) {
    if(CTX.mesh.point_type) {
      for(unsigned int i = 0; i < e->mesh_vertices.size(); i++){
	MVertex *v = e->mesh_vertices[i];
	if(!v->getVisibility()) continue;
	if(v->getPolynomialOrder() > 1)
	  glColor4ubv((GLubyte *) & CTX.color.mesh.vertex_sup);
	else
	  glColor4ubv((GLubyte *) & CTX.color.mesh.vertex);	
	Draw_Sphere(CTX.mesh.point_size, v->x(), v->y(), v->z(), CTX.mesh.light);
      }
    }
    else{
      glBegin(GL_POINTS);
      for(unsigned int i = 0; i < e->mesh_vertices.size(); i++){
	MVertex *v = e->mesh_vertices[i];
	if(!v->getVisibility()) continue;
	if(v->getPolynomialOrder() > 1)
	  glColor4ubv((GLubyte *) & CTX.color.mesh.vertex_sup);
	else
	  glColor4ubv((GLubyte *) & CTX.color.mesh.vertex);	
	glVertex3d(v->x(), v->y(), v->z());
      }
      glEnd();
    }
  }
  if(CTX.mesh.points_num) {
    int labelStep = getLabelStep(e->mesh_vertices.size());
    for(unsigned int i = 0; i < e->mesh_vertices.size(); i++)
      if(i % labelStep == 0) drawVertexLabel(e, e->mesh_vertices[i]);
  }
}

template<class T>
static void drawVerticesPerElement(GEntity *e, std::vector<T*> &elements)
{
  for(unsigned int i = 0; i < elements.size(); i++){
    MElement *ele = elements[i];
    for(int j = 0; j < ele->getNumVertices(); j++){
      MVertex *v = ele->getVertex(j);
      if(isElementVisible(ele) && v->getVisibility()){
	if(CTX.mesh.points) {
	  if(v->getPolynomialOrder() > 1)
	    glColor4ubv((GLubyte *) & CTX.color.mesh.vertex_sup);
	  else
	    glColor4ubv((GLubyte *) & CTX.color.mesh.vertex);	
	  if(CTX.mesh.point_type)
	    Draw_Sphere(CTX.mesh.point_size, v->x(), v->y(), v->z(), CTX.mesh.light);
	  else{
	    glBegin(GL_POINTS);
	    glVertex3d(v->x(), v->y(), v->z());
	    glEnd();
	  }
	}
	if(CTX.mesh.points_num)
	  drawVertexLabel(e, v);
      }
    }
  }
}

template<class T>
static void drawBarycentricDual(std::vector<T*> &elements)
{
  glColor4ubv((GLubyte *) & CTX.color.fg);
  glEnable(GL_LINE_STIPPLE);
  glLineStipple(1, 0x0F0F);
  gl2psEnable(GL2PS_LINE_STIPPLE);
  glBegin(GL_LINES);
  for(unsigned int i = 0; i < elements.size(); i++){
    MElement *ele = elements[i];
    if(!isElementVisible(ele)) continue;
    SPoint3 pc = ele->barycenter();
    if(ele->getDim() == 2){
      for(int j = 0; j < ele->getNumEdges(); j++){
	MEdge e = ele->getEdge(j);
	SPoint3 p = e.barycenter();
	glVertex3d(pc.x(), pc.y(), pc.z());
	glVertex3d(p.x(), p.y(), p.z());
      }
    }
    else if(ele->getDim() == 3){
      for(int j = 0; j < ele->getNumFaces(); j++){
	MFace f = ele->getFace(j);
	SPoint3 p = f.barycenter();
	glVertex3d(pc.x(), pc.y(), pc.z());
	glVertex3d(p.x(), p.y(), p.z());
	for(int k = 0; k < f.getNumVertices(); k++){
	  MEdge e(f.getVertex(k), (k == f.getNumVertices() - 1) ? 
		  f.getVertex(0) : f.getVertex(k + 1));
	  SPoint3 pe = e.barycenter();
	  glVertex3d(p.x(), p.y(), p.z());
	  glVertex3d(pe.x(), pe.y(), pe.z());
	}
      }
    }
  }
  glEnd();
  glDisable(GL_LINE_STIPPLE);
  gl2psDisable(GL2PS_LINE_STIPPLE);
}

// Routine to fill the smooth normal container

template<class T>
static void addSmoothNormals(GEntity *e, std::vector<T*> &elements)
{
  for(unsigned int i = 0; i < elements.size(); i++){
    MElement *ele = elements[i];
    for(int j = 0; j < ele->getNumFacesRep(); j++){
      MFace fr = ele->getFaceRep(j);
      SVector3 n = fr.normal();
      SPoint3 pc;
      if(CTX.mesh.explode != 1.) pc = ele->barycenter();
      for(int k = 0; k < fr.getNumVertices(); k++){
	MVertex *v = fr.getVertex(k);
	SPoint3 p(v->x(), v->y(), v->z());
	if(CTX.mesh.explode != 1.){
	  for(int l = 0; l < 3; l++)
	    p[l] = pc[l] + CTX.mesh.explode * (p[l] - pc[l]);
	}
	e->model()->normals->add(p[0], p[1], p[2], n[0], n[1], n[2]);
      }
    }
  }
}

// Routines for filling and drawing the vertex arrays

static void addEdgesInArrays(GEntity *e)
{
  MRep *m = e->meshRep;
  for(MRep::eriter it = m->firstEdgeRep(); it != m->lastEdgeRep(); ++it){
    MVertex *v[2] = {it->first.first, it->first.second};
    MElement *ele = it->second;
    SVector3 n = ele->getFaceRep(0).normal();
    unsigned int color = getColorByElement(ele);
    for(int i = 0; i < 2; i++){
      if(e->dim() == 2 && CTX.mesh.smooth_normals)
	e->model()->normals->get(v[i]->x(), v[i]->y(), v[i]->z(), n[0], n[1], n[2]);
      m->va_lines->add(v[i]->x(), v[i]->y(), v[i]->z(), n[0], n[1], n[2], color, ele);
    }
  }
}

static void addFacesInArrays(GEntity *e)
{
  MRep *m = e->meshRep;
  for(MRep::friter it = m->firstFaceRep(); it != m->lastFaceRep(); ++it){
    MFace f = it->first;
    MElement *ele = it->second;
    SVector3 n = f.normal();
    unsigned int color = getColorByElement(ele);
    for(int i = 0; i < f.getNumVertices(); i++){
      MVertex *v = f.getVertex(i);
      if(CTX.mesh.smooth_normals)
	e->model()->normals->get(v->x(), v->y(), v->z(), n[0], n[1], n[2]);
      if(f.getNumVertices() == 3)
	m->va_triangles->add(v->x(), v->y(), v->z(), n[0], n[1], n[2], color, ele);
      else if(f.getNumVertices() == 4)
	m->va_quads->add(v->x(), v->y(), v->z(), n[0], n[1], n[2], color, ele);
    }
  }
}

template<class T>
static void addElementsInArrays(GEntity *e, std::vector<T*> &elements)
{
  MRep *m = e->meshRep;
  for(unsigned int i = 0; i < elements.size(); i++){
    MElement *ele = elements[i];
    if(!isElementVisible(ele)) continue;
    if(CTX.mesh.use_cut_plane && CTX.mesh.cut_plane_draw_intersect){
      if(e->dim() == 3 && intersectCutPlane(ele)) continue;
    }
    unsigned int color = getColorByElement(ele);
    SPoint3 pc;
    if(CTX.mesh.explode != 1.) pc = ele->barycenter();
    if(ele->getNumFacesRep()){
      for(int j = 0; j < ele->getNumFacesRep(); j++){
	MFace fr = ele->getFaceRep(j);
	SVector3 n = fr.normal();
	int numverts = fr.getNumVertices();
	for(int k = 0; k < numverts; k++){
	  MVertex *v = fr.getVertex(k);
	  SPoint3 p(v->x(), v->y(), v->z());
	  if(CTX.mesh.explode != 1.){
	    for(int l = 0; l < 3; l++)
	      p[l] = pc[l] + CTX.mesh.explode * (p[l] - pc[l]);
	  }
	  if(e->dim() == 2 && CTX.mesh.smooth_normals)
	    e->model()->normals->get(p[0], p[1], p[2], n[0], n[1], n[2]);
	  if(numverts == 3)
	    m->va_triangles->add(p[0], p[1], p[2], n[0], n[1], n[2], color, ele);
	  else if(numverts == 4)
	    m->va_quads->add(p[0], p[1], p[2], n[0], n[1], n[2], color, ele);
	}
      }
    }
    if(!ele->getNumFacesRep() || ele->getPolynomialOrder() > 1){
      SPoint3 pc;
      if(CTX.mesh.explode != 1.) pc = ele->barycenter();
      for(int j = 0; j < ele->getNumEdgesRep(); j++){
	MEdge er = ele->getEdgeRep(j);
	for(int k = 0; k < er.getNumVertices(); k++){
	  MVertex *v = er.getVertex(k);
	  SPoint3 p(v->x(), v->y(), v->z());
	  if(CTX.mesh.explode != 1.){
	    for(int l = 0; l < 3; l++)
	      p[l] = pc[l] + CTX.mesh.explode * (p[l] - pc[l]);
	  }
	  m->va_lines->add(p[0], p[1], p[2], color, ele);
	}
      }
    }
  }
}

static void drawArrays(GEntity *e, VertexArray *va, GLint type, bool useNormalArray, 
		       int forceColor=0, unsigned int color=0, bool drawOutline=false)
{
  if(!va) return;

  // If we want to be enable picking of individual elements we need to
  // draw each one separately
  if(CTX.render_mode == GMSH_SELECT && CTX.pick_elements) {
    if(va->getNumElementPointers() == va->getNumVertices()){
      for(int i = 0; i < va->getNumVertices(); i += va->getNumVerticesPerElement()){
	glPushName(va->getNumVerticesPerElement());
	glPushName(i);
	glBegin(type);
	for(int j = 0; j < va->getNumVerticesPerElement(); j++)
	  glVertex3fv(va->getVertexArray(3 * (i + j)));
	glEnd();
	glPopName();
	glPopName();
      }
      return;
    }
  }

  glVertexPointer(3, GL_FLOAT, 0, va->getVertexArray());
  glNormalPointer(GL_BYTE, 0, va->getNormalArray());
  glColorPointer(4, GL_UNSIGNED_BYTE, 0, va->getColorArray());
  
  glEnableClientState(GL_VERTEX_ARRAY);

  if(useNormalArray){
    glEnable(GL_LIGHTING);
    glEnableClientState(GL_NORMAL_ARRAY);
  }
  else
    glDisableClientState(GL_NORMAL_ARRAY);

  if(forceColor){
    glDisableClientState(GL_COLOR_ARRAY);
    glColor4ubv((GLubyte *) & color);
  }
  else if(CTX.pick_elements){
    glEnableClientState(GL_COLOR_ARRAY);
  }
  else if(!e->getSelection() && (CTX.mesh.color_carousel == 0 || 
				 CTX.mesh.color_carousel == 3)){
    glEnableClientState(GL_COLOR_ARRAY);
  }
  else{
    glDisableClientState(GL_COLOR_ARRAY);
    color = getColorByEntity(e);
    glColor4ubv((GLubyte *) & color);
  }
  
  if(va->getNumVerticesPerElement() > 2 && !drawOutline && CTX.polygon_offset)
    glEnable(GL_POLYGON_OFFSET_FILL);
  
  if(drawOutline) 
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  
  glDrawArrays(type, 0, va->getNumVertices());
  
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  glDisable(GL_POLYGON_OFFSET_FILL);
  glDisable(GL_LIGHTING);
  
  glDisableClientState(GL_VERTEX_ARRAY);
  glDisableClientState(GL_NORMAL_ARRAY);
  glDisableClientState(GL_COLOR_ARRAY);
}

// GVertex drawing routines

class drawMeshGVertex {
 public:
  void operator () (GVertex *v)
  {  
    if(!v->getVisibility()) return;
    
    if(CTX.render_mode == GMSH_SELECT) {
      glPushName(0);
      glPushName(v->tag());
    }

    if(CTX.mesh.points || CTX.mesh.points_num)
      drawVerticesPerEntity(v);

    if(CTX.render_mode == GMSH_SELECT) {
      glPopName();
      glPopName();
    }
  }
};

// GEdge drawing routines

class initMeshGEdge {
 public :
  void operator () (GEdge *e)
  {
    if(!e->getVisibility()) return;

    if(!e->meshRep) e->meshRep = new MRepEdge(e);

    MRep *m = e->meshRep;
    m->resetArrays();
    m->allElementsVisible = areAllElementsVisible(e->lines);

    if(CTX.mesh.lines){
      m->va_lines = new VertexArray(2, e->lines.size());
      addElementsInArrays(e, e->lines);
    }
  }
};

class drawMeshGEdge {
 public:
  void operator () (GEdge *e)
  {  
    if(!e->getVisibility()) return;
    
    MRep *m = e->meshRep;

    if(!m) return;

    if(CTX.render_mode == GMSH_SELECT) {
      glPushName(1);
      glPushName(e->tag());
    }

    if(CTX.mesh.lines)
      drawArrays(e, m->va_lines, GL_LINES, false);

    if(CTX.mesh.lines_num)
      drawElementLabels(e, e->lines);

    if(CTX.mesh.points || CTX.mesh.points_num){
      if(m->allElementsVisible)
	drawVerticesPerEntity(e);
      else
	drawVerticesPerElement(e, e->lines);
    }

    if(CTX.mesh.tangents)
      drawTangents(e->lines);

    if(CTX.render_mode == GMSH_SELECT) {
      glPopName();
      glPopName();
    }
  }
};

// GFace drawing routines

class initSmoothNormalsGFace {
 public :
  void operator () (GFace *f)
  {
    addSmoothNormals(f, f->triangles);
    addSmoothNormals(f, f->quadrangles);
  }
};

class initMeshGFace {
 public :
  void operator () (GFace *f)
  {
    if(!f->getVisibility()) return;

    if(!f->meshRep) f->meshRep = new MRepFace(f);

    MRep *m = f->meshRep;
    m->resetArrays();
    m->allElementsVisible = 
      CTX.mesh.triangles && areAllElementsVisible(f->triangles) && 
      CTX.mesh.quadrangles && areAllElementsVisible(f->quadrangles);

    bool curvedElements = 
      areSomeElementsCurved(f->triangles) || 
      areSomeElementsCurved(f->quadrangles);

    bool useEdges = CTX.mesh.surfaces_edges ? true : false;
    if(CTX.mesh.surfaces_faces || CTX.mesh.explode != 1. || 
       CTX.pick_elements || !m->allElementsVisible)
      useEdges = false; // cannot use edges in these cases

    // Further optimizations are possible when useEdges is true:
    // 1) store the unique vertices in the vertex array and use
    //    glDrawElements() instead of glDrawArrays(). Question:
    //    which normal do you choose for each vertex, and so how 
    //    can you achieve accurate "flat shading" rendering?
    // 2) use tc to stripe the triangles and draw strips instead of 
    //    individual triangles
    
    if(useEdges){
      Msg(DEBUG, "Using edges to draw surface %d", f->tag());
      m->generateEdgeRep();
      m->va_lines = new VertexArray(2, m->getNumEdgeRep());
      addEdgesInArrays(f);
    }
    else if(CTX.mesh.surfaces_edges || CTX.mesh.surfaces_faces){
      if(curvedElements) // cannot simply draw polygon outlines!
	m->va_lines = new VertexArray(2, 6 * f->triangles.size() 
				      + 8 * f->quadrangles.size());
      m->va_triangles = new VertexArray(3, f->triangles.size());
      m->va_quads = new VertexArray(4, f->quadrangles.size());
      if(CTX.mesh.triangles) addElementsInArrays(f, f->triangles);
      if(CTX.mesh.quadrangles) addElementsInArrays(f, f->quadrangles);
    }
  }
};

class drawMeshGFace {
 public:
  void operator () (GFace *f)
  {  
    if(!f->getVisibility()) return;

    MRep *m = f->meshRep;

    if(!m) return;

    if(CTX.render_mode == GMSH_SELECT) {
      glPushName(2);
      glPushName(f->tag());
    }

    if(CTX.mesh.surfaces_edges){
      if(m->va_lines && m->va_lines->getNumVertices()){
	drawArrays(f, m->va_lines, GL_LINES, CTX.mesh.light && CTX.mesh.light_lines, 
		   CTX.mesh.surfaces_faces, CTX.color.mesh.line);
      }
      else{
	drawArrays(f, m->va_triangles, GL_TRIANGLES, CTX.mesh.light && CTX.mesh.light_lines,
		   CTX.mesh.surfaces_faces, CTX.color.mesh.line, true);
	drawArrays(f, m->va_quads, GL_QUADS, CTX.mesh.light && CTX.mesh.light_lines, 
		   CTX.mesh.surfaces_faces, CTX.color.mesh.line, true);
      }
    }
    
    if(CTX.mesh.surfaces_faces){
      drawArrays(f, m->va_triangles, GL_TRIANGLES, CTX.mesh.light);
      drawArrays(f, m->va_quads, GL_QUADS, CTX.mesh.light);
    }

    if(CTX.mesh.surfaces_num) {
      if(CTX.mesh.triangles)
	drawElementLabels(f, f->triangles, CTX.mesh.surfaces_faces, CTX.color.mesh.line);
      if(CTX.mesh.quadrangles)
	drawElementLabels(f, f->quadrangles, CTX.mesh.surfaces_faces, CTX.color.mesh.line);
    }

    if(CTX.mesh.points || CTX.mesh.points_num){
      if(m->allElementsVisible)
	drawVerticesPerEntity(f);
      else{
	if(CTX.mesh.triangles) drawVerticesPerElement(f, f->triangles);
	if(CTX.mesh.quadrangles) drawVerticesPerElement(f, f->quadrangles);
      }
    }

    if(CTX.mesh.normals) {
      if(CTX.mesh.triangles) drawNormals(f->triangles);
      if(CTX.mesh.quadrangles) drawNormals(f->quadrangles);
    }

    if(CTX.mesh.dual) {
      if(CTX.mesh.triangles) drawBarycentricDual(f->triangles);
      if(CTX.mesh.quadrangles) drawBarycentricDual(f->quadrangles);
    }

    if(CTX.render_mode == GMSH_SELECT) {
      glPopName();
      glPopName();
    }
  }
};

// GRegion drawing routines

class initMeshGRegion {
 public :
  void operator () (GRegion *r)
  {  
    if(!r->getVisibility()) return;

    if(!r->meshRep) r->meshRep = new MRepRegion(r);

    MRep *m = r->meshRep;
    m->resetArrays();
    m->allElementsVisible = 
      CTX.mesh.tetrahedra && areAllElementsVisible(r->tetrahedra) &&
      CTX.mesh.hexahedra && areAllElementsVisible(r->hexahedra) &&
      CTX.mesh.prisms && areAllElementsVisible(r->prisms) &&
      CTX.mesh.pyramids && areAllElementsVisible(r->pyramids);

    bool useEdges = CTX.mesh.volumes_edges ? true : false;
    if(CTX.mesh.volumes_faces || CTX.mesh.explode != 1. || 
       CTX.pick_elements || !m->allElementsVisible)
      useEdges = false; // cannot use edges in these cases

    bool curvedElements = 
      areSomeElementsCurved(r->tetrahedra) || 
      areSomeElementsCurved(r->hexahedra) ||
      areSomeElementsCurved(r->prisms) ||
      areSomeElementsCurved(r->pyramids);

    bool useSkin = CTX.mesh.volumes_faces ? true : false;
    if(CTX.mesh.explode != 1. || !m->allElementsVisible)
      useSkin = false;
    useSkin = false; // FIXME: to do
    
    if(useSkin){ 
      Msg(DEBUG, "Using boundary faces to draw volume %d", r->tag());
      m->generateBoundaryFaceRep(); 
      m->va_triangles = new VertexArray(3, m->getNumFaceRep());
      m->va_quads = new VertexArray(4, m->getNumFaceRep());
      addFacesInArrays(r);
    }
    else if(useEdges){
      Msg(DEBUG, "Using edges to draw volume %d", r->tag());
      m->generateEdgeRep();
      m->va_lines = new VertexArray(2, m->getNumEdgeRep());
      addEdgesInArrays(r);
    }
    else if(CTX.mesh.volumes_edges || CTX.mesh.volumes_faces){
      if(curvedElements) // cannot simply draw polygon outlines!
	m->va_lines = new VertexArray(2, 12 * r->tetrahedra.size() +
				      24 * r->hexahedra.size() +
				      18 * r->prisms.size() +
				      16 * r->pyramids.size());
      m->va_triangles = new VertexArray(3, 4 * r->tetrahedra.size() +
					2 * r->prisms.size() + 
					4 * r->pyramids.size());
      m->va_quads = new VertexArray(4, 6 * r->hexahedra.size() +
				    3 * r->prisms.size() +
				    r->pyramids.size());
      if(CTX.mesh.tetrahedra) addElementsInArrays(r, r->tetrahedra);
      if(CTX.mesh.hexahedra) addElementsInArrays(r, r->hexahedra);
      if(CTX.mesh.prisms) addElementsInArrays(r, r->prisms);
      if(CTX.mesh.pyramids) addElementsInArrays(r, r->pyramids);
    }
  }
};

class drawMeshGRegion {
 public :
  void operator () (GRegion *r)
  {  
    if(!r->getVisibility()) return;

    MRep *m = r->meshRep;

    if(!m) return;

    if(CTX.render_mode == GMSH_SELECT) {
      glPushName(3);
      glPushName(r->tag());
    }

    if(CTX.mesh.volumes_edges){
      if(m->va_lines && m->va_lines->getNumVertices()){
	drawArrays(r, m->va_lines, GL_LINES, CTX.mesh.light && CTX.mesh.light_lines, 
		   CTX.mesh.volumes_faces, CTX.color.mesh.line);
      }
      else{
	drawArrays(r, m->va_triangles, GL_TRIANGLES, CTX.mesh.light && CTX.mesh.light_lines,
		   CTX.mesh.volumes_faces, CTX.color.mesh.line, true);
	drawArrays(r, m->va_quads, GL_QUADS, CTX.mesh.light && CTX.mesh.light_lines, 
		   CTX.mesh.volumes_faces, CTX.color.mesh.line, true);
      }
    }
    
    if(CTX.mesh.volumes_faces){
      drawArrays(r, m->va_triangles, GL_TRIANGLES, CTX.mesh.light);
      drawArrays(r, m->va_quads, GL_QUADS, CTX.mesh.light);
    }
    
    if(CTX.mesh.volumes_num) {
      if(CTX.mesh.tetrahedra) 
	drawElementLabels(r, r->tetrahedra, CTX.mesh.volumes_faces || 
			  CTX.mesh.surfaces_faces, CTX.color.mesh.line);
      if(CTX.mesh.hexahedra) 
	drawElementLabels(r, r->hexahedra, CTX.mesh.volumes_faces || 
			  CTX.mesh.surfaces_faces, CTX.color.mesh.line);
      if(CTX.mesh.prisms) 
	drawElementLabels(r, r->prisms, CTX.mesh.volumes_faces || 
			  CTX.mesh.surfaces_faces, CTX.color.mesh.line);
      if(CTX.mesh.pyramids) 
	drawElementLabels(r, r->pyramids, CTX.mesh.volumes_faces ||
			  CTX.mesh.surfaces_faces, CTX.color.mesh.line);
    }

    if(CTX.mesh.points || CTX.mesh.points_num){
      if(m->allElementsVisible)
	drawVerticesPerEntity(r);
      else{
	if(CTX.mesh.tetrahedra) drawVerticesPerElement(r, r->tetrahedra);
	if(CTX.mesh.hexahedra) drawVerticesPerElement(r, r->hexahedra);
	if(CTX.mesh.prisms) drawVerticesPerElement(r, r->prisms);
	if(CTX.mesh.pyramids) drawVerticesPerElement(r, r->pyramids);
      }
    }

    if(CTX.mesh.dual) {
      if(CTX.mesh.tetrahedra) drawBarycentricDual(r->tetrahedra);
      if(CTX.mesh.hexahedra) drawBarycentricDual(r->hexahedra);
      if(CTX.mesh.prisms) drawBarycentricDual(r->prisms);
      if(CTX.mesh.pyramids) drawBarycentricDual(r->pyramids);
    }

    if(CTX.render_mode == GMSH_SELECT) {
      glPopName();
      glPopName();
    }
  }
};


// Main drawing routine

void Draw_Mesh()
{
  if(!CTX.mesh.draw) return;
  
  glPointSize(CTX.mesh.point_size);
  gl2psPointSize(CTX.mesh.point_size * CTX.print.eps_point_size_factor);

  glLineWidth(CTX.mesh.line_width);
  gl2psLineWidth(CTX.mesh.line_width * CTX.print.eps_line_width_factor);
  
  if(CTX.mesh.light_two_side)
    glLightModelf(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
  else
    glLightModelf(GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE);
  
  for(int i = 0; i < 6; i++)
    if(CTX.clip[i] & 2) 
      glEnable((GLenum)(GL_CLIP_PLANE0 + i));
    else
      glDisable((GLenum)(GL_CLIP_PLANE0 + i));
  
  if(!CTX.threads_lock){
    CTX.threads_lock = 1; 
    GModel *m = GModel::current();
    int status = m->getMeshStatus();
    if(CTX.mesh.changed) {
      Msg(DEBUG, "Mesh has changed: reinitializing drawing data", CTX.mesh.changed);
      if(status >= 1 && CTX.mesh.changed & ENT_LINE)
	std::for_each(m->firstEdge(), m->lastEdge(), initMeshGEdge());
      if(status >= 2 && CTX.mesh.changed & ENT_SURFACE){
	if(m->normals) delete m->normals;
	m->normals = new smooth_normals(CTX.mesh.angle_smooth_normals);
	if(CTX.mesh.smooth_normals)
	  std::for_each(m->firstFace(), m->lastFace(), initSmoothNormalsGFace());
	std::for_each(m->firstFace(), m->lastFace(), initMeshGFace());
      }
      if(status >= 3 && CTX.mesh.changed & ENT_VOLUME)
	std::for_each(m->firstRegion(), m->lastRegion(), initMeshGRegion());
    }
    if(status >= 0)
      std::for_each(m->firstVertex(), m->lastVertex(), drawMeshGVertex());
    if(status >= 1)
      std::for_each(m->firstEdge(), m->lastEdge(), drawMeshGEdge());
    if(status >= 2)
      std::for_each(m->firstFace(), m->lastFace(), drawMeshGFace());
    if(status >= 3)
      std::for_each(m->firstRegion(), m->lastRegion(), drawMeshGRegion());
    CTX.mesh.changed = 0;
    CTX.threads_lock = 0;
  }

  for(int i = 0; i < 6; i++)
    glDisable((GLenum)(GL_CLIP_PLANE0 + i));
}
