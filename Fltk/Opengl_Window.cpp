// $Id: Opengl_Window.cpp,v 1.9 2001-02-03 13:10:26 geuzaine Exp $

#include "Gmsh.h"
#include "GmshUI.h"
#include "Context.h"
#include "Const.h"
#include "Geo.h"
#include "Mesh.h"
#include "Draw.h"
#include "GUI.h"
#include "Opengl_Window.h"

// This file defines the Opengl_Window class (subclass of Fl_GL_Window)

extern GUI *WID;
extern Mesh M;
extern Context_T CTX;

void Process_SelectionBuffer(int x, int y, int *n, GLuint *ii, GLuint *jj);
void Filter_SelectionBuffer(int n, GLuint *typ, GLuint *ient, Vertex **thev,
                            Curve **thec, Surface **thes, Mesh *m);
void myZoom(GLdouble X1, GLdouble X2, GLdouble Y1, GLdouble Y2,
            GLdouble Xc1, GLdouble Xc2, GLdouble Yc1, GLdouble Yc2);

void Opengl_Window::draw() {
  if(!valid()){
    valid(1);
    CTX.viewport[0] = 0 ; CTX.viewport[1] = 0 ;
    CTX.viewport[2] = w() ; CTX.viewport[3] = h() ;
    glViewport(CTX.viewport[0], CTX.viewport[1],
	       CTX.viewport[2], CTX.viewport[3]);
  }
  if((w() != CTX.viewport[2]-CTX.viewport[0]) ||
     (h() != CTX.viewport[3]-CTX.viewport[1])){
    WID->set_size(CTX.viewport[2]-CTX.viewport[0],
		  CTX.viewport[3]-CTX.viewport[1]);
    glViewport(CTX.viewport[0], CTX.viewport[1],
	       CTX.viewport[2], CTX.viewport[3]);
  }
  Orthogonalize(0,0);
  glClearColor(UNPACK_RED(CTX.color.bg)/255.,
               UNPACK_GREEN(CTX.color.bg)/255.,
               UNPACK_BLUE(CTX.color.bg)/255.,
               0.);
  glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);
  Draw3d();
  Draw2d();
}

static double O1,O2,O3,O4;

void Opengl_Window::draw_overlay() {
  /*
  if(!valid()){
    valid(1);
    CTX.viewport[0] = 0 ; CTX.viewport[1] = 0 ;
    CTX.viewport[2] = w() ; CTX.viewport[3] = h() ;
    glViewport(CTX.viewport[0], CTX.viewport[1],
	       CTX.viewport[2], CTX.viewport[3]);
  }
  Orthogonalize(0,0);
  glClearIndex(0);
  glClear(GL_COLOR_BUFFER_BIT);  
  glLineWidth(1.);
  glClearIndex(0);
  glClear(GL_COLOR_BUFFER_BIT);  
  glIndexi((CTX.color.bg<CTX.color.fg)?FL_WHITE:FL_BLACK);
  glBegin(GL_LINE_STRIP);
  glVertex2d(O1, O2);
  glVertex2d(O1+O3, O2);
  glVertex2d(O1+O3, O2+O4);
  glVertex2d(O1, O2+O4);
  glVertex2d(O1, O2);
  glEnd();
  */
}

void Opengl_Window::clear_overlay() {
}


// le principe de FLTK est assez different des autres toolkits: les
// events sont passes au handle du widget qui a le focus. Si ce handle
// revoie 1, alors l'event est considere comme traite, et est
// supprime. Si le handle retourne 0, l'event est passe au handle du
// parent.

int Opengl_Window::handle(int event) {
  static int      xpos, ypos, xmov, ymov, ibut, hits;
  static int      ZoomClick=0, FirstClick=0;
  static GLdouble xc1, yc1, xc2, yc2, xt1, yt1, xscale1, yscale1;
  static GLdouble xb, yb, xc, yc, xe, ye, xz, yz;
  static GLdouble movzx, movzy;
  static Vertex   *v=NULL, *ov;
  static Curve    *c=NULL, *oc;
  static Surface  *s=NULL, *os;

  GLuint  ii[SELECTION_BUFFER_SIZE], jj[SELECTION_BUFFER_SIZE];

  switch (event) {

  case FL_PUSH:
    FirstClick = 1;
    ibut = Fl::event_button();
    xpos = Fl::event_x();
    ypos = Fl::event_y();

    if(ibut == 1 && !Fl::event_state(FL_SHIFT)){
      if(!ZoomClick && Fl::event_state(FL_CTRL)){
        xb = CTX.vxmin + ((double)xpos/(double)w()) * (CTX.vxmax - CTX.vxmin);
        yb = CTX.vymax - ((double)ypos/(double)h()) * (CTX.vymax - CTX.vymin);
        xc1 = xb/CTX.s[0] - CTX.t[0];
        yc1 = yb/CTX.s[1] - CTX.t[1];
        ZoomClick = 1;
      }
      else if(ZoomClick){
        xe = CTX.vxmin + ((double)xpos/(double)w()) * (CTX.vxmax - CTX.vxmin);
        ye = CTX.vymax - ((double)ypos/(double)h()) * (CTX.vymax - CTX.vymin);
        xc2 = xe/CTX.s[0] - CTX.t[0];
        yc2 = ye/CTX.s[1] - CTX.t[1];     
        ZoomClick = 0;
        clear_overlay();
        if(xb!=xe && yb!=ye) myZoom(xb,xe,yb,ye,xc1,xc2,yc1,yc2);
      }
      else{
	WID->try_selection = 1 ;
      }
    }
    else if(ibut == 2 || (ibut == 1 && Fl::event_state(FL_SHIFT))){
      if(Fl::event_state(FL_CTRL) && !ZoomClick){
        set_s(1, CTX.s[0]);
        set_s(2, CTX.s[0]);
        redraw();
      }
      else{
        ZoomClick = 0;
        clear_overlay();
      }
    }
    else{
      if(Fl::event_state(FL_CTRL) && !ZoomClick){
	if(CTX.useTrackball)
	  CTX.setQuaternion(0.,0.,0.,1.);
	else{
	  set_r(0,0.); set_r(1,0.); set_r(2,0.); 
	}
        set_t(0,0.); set_t(1,0.); set_t(2,0.);
        set_s(0,1.); set_s(1,1.); set_s(2,1.);
        redraw();
      }
      else{
        ZoomClick = 0;
        clear_overlay();
      }
    }
    return 1;

  case FL_RELEASE:
    ibut = Fl::event_button();
    xpos = Fl::event_x();
    ypos = Fl::event_y();
    if(!ZoomClick){
      CTX.mesh.draw = 1 ;
      CTX.post.draw = 1 ;
      redraw();
    }
    return 1;

  case FL_DRAG:
    xmov = Fl::event_x() - xpos;
    ymov = Fl::event_y() - ypos;

    if(ZoomClick) {
      printf("should draw the zoom... %d %d %d %d\n", xpos, ypos, xmov, ymov);
      xz = CTX.vxmin + ((double)Fl::event_x()/(double)w()) * (CTX.vxmax - CTX.vxmin);
      yz = CTX.vymax - ((double)Fl::event_y()/(double)h()) * (CTX.vymax - CTX.vymin);
      movzx = xz - xb; movzy = yz - yb;
      O1 = xb;
      O2 = yb;
      O3 = movzx;
      O4 = movzx;
      redraw_overlay();
    }
    else {
      clear_overlay();

      if(FirstClick){
	xc1 = (((double)xpos/(double)w()) * (CTX.vxmax-CTX.vxmin) + CTX.vxmin)
	  / CTX.s[0] - CTX.t[0];
	yc1 = (CTX.vymax - ((double)ypos/(double)h()) * (CTX.vymax-CTX.vymin))
	  / CTX.s[1] - CTX.t[1];
	xt1 = CTX.t[0];
	yt1 = CTX.t[1];
	xscale1 = CTX.s[0];
	yscale1 = CTX.s[1];
	FirstClick=0;
      }

      if(ibut == 1 && !Fl::event_state(FL_SHIFT)){
	if(CTX.useTrackball)
	  CTX.addQuaternion((2.0*xpos - w()) / w(),
			    (h() - 2.0*ypos) / h(),
			    (2.0*Fl::event_x() - w()) / w(),
			    (h() - 2.0*Fl::event_y()) / h());
	else{
	  set_r(1, CTX.r[1] + ((abs(xmov) > abs(ymov))?180*(float)xmov/(float)w():0));
	  set_r(0, CTX.r[0] + ((abs(xmov) > abs(ymov))?0:180*(float)ymov/(float)h()));
	}
      }
      else if(ibut == 2 || (ibut == 1 && Fl::event_state(FL_SHIFT))){
	if(!CTX.useTrackball)
	  set_r(2, CTX.r[2] + ((abs(ymov) > abs(xmov))?0:-180*(float)xmov/(float)w()));         
	set_s(0, CTX.s[0] * ( (abs(ymov) > abs(xmov)) ?
			      ( (ymov>0) ? (float)(1.04*(abs(ymov)+h()))/(float)h()
				: (float)(h())/(float)(1.04*(abs(ymov)+h())) )
			      : 1.) );                    
	set_s(1, CTX.s[0]);
	set_s(2, CTX.s[0]);
	if(abs(ymov) > abs(xmov)){
	  set_t(0, xt1*(xscale1/CTX.s[0])-xc1*(1.-(xscale1/CTX.s[0])));
	  set_t(1, yt1*(yscale1/CTX.s[1])-yc1*(1.-(yscale1/CTX.s[1])));
	}
      }
      else{
	xc = ( ((double)xpos/(double)w()) * (CTX.vxmax - CTX.vxmin) + CTX.vxmin )
	  / CTX.s[0];
	yc = ( CTX.vymax - ((double)ypos/(double)h()) * (CTX.vymax - CTX.vymin))
	  / CTX.s[1];
	set_t(0, xc-xc1);
	set_t(1, yc-yc1);
	set_t(2, 0.);
      }

      if(CTX.fast){
	CTX.mesh.draw = 0 ;
	CTX.post.draw = 0;
      }
      
      redraw();

    }
    xpos += xmov; 
    ypos += ymov; 
    return 1;
    

  case FL_MOVE:
    xmov = Fl::event_x()-xpos;
    ymov = Fl::event_y()-ypos;
    WID->make_opengl_current();
    Process_SelectionBuffer(Fl::event_x(), Fl::event_y(), &hits, ii, jj);
    ov = v; oc = c; os = s; v = NULL; c = NULL; s = NULL;
    Filter_SelectionBuffer(hits,ii,jj,&v,&c,&s,&M);

    if(ov != v || oc != c || os != s) { 
      BeginHighlight();
      HighlightEntity(v,c,s,0);
      EndHighlight(0);
    }

#if 0
    // l'overlay ne marche pas, meme dans les demos de fltk!
    // soumettre un bug ?
    if(ov != v || oc != c || os != s) { 
      WID->make_overlay_current();
      if(ov != v || oc != c || os != s) { 
	glClearIndex(0);
	glClear(GL_COLOR_BUFFER_BIT);  
	glIndexi((CTX.color.bg<CTX.color.fg)?FL_WHITE:FL_BLACK);
	BeginHighlight();
	HighlightEntity(v,c,s,0);
	EndHighlight(0);
      }
      WID->make_opengl_current();
    }
#endif
    xpos += xmov; 
    ypos += ymov; 
    return 1;

  default:
    return Fl_Gl_Window::handle(event);

  }

}

