// $Id: Opengl.cpp,v 1.46 2004-12-28 23:59:48 geuzaine Exp $
//
// Copyright (C) 1997-2004 C. Geuzaine, J.-F. Remacle
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
#include "Numeric.h"
#include "Context.h"
#include "Geo.h"
#include "Mesh.h"
#include "Draw.h"
#include "GUI.h"
#include "gl2ps.h"

extern GUI *WID;
extern Mesh M;
extern Context_T CTX;

void Process_SelectionBuffer(int x, int y, int *n, GLuint * ii, GLuint * jj);
void Filter_SelectionBuffer(int n, GLuint * typ, GLuint * ient,
                            Vertex ** thev, Curve ** thec, Surface ** thes,
                            Mesh * m);
void myZoom(GLdouble X1, GLdouble X2, GLdouble Y1, GLdouble Y2, GLdouble Xc1,
            GLdouble Xc2, GLdouble Yc1, GLdouble Yc2);

// Draw specialization

void InitOpengl(void)
{
  WID->make_opengl_current();
  Orthogonalize(0, 0);
}

void SetOpenglContext(void)
{
  WID->make_opengl_current();
}

void ClearOpengl(void)
{
  glClearColor(UNPACK_RED(CTX.color.bg) / 255.,
               UNPACK_GREEN(CTX.color.bg) / 255.,
               UNPACK_BLUE(CTX.color.bg) / 255., 0.);
  glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
}

void Draw(void)
{
  WID->redraw_opengl();
}

void Draw_String(char *s)
{
  if(CTX.print.gl_fonts) {
    gl_font(CTX.gl_font_enum, CTX.gl_fontsize);
    gl_draw(s);
  }
  else {        // ps or *tex output
    if(CTX.print.format == FORMAT_JPEGTEX ||
       CTX.print.format == FORMAT_PNGTEX)
      return;
    gl2psText(s, CTX.gl_font, CTX.gl_fontsize);
  }
}

void Draw_String(char *s, double style)
{
  unsigned int bits = (unsigned int)style;

  if(!bits){ // use defaults
    Draw_String(s);
    return;
  }

  int size = (bits & 0xff);
  int font = (bits>>8 & 0xff);
  int align = (bits>>16 & 0xff);
  
  int oldsize = CTX.gl_fontsize;
  int oldfont_enum = CTX.gl_font_enum;
  char * oldfont = CTX.gl_font;
  
  CTX.gl_font_enum = GetFontEnum(font);
  CTX.gl_font = GetFontName(font);
  if(size)
    CTX.gl_fontsize = size;
  if(align > 0){
    GLboolean valid;
    glGetBooleanv(GL_CURRENT_RASTER_POSITION_VALID, &valid);
    if(valid == GL_TRUE){
      GLfloat pos[4];
      glGetFloatv(GL_CURRENT_RASTER_POSITION, pos);
      gl_font(CTX.gl_font_enum, CTX.gl_fontsize);
      float width = gl_width(s);
      if(align == 1) // center
	glRasterPos2d(pos[0]-width/2., pos[1]);
      else if(align == 2) // right
	glRasterPos2d(pos[0]-width, pos[1]);
    }
  }

  Draw_String(s);
  
  CTX.gl_font_enum = oldfont_enum;
  CTX.gl_font = oldfont;
  if(size)
    CTX.gl_fontsize = oldsize;
}

void Draw_OnScreenMessages()
{

  glColor4ubv((GLubyte *) & CTX.color.text);
  gl_font(CTX.gl_font_enum, CTX.gl_fontsize);
  double h = gl_height();
  
  if(strlen(WID->onscreen_buffer[0])){
    double w = gl_width(WID->onscreen_buffer[0]);
    glRasterPos2d(CTX.viewport[2]/2.-w/2., CTX.viewport[3] - 1.2*h);
    gl_draw(WID->onscreen_buffer[0]);
  }
  if(strlen(WID->onscreen_buffer[1])){
    double w = gl_width(WID->onscreen_buffer[1]);
    glRasterPos2d(CTX.viewport[2]/2.-w/2., CTX.viewport[3] - 2.4*h);
    gl_draw(WID->onscreen_buffer[1]);
  }
}

// Euler angles set_XXX

void set_r(int i, double val)
{
  if(!CTX.useTrackball) {
    if(!CTX.rlock[i]) {
      CTX.r[i] = val;
    }
  }
}

void set_t(int i, double val)
{
  if(!CTX.tlock[i]) {
    CTX.t[i] = val;
  }
}

void set_s(int i, double val)
{
  if(!CTX.slock[i]) {
    CTX.s[i] = val;
  }
}

// Select entity routines

int check_type(int type, Vertex * v, Curve * c, Surface * s)
{
  return ((type == ENT_POINT && v) ||
          (type == ENT_LINE && c) || 
	  (type == ENT_SURFACE && s));
}

char SelectEntity(int type, Vertex ** v, Curve ** c, Surface ** s)
{
  int hits;
  GLuint ii[SELECTION_BUFFER_SIZE], jj[SELECTION_BUFFER_SIZE];

  WID->selection = type;
  WID->try_selection = 0;
  WID->quit_selection = 0;
  WID->end_selection = 0;
  WID->undo_selection = 0;

  while(1) {
    *v = NULL;
    *c = NULL;
    *s = NULL;

    WID->wait();
    if(WID->quit_selection) {
      WID->quit_selection = 0;
      WID->selection = 0;
      return 'q';
    }
    if(WID->end_selection) {
      WID->end_selection = 0;
      WID->selection = 0;
      return 'e';
    }
    if(WID->undo_selection) {
      WID->undo_selection = 0;
      return 'u';
    }
    if(WID->try_selection) {
      WID->try_selection = 0;
      if(type == ENT_NONE){ // just report the mouse click
	return 'c';
      }
      else{
	Process_SelectionBuffer(Fl::event_x(), Fl::event_y(), &hits, ii, jj);
	Filter_SelectionBuffer(hits, ii, jj, v, c, s, &M);
	if(check_type(type, *v, *c, *s)) {
	  HighlightEntity(*v, *c, *s, 1);
	  WID->selection = 0;
	  return 'l';
	}
      }
    }
  }

}
