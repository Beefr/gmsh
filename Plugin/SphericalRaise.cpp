// $Id: SphericalRaise.cpp,v 1.28 2008-01-20 12:21:30 geuzaine Exp $
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

#include "SphericalRaise.h"
#include "Numeric.h"

StringXNumber SphericalRaiseOptions_Number[] = {
  {GMSH_FULLRC, "Xc", NULL, 0.},
  {GMSH_FULLRC, "Yc", NULL, 0.},
  {GMSH_FULLRC, "Zc", NULL, 0.},
  {GMSH_FULLRC, "Raise", NULL, 1.},
  {GMSH_FULLRC, "Offset", NULL, 0.},
  {GMSH_FULLRC, "TimeStep", NULL, 0.},
  {GMSH_FULLRC, "iView", NULL, -1.}
};

extern "C"
{
  GMSH_Plugin *GMSH_RegisterSphericalRaisePlugin()
  {
    return new GMSH_SphericalRaisePlugin();
  }
}


GMSH_SphericalRaisePlugin::GMSH_SphericalRaisePlugin()
{
  ;
}

void GMSH_SphericalRaisePlugin::getName(char *name) const
{
  strcpy(name, "Spherical Raise");
}

void GMSH_SphericalRaisePlugin::getInfos(char *author, char *copyright,
                                         char *help_text) const
{
  strcpy(author, "C. Geuzaine");
  strcpy(copyright, "DGR (www.multiphysics.com)");
  strcpy(help_text,
         "Plugin(SphericalRaise) transforms the\n"
         "coordinates of the elements in the view\n"
         "`iView' using the values associated with the\n"
         "`TimeStep'-th time step. Instead of elevating\n"
         "the nodes along the X, Y and Z axes as in\n"
	 "View[`iView'].RaiseX, View[`iView'].RaiseY\n"
	 "and View[`iView'].RaiseZ, the raise is applied\n"
	 " along the radius of a sphere centered at\n"
	 "(`Xc', `Yc', `Zc'). To produce a standard\n"
	 "radiation pattern, set `Offset' to minus the\n"
	 "radius of the sphere the original data lives on.\n"
	 "If `iView' < 0, the plugin is run on the current\n"
	 "view.\n"
	 "\n"
	 "Plugin(SphericalRaise) is executed in-place.\n");
}

int GMSH_SphericalRaisePlugin::getNbOptions() const
{
  return sizeof(SphericalRaiseOptions_Number) / sizeof(StringXNumber);
}

StringXNumber *GMSH_SphericalRaisePlugin::getOption(int iopt)
{
  return &SphericalRaiseOptions_Number[iopt];
}

void GMSH_SphericalRaisePlugin::catchErrorMessage(char *errorMessage) const
{
  strcpy(errorMessage, "SphericalRaise failed...");
}

static void sphericalRaise(PViewData *data, List_T *list, int nbElm,
			   int nbNod, int timeStep, double center[3], 
			   double raise, double offset)
{
  double *x, *y, *z, *val, d[3], coef;
  int nb, i, j;

  if(!nbElm)
    return;

  if(timeStep < 0 || timeStep > data->getNumTimeSteps() - 1){
    Msg(GERROR, "Invalid TimeStep (%d) in view", timeStep);
    return;
  }

  // for each element
  //   for each node
  //      compute d=(x-Xc,y-Yc,z-Zc)
  //      norm d
  //      get nodal value val at xyz
  //      compute (x,y,z)_new = (x,y,z)_old + (offset+raise*val)*(dx,dy,dz)

  nb = List_Nbr(list) / nbElm;
  for(i = 0; i < List_Nbr(list); i += nb) {
    x = (double *)List_Pointer_Fast(list, i);
    y = (double *)List_Pointer_Fast(list, i + nbNod);
    z = (double *)List_Pointer_Fast(list, i + 2 * nbNod);
    val = (double *)List_Pointer_Fast(list, i + 3 * nbNod);
    for(j = 0; j < nbNod; j++) {
      d[0] = x[j] - center[0];
      d[1] = y[j] - center[1];
      d[2] = z[j] - center[2];
      norme(d);
      coef = offset + raise * val[nbNod * timeStep + j];
      x[j] += coef * d[0];
      y[j] += coef * d[1];
      z[j] += coef * d[2];
    }
  }
}

PView *GMSH_SphericalRaisePlugin::execute(PView *v)
{
  double center[3];
  center[0] = SphericalRaiseOptions_Number[0].def;
  center[1] = SphericalRaiseOptions_Number[1].def;
  center[2] = SphericalRaiseOptions_Number[2].def;
  double raise = SphericalRaiseOptions_Number[3].def;
  double offset = SphericalRaiseOptions_Number[4].def;
  int timeStep = (int)SphericalRaiseOptions_Number[5].def;
  int iView = (int)SphericalRaiseOptions_Number[6].def;

  PView *v1 = getView(iView, v);
  if(!v1) return v;

  PViewDataList *data1 = getDataList(v1);
  if(!data1) return v;

  sphericalRaise(data1, data1->SP, data1->NbSP, 1, timeStep, center, raise, offset);
  sphericalRaise(data1, data1->SL, data1->NbSL, 2, timeStep, center, raise, offset);
  sphericalRaise(data1, data1->ST, data1->NbST, 3, timeStep, center, raise, offset);
  sphericalRaise(data1, data1->SQ, data1->NbSQ, 4, timeStep, center, raise, offset);
  sphericalRaise(data1, data1->SS, data1->NbSS, 4, timeStep, center, raise, offset);
  sphericalRaise(data1, data1->SH, data1->NbSH, 8, timeStep, center, raise, offset);
  sphericalRaise(data1, data1->SI, data1->NbSI, 6, timeStep, center, raise, offset);
  sphericalRaise(data1, data1->SY, data1->NbSY, 5, timeStep, center, raise, offset);

  data1->finalize();
  v1->setChanged(true);

  return v1;
}
