#include <string>
#include <iostream>
#include "Camera.h"
#include "Gmsh.h"
#include "GmshConfig.h"
#include "GmshMessage.h"
#include "Trackball.h"
#include "Context.h"
#include "drawContext.h"

#if defined(HAVE_FLTK)
#include <FL/Fl_JPEG_Image.H>
#include <FL/Fl_PNG_Image.H>
#endif

using namespace std;
  
  
Camera::Camera( ) {
  aperture = 40;
  focallength = 100.;
  distance=focallength;
  //  eyesep =0.88 ;
  ratio=.015;
  closeness=1.;
  target = origin;
  position = origin;
  position.z = focallength ;
  view.x =0.; 
  view.y =0.; 
  view.z =-1.;
  up.x = 0;  
  up.y = 1; 
  up.z = 0;
  screenwidth=400;
  screenheight=400;
  stereoEnable=false;
  ref_distance=distance;
  this->update();
}
  
Camera::~Camera(){};


void Camera::lookAtCg(){
  target.x=CTX::instance()->cg[0];
  target.y=CTX::instance()->cg[1];
  target.z=CTX::instance()->cg[2];
  double W=CTX::instance()->max[0]-CTX::instance()->min[0];
  double H=CTX::instance()->max[1]-CTX::instance()->min[1];
  double P=CTX::instance()->max[2]-CTX::instance()->min[2];
  //    cout<<" H "<<H << " W"<< W <<endl;
  Lc=sqrt(1.*W*W+1.*H*H+1.*P*P);
  //    cout<<"  "<< 1.*W*W+1.*H*H <<endl;
  //    cout<<"  "<< tan(aperture) <<endl;
  distance=fabs(.5*Lc*4./3./tan(aperture*.01745329/2.));
  distance=Lc*1.8;
  //    cout<<" RC "<<RC << " distance"<< distance <<endl;
  position.x=target.x-distance*view.x;
  position.y=target.y-distance*view.y;
  position.z=target.z-distance*view.z;
  //  cout<<" cg "<<target.x << " "<< target.y  << " "<<target.z<<endl;
  update();
  focallength=distance;
  ref_distance=distance;
  eyesep=focallength*ratio;
}


void Camera::init(){  
  aperture = 40;
  ratio=.015;
  focallength = 100;
  //  ratio=1./50.;
  target = origin;
  distance=focallength;
  position = origin;
  position.z = distance ;
  view.x =0.; 
  view.y =0.; 
  view.z =-1.;
  up.x = 0;  
  up.y = 1; 
  up.z = 0;
  ref_distance=distance;
  eyesep=distance*ratio;
  lookAtCg();
}

void Camera::update() {
  right.x=view.y*up.z-view.z*up.y;
  right.y=view.z*up.x-view.x*up.z;
  right.z=view.x*up.y-view.y*up.x;
  up.x=right.y*view.z-right.z*view.y;
  up.y=right.z*view.x-right.x*view.z;
  up.z=right.x*view.y-right.y*view.x;
  ref_distance=distance;
  normalize(up);
  normalize(right);
  normalize(view);
}

void Camera::moveRight(double theta) {
  this->update();

  position.x=position.x-distance*tan(theta)*right.x;
  position.y=position.y-distance*tan(theta)*right.y;
  position.z=position.z-distance*tan(theta)*right.z;
  target.x=position.x+distance*view.x;
  target.y=position.y+distance*view.y;
  target.z=position.z+distance*view.z;
  this->update();
}

void Camera::moveUp(double theta) {
  this->update();
  position.x=position.x+distance*tan(theta)*up.x;
  position.y=position.y+distance*tan(theta)*up.y;
  position.z=position.z+distance*tan(theta)*up.z;
  target.x=position.x+distance*view.x;
  target.y=position.y+distance*view.y;
  target.z=position.z+distance*view.z;
  this->update();
}

void Camera::rotate(double* q) {
  this->update();
  // rotation projection in global coordinates  
  quaternion omega;
  omega.x=-q[0]*right.x+q[1]*up.x+q[2]*view.x ;
  omega.y=-q[0]*right.y+q[1]*up.y+q[2]*view.y ;
  omega.z=-q[0]*right.z+q[1]*up.z+q[2]*view.z ;
  omega.w=q[3];

  quaternion q_view,q_position,new_q_view,new_q_position;
  quaternion q_right,q_up,new_q_right,new_q_up;

  // normalize the axe of rotation in the quaternion omega if not null
  double sina=sin(acos(omega.w));
  double length;
  if (sina != 0.){
    length=(omega.x*omega.x+omega.y*+omega.y+omega.z*omega.z)/(sina*sina);
  }
  else{
    length=0.;
  }
  length=sqrt(length);
  if (length!=0.){

    omega.x=omega.x/length;
    omega.y=omega.y/length;
    omega.z=omega.z/length;
    //  rotation of the camera (view,up and right vectors) 
    // arround 0 0 0  the CenterOfRotation  
    //normalize(camera.view);
  
    //rotate view direction
    q_view.x=view.x;
    q_view.y=view.y;
    q_view.z=view.z;
    q_view.w=0.;
    normalize(q_view);
    new_q_view=mult(mult(omega,q_view),conjugate(omega));
    view.x=new_q_view.x ;
    view.y=new_q_view.y;
    view.z=new_q_view.z;
    //rotate up direction
    q_up.x=up.x;
    q_up.y=up.y;
    q_up.z=up.z;
    q_up.w=0.;
    normalize(q_up);
    new_q_up=mult(mult(omega,q_up),conjugate(omega));
    up.x=new_q_up.x ;
    up.y=new_q_up.y;
    up.z=new_q_up.z;
    //rotate right direction
    q_right.x=right.x;
    q_right.y=right.y;
    q_right.z=right.z;
    q_right.w=0.;
    normalize(q_right);
    new_q_right=mult(mult(omega,q_right),conjugate(omega));
    right.x=new_q_right.x ;
    right.y=new_q_right.y;
    right.z=new_q_right.z;

     //actualize camera position 
    position.x=target.x-view.x*distance;
    position.y=target.y-view.y*distance;
    position.z=target.z-view.z*distance;
      
  }
  this->update();
}

