// Gmsh - Copyright (C) 1997-2009 C. Geuzaine, J.-F. Remacle
//
// See the LICENSE.txt file for license information. Please report all
// bugs and problems to <gmsh@geuz.org>.

#ifndef _DRAW_CONTEXT_H_
#define _DRAW_CONTEXT_H_

#include <string>
#include <FL/gl.h>

//FIXME: workaround faulty fltk installs
//#include <FL/glu.h>
#ifdef __APPLE__
#  include <OpenGL/glu.h>
#else
#  include <GL/glu.h>
#endif

#include "SBoundingBox3d.h"

class PView;

class drawTransform {
 public:
  drawTransform(){}
  virtual ~drawTransform(){}
  virtual void transform(double &x, double &y, double &z){}
  virtual void transformOneForm(double &x, double &y, double &z){}
  virtual void transformTwoForm(double &x, double &y, double &z){}
  virtual void setMatrix(double mat[3][3], double tra[3]){}
};

class drawTransformScaled : public drawTransform {
 private:
  double _mat[3][3];
  double _tra[3];
 public:
  drawTransformScaled(double mat[3][3], double tra[3]=0)
    : drawTransform()
  {
    setMatrix(mat, tra);
  }
  virtual void setMatrix(double mat[3][3], double tra[3]=0)
  {
    for(int i = 0; i < 3; i++){
      for(int j = 0; j < 3; j++)
        _mat[i][j] = mat[i][j];
      if(tra) _tra[i] = tra[i];
      else _tra[i] = 0.;
    }
  }
  virtual void transform(double &x, double &y, double &z)
  {
    double xyz[3] = {x, y, z};
    x = y = z = 0.;
    for(int k = 0; k < 3; k++){
      x += _mat[0][k] * xyz[k];
      y += _mat[1][k] * xyz[k];
      z += _mat[2][k] * xyz[k];
    }
    x += _tra[0];
    y += _tra[1];
    z += _tra[2];
  }
};

class drawContext {
 private:
  drawTransform *_transform;
  GLUquadricObj *_quadric;
  GLuint _displayLists;

 public:
  double r[3]; // current Euler angles (in degrees!) 
  double t[3], s[3]; // current translation and scale 
  double quaternion[4]; // current quaternion used for "trackball" rotation
  int viewport[4]; // current viewport 
  double rot[16]; // current rotation matrix 
  double t_init[3]; // initial translation before applying modelview transform
  double vxmin, vxmax, vymin, vymax; // current viewport in real coordinates 
  double pixel_equiv_x, pixel_equiv_y; // approx equiv model length of a pixel 
  double model[16], proj[16]; // the modelview and projection matrix as they were
                              // at the time of the last InitPosition() call
  enum RenderMode {GMSH_RENDER=1, GMSH_SELECT=2, GMSH_FEEDBACK=3};
  int render_mode; // current rendering mode

 public:
  drawContext(drawTransform *transform=0);
  ~drawContext();
  void setTransform(drawTransform *transform){ _transform = transform; }
  drawTransform *getTransform(){ return _transform; }
  void transform(double &x, double &y, double &z)
  { 
    if(_transform) _transform->transform(x, y, z); 
  }
  void transformOneForm(double &x, double &y, double &z)
  {
    if(_transform) _transform->transformOneForm(x, y, z); 
  }
  void transformTwoForm(double &x, double &y, double &z)
  {
    if(_transform) _transform->transformTwoForm(x, y, z); 
  }
  void createQuadricsAndDisplayLists();
  void buildRotationMatrix();
  void setQuaternion(double p1x, double p1y, double p2x, double p2y);
  void addQuaternion(double p1x, double p1y, double p2x, double p2y);
  void addQuaternionFromAxisAndAngle(double axis[3], double angle);
  void setQuaternionFromEulerAngles();
  void setEulerAnglesFromRotationMatrix();
  void initProjection(int xpick=0, int ypick=0, int wpick=0, int hpick=0);
  void initRenderModel();
  void initPosition();
  void unproject(double x, double y, double p[3], double d[3]);
  void viewport2World(double win[3], double xyz[3]);
  void world2Viewport(double xyz[3], double win[3]);
  int fix2dCoordinates(double *x, double *y);
  void draw3d();
  void draw2d();
  void drawGeom();
  void drawMesh();
  void drawPost();
  void drawText2d();
  void drawGraph2d();
  void drawAxis(double xmin, double ymin, double zmin,
                double xmax, double ymax, double zmax, 
                int nticks, int mikado);
  void drawAxes(int mode, int tics[3], char format[3][256],
                char label[3][256], double bb[6], int mikado);
  void drawAxes(int mode, int tics[3], char format[3][256], 
                char label[3][256], SBoundingBox3d &bb, int mikado);
  void drawAxes();
  void drawSmallAxes();
  void drawScales();
  void drawString(std::string s, const char *font_name, int font_enum, 
                  int font_size, int align);
  void drawString(std::string s);
  void drawStringCenter(std::string s);
  void drawStringRight(std::string s);
  void drawString(std::string s, double style);
  void drawSphere(double R, double x, double y, double z, int n1, int n2, int light);
  void drawSphere(double size, double x, double y, double z, int light);
  void drawCylinder(double width, double *x, double *y, double *z, int light);
  void drawTaperedCylinder(double width, double val1, double val2, 
                           double ValMin, double ValMax, 
                           double *x, double *y, double *z, int light);
  void drawArrow3d(double x, double y, double z, double dx, double dy, double dz, 
                   double length, int light);
  void drawVector(int Type, int Fill, double x, double y, double z,
                  double dx, double dy, double dz, int light);
  void drawBox(double xmin, double ymin, double zmin,
               double xmax, double ymax, double zmax,
               bool labels=true);
  void drawPlaneInBoundingBox(double xmin, double ymin, double zmin,
                              double xmax, double ymax, double zmax,
                              double a, double b, double c, double d,
                              int shade=0);
};

#endif
