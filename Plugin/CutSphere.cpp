// $Id: CutSphere.cpp,v 1.18 2001-10-29 08:52:21 geuzaine Exp $

#include <string.h>
#include "CutSphere.h"
#include "List.h"
#include "Context.h"

extern Context_T CTX;

StringXNumber CutSphereOptions_Number[] = {
  { GMSH_FULLRC, "Xc" , NULL , 0. },
  { GMSH_FULLRC, "Yc" , NULL , 0. },
  { GMSH_FULLRC, "Zc" , NULL , 0. },
  { GMSH_FULLRC, "R"  , NULL , 0.25 },
  { GMSH_FULLRC, "iView" , NULL , -1. }
};

extern "C"
{
  GMSH_Plugin *GMSH_RegisterCutSpherePlugin ()
  {
    return new GMSH_CutSpherePlugin ();
  }
}


GMSH_CutSpherePlugin::GMSH_CutSpherePlugin()
{
}

void GMSH_CutSpherePlugin::getName(char *name) const
{
  strcpy(name,"Cut Sphere");
}

void GMSH_CutSpherePlugin::getInfos(char *author, char *copyright, char *help_text) const
{
  strcpy(author,"J.-F. Remacle (remacle@scorec.rpi.edu)");
  strcpy(copyright,"DGR (www.multiphysics.com)");
  strcpy(help_text,
	 "Cuts a 3D scalar view with the sphere\n"
	 "(X-Xc)^2 + (Y-Yc)^2 + (Z-Zc)^2  = R^2.\n"
	 "Script name: Plugin(CutSphere).");
}

int GMSH_CutSpherePlugin::getNbOptions() const
{
  return sizeof(CutSphereOptions_Number)/sizeof(StringXNumber);
}

StringXNumber* GMSH_CutSpherePlugin:: GetOption (int iopt)
{
  return &CutSphereOptions_Number[iopt];
}

void GMSH_CutSpherePlugin::CatchErrorMessage (char *errorMessage) const
{
  strcpy(errorMessage,"CutSphere failed...");
}

double GMSH_CutSpherePlugin :: levelset (double x, double y, double z, double val) const
{
  double a = CutSphereOptions_Number[0].def;
  double b = CutSphereOptions_Number[1].def;
  double c = CutSphereOptions_Number[2].def;
  double r = CutSphereOptions_Number[3].def;
  return (x-a)*(x-a) + (y-b)*(y-b) + (z-c)*(z-c) - r*r;
}

Post_View *GMSH_CutSpherePlugin::execute (Post_View *v)
{
  Post_View *vv;

  int iView = (int)CutSphereOptions_Number[4].def;
  _orientation = ORIENT_SPHERE;
  _ref[0] = CutSphereOptions_Number[0].def;
  _ref[1] = CutSphereOptions_Number[1].def;
  _ref[2] = CutSphereOptions_Number[2].def;

  if(v && iView < 0)
    vv = v;
  else{
    if(!v && iView < 0) iView = 0;
    if(!(vv = (Post_View*)List_Pointer_Test(CTX.post.list,iView))){
      Msg(WARNING,"View[%d] does not exist",iView);
      return 0;
    }
  }

  return GMSH_LevelsetPlugin::execute(vv);
}

