// $Id: CutGrid.cpp,v 1.1 2004-04-22 09:35:01 remacle Exp $
//
// Copyright (C) 1997-2003 C. Geuzaine, J.-F. Remacle
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
// Please report all bugs and problems to "gmsh@geuz.org".

#include "OctreePost.h"
#include "CutGrid.h"
#include "List.h"
#include "Context.h"
#include "Views.h"
#include "Message.h"

extern Context_T CTX;

StringXNumber CutGridOptions_Number[] = {
  {GMSH_FULLRC, "X0", NULL, -1.},
  {GMSH_FULLRC, "Y0", NULL, -1.},
  {GMSH_FULLRC, "Z0", NULL, 0.},
  {GMSH_FULLRC, "X1", NULL, -1.},
  {GMSH_FULLRC, "Y1", NULL, 0.},
  {GMSH_FULLRC, "Z1", NULL, 0.},
  {GMSH_FULLRC, "X2", NULL, 0.},
  {GMSH_FULLRC, "Y2", NULL, -1.},
  {GMSH_FULLRC, "Z2", NULL, 0.},
  {GMSH_FULLRC, "nPoints", NULL, 20},
  {GMSH_FULLRC, "iView", NULL, -1.}
};

extern "C"
{
  GMSH_Plugin *GMSH_RegisterCutGridPlugin()
  {
    return new GMSH_CutGridPlugin();
  }
}


GMSH_CutGridPlugin::GMSH_CutGridPlugin()
{
  ;
}

void GMSH_CutGridPlugin::getName(char *name) const
{
  strcpy(name, "Cut grid");
}

void GMSH_CutGridPlugin::getInfos(char *author, char *copyright,
                                   char *help_text) const
{
  strcpy(author, "J.-F. Remacle (remacle@scorec.rpi.edu)");
  strcpy(copyright, "DGR (www.multiphysics.com)");
  strcpy(help_text,
         "Cuts a 3D view with a rectangular {X0,Y0,Z0}-{X1,Y1,Z1} grid\n"
         "using nbPoints x nbPoints\n" "Script name: Plugin(CutGrid).");
}

int GMSH_CutGridPlugin::getNbOptions() const
{
  return sizeof(CutGridOptions_Number) / sizeof(StringXNumber);
}

StringXNumber *GMSH_CutGridPlugin::getOption(int iopt)
{
  return &CutGridOptions_Number[iopt];
}

void GMSH_CutGridPlugin::catchErrorMessage(char *errorMessage) const
{
  strcpy(errorMessage, "CutGrid failed...");
}

int GMSH_CutGridPlugin::getNbU()const 
{
  return   (int)CutGridOptions_Number[9].def;
}

int GMSH_CutGridPlugin::getNbV()const 
{
  return   (int)CutGridOptions_Number[9].def;
}

void GMSH_CutGridPlugin::getPoint(int iU, int iV, double *X )const 
{
  double u = (double)iU / (double)(getNbU () - 1.);
  double v = (double)iV / (double)(getNbV () - 1.);
  X[0] = CutGridOptions_Number[0].def + 
    u  * (CutGridOptions_Number[3].def-CutGridOptions_Number[0].def) +
    v  * (CutGridOptions_Number[6].def-CutGridOptions_Number[0].def) ;
  X[1] = CutGridOptions_Number[1].def + 
    u  * (CutGridOptions_Number[4].def-CutGridOptions_Number[1].def) +
    v  * (CutGridOptions_Number[7].def-CutGridOptions_Number[1].def) ;
  X[2] = CutGridOptions_Number[2].def + 
    u  * (CutGridOptions_Number[5].def-CutGridOptions_Number[2].def) +
    v  * (CutGridOptions_Number[8].def-CutGridOptions_Number[2].def) ;
}

Post_View * GMSH_CutGridPlugin::GenerateView (Post_View * v) const 
{
  Post_View * View = BeginView (1);

  double X1[3],X2[3],X3[3],X4[3];
  double *VALUES1 = new double [9*v->NbTimeStep];
  double *VALUES2 = new double [9*v->NbTimeStep];
  double *VALUES3 = new double [9*v->NbTimeStep];
  double *VALUES4 = new double [9*v->NbTimeStep];

  OctreePost o ( v );

  for (int i=0 ; i < getNbU ()-1 ; ++i )
    {
      for (int j=0 ; j < getNbV ()-1;++j )
	{
	  getPoint ( i  ,j  ,X1  );
	  getPoint ( i+1,j  ,X2  );
	  getPoint ( i+1,j+1,X3  );
	  getPoint ( i  ,j+1,X4  );
	  if ( v->NbSS || v->NbSH)
	    {
	      List_Add(View->SQ, &X1[0]);
	      List_Add(View->SQ, &X2[0]);
	      List_Add(View->SQ, &X3[0]);
	      List_Add(View->SQ, &X4[0]);
	      List_Add(View->SQ, &X1[1]);
	      List_Add(View->SQ, &X2[1]);
	      List_Add(View->SQ, &X3[1]);
	      List_Add(View->SQ, &X4[1]);
	      List_Add(View->SQ, &X1[2]);
	      List_Add(View->SQ, &X2[2]);
	      List_Add(View->SQ, &X3[2]);
	      List_Add(View->SQ, &X4[2]);
	      View->NbSQ ++;
	      o.searchScalar ( X1[0],X1[1],X1[2] , VALUES1 );
	      o.searchScalar ( X2[0],X2[1],X2[2] , VALUES2 );
	      o.searchScalar ( X3[0],X3[1],X3[2] , VALUES3 );
	      o.searchScalar ( X4[0],X4[1],X4[2] , VALUES4 );
	      for (int k=0;k<v->NbTimeStep;++k)
		{
		  List_Add(View->SQ, &VALUES1[k]);	      
		  List_Add(View->SQ, &VALUES2[k]);	      
		  List_Add(View->SQ, &VALUES3[k]);	      
		  List_Add(View->SQ, &VALUES4[k]);	      
		}
	    }
	  if ( v->NbVS || v->NbVH)
	    {
	      List_Add(View->VQ, &X1[0]);
	      List_Add(View->VQ, &X2[0]);
	      List_Add(View->VQ, &X3[0]);
	      List_Add(View->VQ, &X4[0]);
	      List_Add(View->VQ, &X1[1]);
	      List_Add(View->VQ, &X2[1]);
	      List_Add(View->VQ, &X3[1]);
	      List_Add(View->VQ, &X4[1]);
	      List_Add(View->VQ, &X1[2]);
	      List_Add(View->VQ, &X2[2]);
	      List_Add(View->VQ, &X3[2]);
	      List_Add(View->VQ, &X4[2]);
	      View->NbVQ ++;
	      double sizeElem;
	      o.searchVector ( X1[0],X1[1],X1[2] , &sizeElem,VALUES1 );
	      o.searchVector ( X2[0],X2[1],X2[2] , &sizeElem,VALUES2 );
	      o.searchVector ( X3[0],X3[1],X3[2] , &sizeElem,VALUES3 );
	      o.searchVector ( X4[0],X4[1],X4[2] , &sizeElem,VALUES4 );
	      for (int k=0;k<v->NbTimeStep;++k)
		{
		  List_Add(View->VQ, &VALUES1[3*i]);	      
		  List_Add(View->VQ, &VALUES1[3*i+1]);	      
		  List_Add(View->VQ, &VALUES1[3*i+2]);	      
		  List_Add(View->VQ, &VALUES2[3*i]);	      
		  List_Add(View->VQ, &VALUES2[3*i+1]);	      
		  List_Add(View->VQ, &VALUES2[3*i+2]);	      
		  List_Add(View->VQ, &VALUES3[3*i]);	      
		  List_Add(View->VQ, &VALUES3[3*i+1]);	      
		  List_Add(View->VQ, &VALUES3[3*i+2]);	      
		  List_Add(View->VQ, &VALUES4[3*i]);	      
		  List_Add(View->VQ, &VALUES4[3*i+1]);	      
		  List_Add(View->VQ, &VALUES4[3*i+2]);	      
		}
	    }
	}
    }

  char name[1024], filename[1024];
  sprintf(name, "cut-%s", v->Name);
  sprintf(filename, "cut-%s", v->FileName);
  EndView(View, v->NbTimeStep, filename, name);

  delete [] VALUES1;
  delete [] VALUES2;
  delete [] VALUES3;
  delete [] VALUES4;
}

Post_View *GMSH_CutGridPlugin::execute(Post_View * v)
{
  Post_View *vv;

  int iView = (int)CutGridOptions_Number[10].def;

  if(v && iView < 0)
    vv = v;
  else {
    if(!v && iView < 0)
      iView = 0;
    if(!(vv = (Post_View *) List_Pointer_Test(CTX.post.list, iView))) {
      return 0;
    }
  }
 Post_View * newView = GenerateView (vv);

 return newView;
}

void GMSH_CutGridPlugin::Run()
{
  execute(0);
}
void GMSH_CutGridPlugin::Save()
{
}
