// $Id: Axes.cpp,v 1.5 2001-08-11 23:28:31 geuzaine Exp $

#include "Gmsh.h"
#include "GmshUI.h"
#include "Numeric.h"
#include "Mesh.h"
#include "Draw.h"
#include "Context.h"
#include "gl2ps.h"

extern Context_T   CTX;


void Draw_Axes (double s) {
  double  f, g, b, c;
  
  if(s == 0.) return;

  if(!CTX.range[0] && !CTX.range[1] && !CTX.range[2]) return ;

  f = 0.666 * s;
  g = 1.233 * s;
  b = .1 * s;
  c = 0.666 * b;

  glLineWidth(1.); gl2psLineWidth(1.);
  glColor4ubv((GLubyte*)&CTX.color.axes);

  glBegin(GL_LINES);
  if(CTX.range[2] != 0.){
    /* X */
    glVertex3d(0.,   0.,   0.);  
    glVertex3d(s,    0.,   0.);  
    glVertex3d(g-b,  b,    0.);  
    glVertex3d(g+b, -b,    0.);  
    glVertex3d(g,   -b,    b);  
    glVertex3d(g,    b,   -b);  
    /* Y */
    glVertex3d(0.,   0.,   0.);  
    glVertex3d(0.,   s,    0.);  
    glVertex3d(-b,   g+b,  0.);  
    glVertex3d(0.,   g,    0.);  
    glVertex3d(0.,   g,    0.);  
    glVertex3d(0.,   g+b, -b);  
    glVertex3d(0.,   g,    0.);  
    glVertex3d(.5*b, g-b, .5*b);  
    /* Z */
    glVertex3d(0.,   0.,   0.);  
    glVertex3d(0.,   0.,   s);  
    glVertex3d(-b,   b,    g);  
    glVertex3d(0.,   b,    g-b);  
    glVertex3d(0.,   b,    g-b);  
    glVertex3d(0.,  -b,    g+b);  
    glVertex3d(0.,  -b,    g+b);  
    glVertex3d(b,   -b,    g);  
  }
  else{
    /* X */
    glVertex3d(0.,   0.,   0.);  
    glVertex3d(s,    0.,   0.);  
    glVertex3d(g-c,  b,    0.);  
    glVertex3d(g+c, -b,    0.);  
    glVertex3d(g-c, -b,    0.);  
    glVertex3d(g+c,  b,    0.);  
    /* Y */
    glVertex3d(0.,   0.,   0.);  
    glVertex3d(0.,   s,    0.);  
    glVertex3d(-c,   g+b,  0.);  
    glVertex3d(0.,   g,    0.);  
    glVertex3d(0.,   g,    0.);  
    glVertex3d(c,    g+b,  0.);  
    glVertex3d(0.,   g,    0.);  
    glVertex3d(0.,   g-b,  0.);
  }
  glEnd();
  
  glEnable(GL_LINE_STIPPLE);
  glLineStipple(2,0x0F0F);
  glBegin(GL_LINES);
  if(CTX.range[2] != 0.){
    glVertex3d(f,  0., 0.);  
    glVertex3d(f,  0., f );  
    glVertex3d(f,  0., f );  
    glVertex3d(0., 0., f );  
    glVertex3d(0., 0., f );  
    glVertex3d(0., f,  f );  
    glVertex3d(0., f,  f );  
    glVertex3d(0., f,  0.);  
  }
  glVertex3d(0., f,  0.);  
  glVertex3d(f,  f,  0.);  
  glVertex3d(f,  f,  0.);  
  glVertex3d(f,  0., 0.);  
  glEnd();
  glDisable(GL_LINE_STIPPLE);

}

void Draw_SmallAxes(void){
  double l,o,xx,xy,yx,yy,zx,zy,cx,cy;

  l  = 30  ;
  o  = 2  ;
  cx = CTX.viewport[2] - 45;
  cy = CTX.viewport[1] + 35;

  xx = l*CTX.rot[0][0] ; xy = l*CTX.rot[0][1] ;
  yx = l*CTX.rot[1][0] ; yy = l*CTX.rot[1][1] ;
  zx = l*CTX.rot[2][0] ; zy = l*CTX.rot[2][1] ;

  glColor4ubv((GLubyte*)&CTX.color.small_axes);

  glBegin(GL_LINES);
  glVertex2d(cx,cy); glVertex2d(cx+xx,cy+xy);
  glVertex2d(cx,cy); glVertex2d(cx+yx,cy+yy);
  glVertex2d(cx,cy); glVertex2d(cx+zx,cy+zy);  
  glEnd();
  glRasterPos2d(cx+xx+o,cy+xy+o); Draw_String("X");
  glRasterPos2d(cx+yx+o,cy+yy+o); Draw_String("Y");
  glRasterPos2d(cx+zx+o,cy+zy+o); Draw_String("Z");

}

