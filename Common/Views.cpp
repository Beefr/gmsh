// $Id: Views.cpp,v 1.105 2003-11-29 01:38:49 geuzaine Exp $
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

#include <set>
#include "Gmsh.h"
#include "Numeric.h"
#include "Views.h"
#include "Context.h"
#include "Options.h"
#include "ColorTable.h"

extern Context_T CTX;

#if defined(HAVE_FLTK)
void AddViewInUI();
void RemoveViewInUI();
#endif

#define VAL_INF 1.e200

// Static reference view

Post_View *Post_ViewReference = NULL;

// FIXME: the whole View interface should be rewritten in C++...

int fcmpPostViewNum(const void *v1, const void *v2)
{
  return (((Post_View *) v1)->Num - ((Post_View *) v2)->Num);
}

int fcmpPostViewDuplicateOf(const void *v1, const void *v2)
{
  return (((Post_View *) v1)->DuplicateOf - ((Post_View *) v2)->DuplicateOf);
}

Post_View *BeginView(int allocate)
{
  Post_View vv, *v;
  static int UniqueNum = 0;
  int i;

  if(!CTX.post.list)
    CTX.post.list = List_Create(100, 1, sizeof(Post_View));

  if(!CTX.post.force_num) {
    vv.Num = ++UniqueNum;       // each view _must_ have a unique number
    List_Add(CTX.post.list, &vv);
  }
  else {
    vv.Num = CTX.post.force_num;
    List_Replace(CTX.post.list, &vv, fcmpPostViewNum);
  }

  i = List_ISearch(CTX.post.list, &vv, fcmpPostViewNum);
  v = (Post_View *) List_Pointer(CTX.post.list, i);

  v->Index = i;
  v->Dirty = 1;
  v->DisplayListNum = -1;
  v->NbTimeStep = 0;
  v->NbSP = v->NbVP = v->NbTP = 0;
  v->NbSL = v->NbVL = v->NbTL = 0;
  v->NbST = v->NbVT = v->NbTT = 0;
  v->NbSQ = v->NbVQ = v->NbTQ = 0;
  v->NbSS = v->NbVS = v->NbTS = 0;
  v->NbSH = v->NbVH = v->NbTH = 0;
  v->NbSI = v->NbVI = v->NbTI = 0;
  v->NbSY = v->NbVY = v->NbTY = 0;
  v->NbT2 = v->NbT3 = 0;

  if(allocate) {
    v->DataSize = sizeof(double);

    v->Time = List_Create(100, 1000, sizeof(double));

    v->SP = List_Create(100, 1000, sizeof(double));
    v->VP = List_Create(100, 1000, sizeof(double));
    v->TP = List_Create(100, 1000, sizeof(double));

    v->SL = List_Create(100, 1000, sizeof(double));
    v->VL = List_Create(100, 1000, sizeof(double));
    v->TL = List_Create(100, 1000, sizeof(double));

    v->ST = List_Create(100, 1000, sizeof(double));
    v->VT = List_Create(100, 1000, sizeof(double));
    v->TT = List_Create(100, 1000, sizeof(double));

    v->SQ = List_Create(100, 1000, sizeof(double));
    v->VQ = List_Create(100, 1000, sizeof(double));
    v->TQ = List_Create(100, 1000, sizeof(double));

    v->SS = List_Create(100, 1000, sizeof(double));
    v->VS = List_Create(100, 1000, sizeof(double));
    v->TS = List_Create(100, 1000, sizeof(double));

    v->SH = List_Create(100, 1000, sizeof(double));
    v->VH = List_Create(100, 1000, sizeof(double));
    v->TH = List_Create(100, 1000, sizeof(double));

    v->SI = List_Create(100, 1000, sizeof(double));
    v->VI = List_Create(100, 1000, sizeof(double));
    v->TI = List_Create(100, 1000, sizeof(double));

    v->SY = List_Create(100, 1000, sizeof(double));
    v->VY = List_Create(100, 1000, sizeof(double));
    v->TY = List_Create(100, 1000, sizeof(double));

    v->T2D = List_Create(10, 100, sizeof(double));
    v->T2C = List_Create(100, 1000, sizeof(char));
    v->T3D = List_Create(10, 100, sizeof(double));
    v->T3C = List_Create(100, 1000, sizeof(char));
  }
  else {
    v->Time = NULL;
    v->SP = v->VP = v->TP = NULL;
    v->SL = v->VL = v->TL = NULL;
    v->ST = v->VT = v->TT = NULL;
    v->SQ = v->VQ = v->TQ = NULL;
    v->SS = v->VS = v->TS = NULL;
    v->SH = v->VH = v->TH = NULL;
    v->SI = v->VI = v->TI = NULL;
    v->SY = v->VY = v->TY = NULL;
    v->T2D = v->T2C = NULL;
    v->T3D = v->T3C = NULL;
  }

  // Copy all options from the reference view initialized in InitOptions()
  CopyViewOptions(Post_ViewReference, v);

  v->Changed = 1;
  v->Links = 0;
  v->DuplicateOf = 0;
  v->ScalarOnly = 1;
  v->TextOnly = 1;
  v->normals = NULL;
  v->Min = VAL_INF;
  v->Max = -VAL_INF;
  for(i = 0; i < 3; i++) {
    v->BBox[2 * i] = VAL_INF;
    v->BBox[2 * i + 1] = -VAL_INF;
  }

  return v;
}

double ComputeVonMises(double *V)
{
  static const double THIRD = 1.e0 / 3.e0;
  double tr = (V[0] + V[4] + V[8]) * THIRD;
  double v11 = V[0] - tr;
  double v12 = V[1];
  double v13 = V[2];
  double v21 = V[3];
  double v22 = V[4] - tr;
  double v23 = V[5];
  double v31 = V[6];
  double v32 = V[7];
  double v33 = V[8] - tr;
  return sqrt(1.5 * (v11 * v11 + v12 * v12 + v13 * v13 +
                     v21 * v21 + v22 * v22 + v23 * v23 +
                     v31 * v31 + v32 * v32 + v33 * v33));
}
  
void Stat_Element(Post_View * v, int type, int nbnod, int N,
                  double *X, double *Y, double *Z, double *V)
{
  int i;
  double l0;

  switch (type) {

  case 0:      // scalar
    if(v->Min == VAL_INF || v->Max == -VAL_INF) {
      v->Min = V[0];
      v->Max = V[0];
      v->NbTimeStep = N / nbnod;
    }
    else if(N / nbnod < v->NbTimeStep)
      v->NbTimeStep = N / nbnod;

    for(i = 0; i < N; i++) {
      if(V[i] < v->Min)
        v->Min = V[i];
      if(V[i] > v->Max)
        v->Max = V[i];
    }
    break;

  case 1:      // vector
    if(v->Min == VAL_INF || v->Max == -VAL_INF) {
      l0 = sqrt(DSQR(V[0]) + DSQR(V[1]) + DSQR(V[2]));
      v->Min = l0;
      v->Max = l0;
      v->NbTimeStep = N / (3 * nbnod);
    }
    else if(N / (3 * nbnod) < v->NbTimeStep)
      v->NbTimeStep = N / (3 * nbnod);

    for(i = 0; i < N; i += 3) {
      l0 = sqrt(DSQR(V[i]) + DSQR(V[i + 1]) + DSQR(V[i + 2]));
      if(l0 < v->Min)
        v->Min = l0;
      if(l0 > v->Max)
        v->Max = l0;
    }
    v->ScalarOnly = 0;
    break;

  case 2:      // tensor
    // by lack of any current better solution,
    // tensors are displayed as their Von Mises
    // invariant (J2 invariant)
    if(v->Min == VAL_INF || v->Max == -VAL_INF) {
      l0 = ComputeVonMises(V);
      v->Min = l0;
      v->Max = l0;
      v->NbTimeStep = N / (9 * nbnod);
    }
    else if(N / (9 * nbnod) < v->NbTimeStep)
      v->NbTimeStep = N / (9 * nbnod);

    for(i = 0; i < N; i += 9) {
      l0 = ComputeVonMises(V+i);
      if(l0 < v->Min)
        v->Min = l0;
      if(l0 > v->Max)
        v->Max = l0;
    }
    v->ScalarOnly = 0;
    break;

  }

  for(i = 0; i < nbnod; i++) {
    if(X[i] < v->BBox[0])
      v->BBox[0] = X[i];
    if(X[i] > v->BBox[1])
      v->BBox[1] = X[i];
    if(Y[i] < v->BBox[2])
      v->BBox[2] = Y[i];
    if(Y[i] > v->BBox[3])
      v->BBox[3] = Y[i];
    if(Z[i] < v->BBox[4])
      v->BBox[4] = Z[i];
    if(Z[i] > v->BBox[5])
      v->BBox[5] = Z[i];
  }

  v->TextOnly = 0;
}

void Stat_List(Post_View * v, List_T * listelm, int type, int nbelm,
               int nbnod)
{
  int i, nb;
  if(nbelm) {
    nb = List_Nbr(listelm) / nbelm;
    for(i = 0; i < List_Nbr(listelm); i += nb)
      Stat_Element(v, type, nbnod, nb - 3 * nbnod,
                   (double *)List_Pointer_Fast(listelm, i),
                   (double *)List_Pointer_Fast(listelm, i + 1 * nbnod),
                   (double *)List_Pointer_Fast(listelm, i + 2 * nbnod),
                   (double *)List_Pointer_Fast(listelm, i + 3 * nbnod));
  }
}

void EndView(Post_View * v, int add_in_gui, char *file_name, char *name)
{
  int i;
  double d;

  // Points
  Stat_List(v, v->SP, 0, v->NbSP, 1);
  Stat_List(v, v->VP, 1, v->NbVP, 1);
  Stat_List(v, v->TP, 2, v->NbTP, 1);

  // Lines               
  Stat_List(v, v->SL, 0, v->NbSL, 2);
  Stat_List(v, v->VL, 1, v->NbVL, 2);
  Stat_List(v, v->TL, 2, v->NbTL, 2);

  // Triangles           
  Stat_List(v, v->ST, 0, v->NbST, 3);
  Stat_List(v, v->VT, 1, v->NbVT, 3);
  Stat_List(v, v->TT, 2, v->NbTT, 3);

  // Quadrangles  
  Stat_List(v, v->SQ, 0, v->NbSQ, 4);
  Stat_List(v, v->VQ, 1, v->NbVQ, 4);
  Stat_List(v, v->TQ, 2, v->NbTQ, 4);

  // Tetrahedra          
  Stat_List(v, v->SS, 0, v->NbSS, 4);
  Stat_List(v, v->VS, 1, v->NbVS, 4);
  Stat_List(v, v->TS, 2, v->NbTS, 4);

  // Hexahedra           
  Stat_List(v, v->SH, 0, v->NbSH, 8);
  Stat_List(v, v->VH, 1, v->NbVH, 8);
  Stat_List(v, v->TH, 2, v->NbTH, 8);

  // Prisms              
  Stat_List(v, v->SI, 0, v->NbSI, 6);
  Stat_List(v, v->VI, 1, v->NbVI, 6);
  Stat_List(v, v->TI, 2, v->NbTI, 6);

  // Pyramids            
  Stat_List(v, v->SY, 0, v->NbSY, 5);
  Stat_List(v, v->VY, 1, v->NbVY, 5);
  Stat_List(v, v->TY, 2, v->NbTY, 5);

  // Dummy time values if using old parsed format
  if(v->Time && !List_Nbr(v->Time)) {
    for(i = 0; i < v->NbTimeStep; i++) {
      d = (double)i;
      List_Add(v->Time, &d);
    }
  }

  opt_view_name(v->Index, GMSH_SET | GMSH_GUI, name);
  opt_view_filename(v->Index, GMSH_SET | GMSH_GUI, file_name);
  opt_view_nb_timestep(v->Index, GMSH_GUI, 0);
  opt_view_timestep(v->Index, GMSH_SET | GMSH_GUI, v->TimeStep);
  if(v->Min > v->Max) {
    opt_view_min(v->Index, GMSH_SET | GMSH_GUI, 0.);
    opt_view_max(v->Index, GMSH_SET | GMSH_GUI, 0.);
  }
  else {
    opt_view_min(v->Index, GMSH_GUI, 0);
    opt_view_max(v->Index, GMSH_GUI, 0);
  }
  opt_view_custom_min(v->Index, GMSH_SET | GMSH_GUI, v->Min);
  opt_view_custom_max(v->Index, GMSH_SET | GMSH_GUI, v->Max);

  if(CTX.post.smooth)
    v->smooth();

#if defined(HAVE_FLTK)
  if(!CTX.post.force_num && add_in_gui)
    AddViewInUI();
#endif

  v->Dirty = 0; //the view is complete, we may draw it

  Msg(INFO, "Added View[%d]", v->Index);
}

void DuplicateView(int num, int withoptions)
{
  if(!CTX.post.list || num < 0 || num >= List_Nbr(CTX.post.list))
    return;
  DuplicateView((Post_View *) List_Pointer(CTX.post.list, num), withoptions);
}

void DuplicateView(Post_View * v1, int withoptions)
{
  Post_View v, *v2, *v3;

  v2 = BeginView(0);
  EndView(v2, 0, v1->FileName, v1->Name);

  if(!v1->DuplicateOf) {
    v2->DuplicateOf = v1->Num;
    v1->Links++;
  }
  else {
    v.Num = v1->DuplicateOf;
    if(!(v3 = (Post_View *) List_PQuery(CTX.post.list, &v, fcmpPostViewNum))) {
      v2->DuplicateOf = v1->Num;
      v1->Links++;
    }
    else {
      v2->DuplicateOf = v3->Num;
      v3->Links++;
    }
  }

  // When we duplicate a view, we just point to a reference view: we
  // DON'T allocate a new data set!
  // *INDENT-OFF*

  v2->Time = v1->Time;

  v2->NbSP = v1->NbSP; v2->SP = v1->SP; 
  v2->NbVP = v1->NbVP; v2->VP = v1->VP; 
  v2->NbTP = v1->NbTP; v2->TP = v1->TP;

  v2->NbSL = v1->NbSL; v2->SL = v1->SL; 
  v2->NbVL = v1->NbVL; v2->VL = v1->VL; 
  v2->NbTL = v1->NbTL; v2->TL = v1->TL;

  v2->NbST = v1->NbST; v2->ST = v1->ST; 
  v2->NbVT = v1->NbVT; v2->VT = v1->VT; 
  v2->NbTT = v1->NbTT; v2->TT = v1->TT;

  v2->NbSQ = v1->NbSQ; v2->SQ = v1->SQ; 
  v2->NbVQ = v1->NbVQ; v2->VQ = v1->VQ; 
  v2->NbTQ = v1->NbTQ; v2->TQ = v1->TQ;

  v2->NbSS = v1->NbSS; v2->SS = v1->SS; 
  v2->NbVS = v1->NbVS; v2->VS = v1->VS; 
  v2->NbTS = v1->NbTS; v2->TS = v1->TS;

  v2->NbSH = v1->NbSH; v2->SH = v1->SH; 
  v2->NbVH = v1->NbVH; v2->VH = v1->VH; 
  v2->NbTH = v1->NbTH; v2->TH = v1->TH;

  v2->NbSI = v1->NbSI; v2->SI = v1->SI; 
  v2->NbVI = v1->NbVI; v2->VI = v1->VI; 
  v2->NbTI = v1->NbTI; v2->TI = v1->TI;

  v2->NbSY = v1->NbSY; v2->SY = v1->SY; 
  v2->NbVY = v1->NbVY; v2->VY = v1->VY; 
  v2->NbTY = v1->NbTY; v2->TY = v1->TY;

  v2->NbT2 = v1->NbT2; v2->T2D = v1->T2D; v2->T2C = v1->T2C;
  v2->NbT3 = v1->NbT3; v2->T3D = v1->T3D; v2->T3C = v1->T3C;

  v2->DataSize    = v1->DataSize;
  v2->ScalarOnly  = v1->ScalarOnly;
  v2->TextOnly    = v1->TextOnly;
  v2->Min         = v1->Min;       
  v2->Max         = v1->Max;      
  v2->NbTimeStep  = v1->NbTimeStep;
  for(int i=0 ; i<6 ; i++)
    v2->BBox[i]   = v1->BBox[i];

  // *INDENT-ON*

  if(withoptions)
    CopyViewOptions(v1, v2);

#if defined(HAVE_FLTK)
  AddViewInUI();
#endif
}

bool RemoveViewByIndex(int index)
{
  Post_View *v;

  if(index < 0 || index >= List_Nbr(CTX.post.list)) {
    return false;
  }
  v = (Post_View *) List_Pointer(CTX.post.list, index);
  FreeView(v);
  List_PSuppress(CTX.post.list, index);

#if defined(HAVE_FLTK)
  RemoveViewInUI();
#endif

  // recalculate the indices
  for(int i = 0; i < List_Nbr(CTX.post.list); i++){
    v = (Post_View *) List_Pointer(CTX.post.list, i);
    v->Index = i;
  }

  Msg(INFO, "Removed View[%d] (%d views left)", index, List_Nbr(CTX.post.list));
  return true;
}

bool RemoveViewByNumber(int num)
{
  Post_View vv;

  vv.Num = num;
  int i = List_ISearch(CTX.post.list, &vv, fcmpPostViewNum);
  
  return RemoveViewByIndex(i);
}

void FreeView(Post_View * v)
{
  Post_View vv, *v2;
  int i, numdup, free = 1;

  if(v->DuplicateOf) {
    vv.Num = v->DuplicateOf;
    Msg(DEBUG, "This view is a duplicata");
    if(!(v2 = (Post_View *) List_PQuery(CTX.post.list, &vv, fcmpPostViewNum))) {
      Msg(DEBUG, "  -the original view is gone");
      numdup = 0;
      for(i = 0; i < List_Nbr(CTX.post.list); i++)
        numdup +=
          (((Post_View *) List_Pointer(CTX.post.list, i))->DuplicateOf ==
           v->DuplicateOf);
      if(numdup == 1) {
        Msg(DEBUG, "  -there are no other duplicata, so I can free");
        free = 1;
      }
      else {
        Msg(DEBUG, "  -there are still duplicata, so I cannot free");
        free = 0;
      }
    }
    else {
      v2->Links--;
      free = 0;
      Msg(DEBUG, "  -the original still exists, so I cannot free");
    }
  }

  if(free && !v->Links) {
    Msg(DEBUG, "FREEING VIEW");
    List_Delete(v->Time);
    // *INDENT-OFF*
    List_Delete(v->SP); List_Delete(v->VP); List_Delete(v->TP);
    List_Delete(v->SL); List_Delete(v->VL); List_Delete(v->TL);
    List_Delete(v->ST); List_Delete(v->VT); List_Delete(v->TT);
    List_Delete(v->SQ); List_Delete(v->VQ); List_Delete(v->TQ);
    List_Delete(v->SS); List_Delete(v->VS); List_Delete(v->TS);
    List_Delete(v->SH); List_Delete(v->VH); List_Delete(v->TH);
    List_Delete(v->SI); List_Delete(v->VI); List_Delete(v->TI);
    List_Delete(v->SY); List_Delete(v->VY); List_Delete(v->TY);
    List_Delete(v->T2D); List_Delete(v->T2C);
    List_Delete(v->T3D); List_Delete(v->T3C);
    // *INDENT-ON*
    //set to NULL in case we don't free v (e.g. when doing a 'reload')
    //+ the reload does not work (e.g. the file is gone). This way,
    //the next Free stuff will still work gracefully.
    v->Time = NULL;
    v->SP = v->VP = v->TP = NULL;
    v->SL = v->VL = v->TL = NULL;
    v->ST = v->VT = v->TT = NULL;
    v->SQ = v->VQ = v->TQ = NULL;
    v->SS = v->VS = v->TS = NULL;
    v->SH = v->VH = v->TH = NULL;
    v->SI = v->VI = v->TI = NULL;
    v->SY = v->VY = v->TY = NULL;
    v->T2D = v->T2C = NULL;
    v->T3D = v->T3C = NULL;
    v->reset_normals();
  }

}

void CopyViewOptions(Post_View * src, Post_View * dest)
{
  dest->Type = src->Type;
  dest->Position[0] = src->Position[0];
  dest->Position[1] = src->Position[1];
  dest->AutoPosition = src->AutoPosition;
  dest->Size[0] = src->Size[0];
  dest->Size[1] = src->Size[1];
  strcpy(dest->Format, src->Format);
  strcpy(dest->AbscissaFormat, src->AbscissaFormat);
  strcpy(dest->AbscissaName, src->AbscissaName);
  dest->CustomMin = src->CustomMin;
  dest->CustomMax = src->CustomMax;
  dest->Offset[0] = src->Offset[0];
  dest->Offset[1] = src->Offset[1];
  dest->Offset[2] = src->Offset[2];
  dest->Raise[0] = src->Raise[0];
  dest->Raise[1] = src->Raise[1];
  dest->Raise[2] = src->Raise[2];
  dest->ArrowSize = src->ArrowSize;
  dest->DisplacementFactor = src->DisplacementFactor;
  dest->Explode = src->Explode;
  dest->Visible = src->Visible;
  dest->IntervalsType = src->IntervalsType;
  dest->SaturateValues = src->SaturateValues;
  dest->AlphaChannel = src->AlphaChannel;
  dest->Boundary = src->Boundary;
  dest->NbAbscissa = src->NbAbscissa;
  dest->NbIso = src->NbIso;
  dest->Light = src->Light;
  dest->SmoothNormals = src->SmoothNormals;
  dest->AngleSmoothNormals = src->AngleSmoothNormals;
  dest->ShowElement = src->ShowElement;
  dest->ShowTime = src->ShowTime;
  dest->ShowScale = src->ShowScale;
  dest->DrawPoints = src->DrawPoints;
  dest->DrawLines = src->DrawLines;
  dest->DrawTriangles = src->DrawTriangles;
  dest->DrawQuadrangles = src->DrawQuadrangles;
  dest->DrawTetrahedra = src->DrawTetrahedra;
  dest->DrawHexahedra = src->DrawHexahedra;
  dest->DrawPrisms = src->DrawPrisms;
  dest->DrawPyramids = src->DrawPyramids;
  dest->DrawScalars = src->DrawScalars;
  dest->DrawVectors = src->DrawVectors;
  dest->DrawTensors = src->DrawTensors;
  dest->DrawStrings = src->DrawStrings;
  dest->TransparentScale = src->TransparentScale;
  dest->ScaleType = src->ScaleType;
  dest->RangeType = src->RangeType;
  dest->VectorType = src->VectorType;
  dest->ArrowLocation = src->ArrowLocation;
  dest->TensorType = src->TensorType;
  dest->TimeStep = src->TimeStep;
  dest->PointSize = src->PointSize;
  dest->LineWidth = src->LineWidth;
  dest->PointType = src->PointType;
  dest->LineType = src->LineType;
  dest->Grid = src->Grid;
  ColorTable_Copy(&src->CT);
  ColorTable_Paste(&dest->CT);
}

GmshColorTable *Get_ColorTable(int num)
{
  Post_View *v;

  if(!CTX.post.list)
    v = Post_ViewReference;
  else
    v = (Post_View *) List_Pointer_Test(CTX.post.list, num);
  if(v)
    return &v->CT;
  else
    return NULL;
}

void Print_ColorTable(int num, char *prefix, FILE * file)
{
  char tmp[1024];
  Post_View *v;
  if(!CTX.post.list)
    v = Post_ViewReference;
  else
    v = (Post_View *) List_Pointer_Test(CTX.post.list, num);
  if(!v)
    return;
  sprintf(tmp, "%s = {", prefix);
  if(file)
    fprintf(file, "%s\n", tmp);
  else
    Msg(DIRECT, tmp);
  ColorTable_Print(&v->CT, file);
  sprintf(tmp, "};");
  if(file)
    fprintf(file, "%s\n", tmp);
  else
    Msg(DIRECT, tmp);
}

Post_View *Create2DGraph(char *xname, char *yname,
                         int nbdata, double *x, double *y)
{
  int i;
  double d = 0.;
  char filename[1024];
  Post_View *v;

  v = BeginView(1);
  for(i = 0; i < nbdata; i++) {
    List_Add(v->SP, &x[i]);
    List_Add(v->SP, &d);
    List_Add(v->SP, &d);
    List_Add(v->SP, &y[i]);
    v->NbSP++;
  }
  sprintf(filename, "%s.pos", yname);
  EndView(v, 1, filename, yname);
  v->Type = DRAW_POST_2D_SPACE;
  strcpy(v->AbscissaName, xname);
  return v;
}

// INput/output

void ReadView(FILE *file, char *filename)
{
  char str[256], name[256];
  int i, nb, format, size, testone, swap, t2l, t3l;
  double version;
  Post_View *v;

  Msg(INFO, "Reading post-processing file '%s'", filename);

  while(1) {

    do {
      fgets(str, 256, file);
      if(feof(file))
        break;
    } while(str[0] != '$');

    if(feof(file))
      break;

    /*  F o r m a t  */

    if(!strncmp(&str[1], "PostFormat", 10)) {
      fscanf(file, "%lf %d %d\n", &version, &format, &size);
      if(version < 1.0) {
        Msg(GERROR,
            "The version of this post-processing file is too old (%g < 1.0)",
            version);
        return;
      }
      if(size == sizeof(double))
        Msg(DEBUG, "Data is in double precision format (size==%d)", size);
      else if(size == sizeof(float))
        Msg(DEBUG, "Data is in single precision format (size==%d)", size);
      else {
        Msg(GERROR,
            "Unknown type of data (size = %d) in post-processing file", size);
        return;
      }
      if(format == 0)
        format = LIST_FORMAT_ASCII;
      else if(format == 1)
        format = LIST_FORMAT_BINARY;
      else {
        Msg(GERROR, "Unknown format for view");
        return;
      }
    }

    /*  V i e w  */

    if(!strncmp(&str[1], "View", 4)) {

      v = BeginView(0);

      if(version <= 1.0) {
        fscanf(file, "%s %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
               name, &v->NbTimeStep,
               &v->NbSP, &v->NbVP, &v->NbTP,
               &v->NbSL, &v->NbVL, &v->NbTL,
               &v->NbST, &v->NbVT, &v->NbTT, &v->NbSS, &v->NbVS, &v->NbTS);
        v->NbT2 = t2l = v->NbT3 = t3l = 0;
        Msg(INFO, "Detected version <= 1.0");
      }
      else if(version == 1.1) {
        fscanf(file,
               "%s %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
               name, &v->NbTimeStep, &v->NbSP, &v->NbVP, &v->NbTP, &v->NbSL,
               &v->NbVL, &v->NbTL, &v->NbST, &v->NbVT, &v->NbTT, &v->NbSS,
               &v->NbVS, &v->NbTS, &v->NbT2, &t2l, &v->NbT3, &t3l);
        Msg(INFO, "Detected version 1.1");
      }
      else if(version == 1.2) {
        fscanf(file, "%s %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d "
               "%d %d %d %d %d %d %d %d %d %d %d %d %d\n",
               name, &v->NbTimeStep,
               &v->NbSP, &v->NbVP, &v->NbTP,
               &v->NbSL, &v->NbVL, &v->NbTL,
               &v->NbST, &v->NbVT, &v->NbTT,
               &v->NbSQ, &v->NbVQ, &v->NbTQ,
               &v->NbSS, &v->NbVS, &v->NbTS,
               &v->NbSH, &v->NbVH, &v->NbTH,
               &v->NbSI, &v->NbVI, &v->NbTI,
               &v->NbSY, &v->NbVY, &v->NbTY, &v->NbT2, &t2l, &v->NbT3, &t3l);
        Msg(INFO, "Detected version 1.2");
      }
      else {
        Msg(GERROR, "Unknown post-processing file format (version %g)",
            version);
        return;
      }

      for(i = 0; i < (int)strlen(name); i++)
        if(name[i] == '^')
          name[i] = ' ';

      swap = 0;
      if(format == LIST_FORMAT_BINARY) {
        fread(&testone, sizeof(int), 1, file);
        if(testone != 1) {
          Msg(INFO, "Swapping bytes from binary file");
          swap = 1;
        }
      }

      v->DataSize = size;

      // Time values
      v->Time = List_CreateFromFile(v->NbTimeStep, size, file, format, swap);

#define LL List_CreateFromFile(nb, size, file, format, swap)

      // Points
      nb = v->NbSP ? v->NbSP * (v->NbTimeStep + 3) : 0;
      v->SP = LL;
      nb = v->NbVP ? v->NbVP * (v->NbTimeStep * 3 + 3) : 0;
      v->VP = LL;
      nb = v->NbTP ? v->NbTP * (v->NbTimeStep * 9 + 3) : 0;
      v->TP = LL;

      // Lines
      nb = v->NbSL ? v->NbSL * (v->NbTimeStep * 2 + 6) : 0;
      v->SL = LL;
      nb = v->NbVL ? v->NbVL * (v->NbTimeStep * 2 * 3 + 6) : 0;
      v->VL = LL;
      nb = v->NbTL ? v->NbTL * (v->NbTimeStep * 2 * 9 + 6) : 0;
      v->TL = LL;

      // Triangles
      nb = v->NbST ? v->NbST * (v->NbTimeStep * 3 + 9) : 0;
      v->ST = LL;
      nb = v->NbVT ? v->NbVT * (v->NbTimeStep * 3 * 3 + 9) : 0;
      v->VT = LL;
      nb = v->NbTT ? v->NbTT * (v->NbTimeStep * 3 * 9 + 9) : 0;
      v->TT = LL;

      // Quadrangles
      nb = v->NbSQ ? v->NbSQ * (v->NbTimeStep * 4 + 12) : 0;
      v->SQ = LL;
      nb = v->NbVQ ? v->NbVQ * (v->NbTimeStep * 4 * 3 + 12) : 0;
      v->VQ = LL;
      nb = v->NbTQ ? v->NbTQ * (v->NbTimeStep * 4 * 9 + 12) : 0;
      v->TQ = LL;

      // Tetrahedra
      nb = v->NbSS ? v->NbSS * (v->NbTimeStep * 4 + 12) : 0;
      v->SS = LL;
      nb = v->NbVS ? v->NbVS * (v->NbTimeStep * 4 * 3 + 12) : 0;
      v->VS = LL;
      nb = v->NbTS ? v->NbTS * (v->NbTimeStep * 4 * 9 + 12) : 0;
      v->TS = LL;

      // Hexahedra
      nb = v->NbSH ? v->NbSH * (v->NbTimeStep * 8 + 24) : 0;
      v->SH = LL;
      nb = v->NbVH ? v->NbVH * (v->NbTimeStep * 8 * 3 + 24) : 0;
      v->VH = LL;
      nb = v->NbTH ? v->NbTH * (v->NbTimeStep * 8 * 9 + 24) : 0;
      v->TH = LL;

      // Prisms
      nb = v->NbSI ? v->NbSI * (v->NbTimeStep * 6 + 18) : 0;
      v->SI = LL;
      nb = v->NbVI ? v->NbVI * (v->NbTimeStep * 6 * 3 + 18) : 0;
      v->VI = LL;
      nb = v->NbTI ? v->NbTI * (v->NbTimeStep * 6 * 9 + 18) : 0;
      v->TI = LL;

      // Pyramids
      nb = v->NbSY ? v->NbSY * (v->NbTimeStep * 5 + 15) : 0;
      v->SY = LL;
      nb = v->NbVY ? v->NbVY * (v->NbTimeStep * 5 * 3 + 15) : 0;
      v->VY = LL;
      nb = v->NbTY ? v->NbTY * (v->NbTimeStep * 5 * 9 + 15) : 0;
      v->TY = LL;

#undef LL

      // Strings
      nb = v->NbT2 ? v->NbT2 * 4 : 0;
      v->T2D = List_CreateFromFile(nb, size, file, format, swap);
      v->T2C = List_CreateFromFile(t2l, sizeof(char), file, format, swap);

      nb = v->NbT3 ? v->NbT3 * 5 : 0;
      v->T3D = List_CreateFromFile(nb, size, file, format, swap);
      v->T3C = List_CreateFromFile(t3l, sizeof(char), file, format, swap);


      Msg(DEBUG,
          "Read View '%s' (%d TimeSteps): %d %d %d %d %d %d %d %d %d %d %d "
          "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d", name, v->NbTimeStep,
          v->NbSP, v->NbVP, v->NbTP, v->NbSL, v->NbVL, v->NbTL, v->NbST,
          v->NbVT, v->NbTT, v->NbSQ, v->NbVQ, v->NbTQ, v->NbSS, v->NbVS,
          v->NbTS, v->NbSH, v->NbVH, v->NbTH, v->NbSI, v->NbVI, v->NbTI,
          v->NbSY, v->NbVY, v->NbTY, v->NbT2, v->NbT3);
      Msg(DEBUG,
          "List_Nbrs: " "SP%d VP%d TP%d " "SL%d VL%d TL%d " "ST%d VT%d TT%d "
          "SQ%d VQ%d TQ%d " "SS%d VS%d TS%d " "SH%d VH%d TH%d "
          "SI%d VI%d TI%d " "SY%d VY%d TY%d " "T2D%d T2C%d T3D%d T3C%d",
          List_Nbr(v->SP), List_Nbr(v->VP), List_Nbr(v->TP), List_Nbr(v->SL),
          List_Nbr(v->VL), List_Nbr(v->TL), List_Nbr(v->ST), List_Nbr(v->VT),
          List_Nbr(v->TT), List_Nbr(v->SQ), List_Nbr(v->VQ), List_Nbr(v->TQ),
          List_Nbr(v->SS), List_Nbr(v->VS), List_Nbr(v->TS), List_Nbr(v->SH),
          List_Nbr(v->VH), List_Nbr(v->TH), List_Nbr(v->SI), List_Nbr(v->VI),
          List_Nbr(v->TI), List_Nbr(v->SY), List_Nbr(v->VY), List_Nbr(v->TY),
          List_Nbr(v->T2D), List_Nbr(v->T2C), List_Nbr(v->T3D),
          List_Nbr(v->T3C));

      EndView(v, 1, filename, name);
    }

    do {
      fgets(str, 256, file);
      if(feof(file))
        Msg(GERROR, "Prematured end of file");
    } while(str[0] != '$');

  }     /* while 1 ... */

  Msg(INFO, "Read post-processing file '%s'", filename);
  Msg(STATUS2N, "Read '%s'", filename);
}

// FIXME: add an integer per simplex (region num)
// FIXME: add a format similar to the msh format (node list + simplex list)
// FIXME: add a structured format

void WriteView(int Flag_BIN, Post_View * v, char *filename)
{
  FILE *file;
  char name[256];
  int i, f, One = 1;

  if(filename) {
    file = fopen(filename, Flag_BIN ? "wb" : "w");
    if(!file) {
      Msg(GERROR, "Unable to open file '%s'", filename);
      return;
    }
    Msg(INFO, "Writing post-processing file '%s'", filename);
  }
  else
    file = stdout;

  fprintf(file, "$PostFormat /* Gmsh 1.2, %s */\n",
          Flag_BIN ? "binary" : "ascii");
  fprintf(file, "1.2 %d %d\n", Flag_BIN, (int)sizeof(double));
  fprintf(file, "$EndPostFormat\n");

  for(i = 0; i < (int)strlen(v->Name); i++) {
    if(v->Name[i] == ' ')
      name[i] = '^';
    else
      name[i] = v->Name[i];
  }
  name[i] = '\0';

  fprintf(file, "$View /* %s */\n", v->Name);
  fprintf(file, "%s ", name);
  fprintf(file, "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d "
          "%d %d %d %d %d %d %d %d %d %d %d %d\n",
          List_Nbr(v->Time),
          v->NbSP, v->NbVP, v->NbTP, v->NbSL, v->NbVL, v->NbTL,
          v->NbST, v->NbVT, v->NbTT, v->NbSQ, v->NbVQ, v->NbTQ,
          v->NbSS, v->NbVS, v->NbTS, v->NbSH, v->NbVH, v->NbTH,
          v->NbSI, v->NbVI, v->NbTI, v->NbSY, v->NbVY, v->NbTY,
          v->NbT2, List_Nbr(v->T2C), v->NbT3, List_Nbr(v->T3C));
  if(Flag_BIN) {
    f = LIST_FORMAT_BINARY;
    fwrite(&One, sizeof(int), 1, file);
  }
  else
    f = LIST_FORMAT_ASCII;
  List_WriteToFile(v->Time, file, f);
  List_WriteToFile(v->SP, file, f);
  List_WriteToFile(v->VP, file, f);
  List_WriteToFile(v->TP, file, f);
  List_WriteToFile(v->SL, file, f);
  List_WriteToFile(v->VL, file, f);
  List_WriteToFile(v->TL, file, f);
  List_WriteToFile(v->ST, file, f);
  List_WriteToFile(v->VT, file, f);
  List_WriteToFile(v->TT, file, f);
  List_WriteToFile(v->SQ, file, f);
  List_WriteToFile(v->VQ, file, f);
  List_WriteToFile(v->TQ, file, f);
  List_WriteToFile(v->SS, file, f);
  List_WriteToFile(v->VS, file, f);
  List_WriteToFile(v->TS, file, f);
  List_WriteToFile(v->SH, file, f);
  List_WriteToFile(v->VH, file, f);
  List_WriteToFile(v->TH, file, f);
  List_WriteToFile(v->SI, file, f);
  List_WriteToFile(v->VI, file, f);
  List_WriteToFile(v->TI, file, f);
  List_WriteToFile(v->SY, file, f);
  List_WriteToFile(v->VY, file, f);
  List_WriteToFile(v->TY, file, f);
  List_WriteToFile(v->T2D, file, f);
  List_WriteToFile(v->T2C, file, f);
  List_WriteToFile(v->T3D, file, f);
  List_WriteToFile(v->T3C, file, f);
  fprintf(file, "\n");
  fprintf(file, "$EndView\n");

  if(filename) {
    fclose(file);
    Msg(INFO, "Wrote post-processing file '%s'", filename);
    Msg(STATUS2N, "Wrote '%s'", filename);
  }

}

// Smoothing

using namespace std;

struct xyzv
{
private:
public:
  double x, y, z, *vals;
  int nbvals;
  int nboccurences;
  static double eps;
  void update(int nbVals, double *);
    xyzv(double x, double y, double z);
   ~xyzv();
    xyzv & operator =(const xyzv &);
    xyzv(const xyzv &);
};

double xyzv::eps = 0.0;

xyzv::xyzv(double xx, double yy, double zz)
:x(xx), y(yy), z(zz), vals(0), nbvals(0), nboccurences(0)
{
}

xyzv::~xyzv()
{
  if(vals)
    delete[]vals;
}

xyzv::xyzv(const xyzv & other)
{
  x = other.x;
  y = other.y;
  z = other.z;
  nbvals = other.nbvals;
  nboccurences = other.nboccurences;
  if(other.vals && other.nbvals) {
    vals = new double[other.nbvals];
    for(int i = 0; i < nbvals; i++)
      vals[i] = other.vals[i];
  }
}

xyzv & xyzv::operator =(const xyzv & other)
{
  if(this != &other) {
    x = other.x;
    y = other.y;
    z = other.z;
    nbvals = other.nbvals;
    nboccurences = other.nboccurences;
    if(other.vals && other.nbvals) {
      vals = new double[other.nbvals];
      for(int i = 0; i < nbvals; i++)
        vals[i] = other.vals[i];
    }
  }
  return *this;
}

void xyzv::update(int n, double *v)
{
  int i;
  if(!vals) {
    vals = new double[n];
    for(i = 0; i < n; i++)
      vals[i] = 0.0;
    nbvals = n;
    nboccurences = 0;
  }
  else if(nbvals != n) {
    throw n;
  }

  double x1 = (double)(nboccurences) / (double)(nboccurences + 1);
  double x2 = 1. / (double)(nboccurences + 1);
  for(i = 0; i < nbvals; i++)
    vals[i] = (x1 * vals[i] + x2 * v[i]);
  nboccurences++;

  //printf("val(%d,%f,%f,%f) = %f\n",nboccurences,x,y,z,vals[0]);
}

struct lessthanxyzv
{
  bool operator () (const xyzv & p2, const xyzv & p1)const
  {
    if(p1.x - p2.x > xyzv::eps)
      return true;
    if(p1.x - p2.x < -xyzv::eps)
      return false;
    if(p1.y - p2.y > xyzv::eps)
      return true;
    if(p1.y - p2.y < -xyzv::eps)
      return false;
    if(p1.z - p2.z > xyzv::eps)
      return true;
    return false;
  }
};

typedef set < xyzv, lessthanxyzv > mycont;
typedef mycont::const_iterator iter;

class smooth_container
{
  public: mycont c;
};

void generate_connectivities(List_T * list, int nbList, int nbTimeStep, int nbVert,
                             mycont & connectivities)
{
  double *x, *y, *z, *v;
  int i, j, k;

  if(!nbList) return;

  double *vals = new double[nbTimeStep];
  int nb = List_Nbr(list)/nbList;
  for(i = 0; i < List_Nbr(list); i += nb) {
    x = (double *)List_Pointer_Fast(list, i);
    y = (double *)List_Pointer_Fast(list, i + nbVert);
    z = (double *)List_Pointer_Fast(list, i + 2 * nbVert);
    v = (double *)List_Pointer_Fast(list, i + 3 * nbVert);

    for(j = 0; j < nbVert; j++) {
      for(k = 0; k < nbTimeStep; k++)
        vals[k] = v[j + k * nbVert];
      xyzv xyz(x[j], y[j], z[j]);
      iter it = connectivities.find(xyz);
      if(it == connectivities.end()) {
        xyz.update(nbTimeStep, vals);
        connectivities.insert(xyz);
      }
      else {
        // a little weird ... because we know that this will not
        // destroy the set ordering
        xyzv *xx = (xyzv *) & (*it);
        xx->update(nbTimeStep, vals);
      }
    }
  }
  delete[]vals;
}

void smooth_list(List_T * list, int nbList, double *min, double *max,
                 int nbTimeStep, int nbVert, mycont & connectivities)
{
  double *x, *y, *z, *v;
  int i, j, k;

  if(!nbList)
    return;

  *min = VAL_INF;
  *max = -VAL_INF;

  int nb = List_Nbr(list)/nbList;
  for(i = 0; i < List_Nbr(list); i += nb) {
    x = (double *)List_Pointer_Fast(list, i);
    y = (double *)List_Pointer_Fast(list, i + nbVert);
    z = (double *)List_Pointer_Fast(list, i + 2 * nbVert);
    v = (double *)List_Pointer_Fast(list, i + 3 * nbVert);
    for(j = 0; j < nbVert; j++) {
      xyzv xyz(x[j], y[j], z[j]);
      iter it = connectivities.find(xyz);
      if(it != connectivities.end()) {
        for(k = 0; k < nbTimeStep; k++) {
          v[j + k * nbVert] = (*it).vals[k];
          if(v[j + k * nbVert] < *min)
            *min = v[j + k * nbVert];
          if(v[j + k * nbVert] > *max)
            *max = v[j + k * nbVert];
        }
      }
    }
  }
}

void Post_View::smooth()
{
  xyzv::eps = CTX.lc * 1.e-8;

  if(NbSL || NbST || NbSQ || NbSS || NbSH || NbSI || NbSY) {
    mycont con;
    Msg(INFO, "Smoothing scalar primitives in View[%d]", Index);
    generate_connectivities(SL, NbSL, NbTimeStep, 2, con);
    generate_connectivities(ST, NbST, NbTimeStep, 3, con);
    generate_connectivities(SQ, NbSQ, NbTimeStep, 4, con);
    generate_connectivities(SS, NbSS, NbTimeStep, 4, con);
    generate_connectivities(SH, NbSH, NbTimeStep, 8, con);
    generate_connectivities(SI, NbSI, NbTimeStep, 6, con);
    generate_connectivities(SY, NbSY, NbTimeStep, 5, con);
    smooth_list(SL, NbSL, &Min, &Max, NbTimeStep, 2, con);
    smooth_list(ST, NbST, &Min, &Max, NbTimeStep, 3, con);
    smooth_list(SQ, NbSQ, &Min, &Max, NbTimeStep, 4, con);
    smooth_list(SS, NbSS, &Min, &Max, NbTimeStep, 4, con);
    smooth_list(SH, NbSH, &Min, &Max, NbTimeStep, 8, con);
    smooth_list(SI, NbSI, &Min, &Max, NbTimeStep, 6, con);
    smooth_list(SY, NbSY, &Min, &Max, NbTimeStep, 5, con);
    Changed = 1;
  }
}

// Normal smoothing

void Post_View::reset_normals()
{
  if(normals)
    delete normals;
  normals = 0;
}

void Post_View::add_normal(double x, double y, double z,
                           double nx, double ny, double nz)
{
  if(!normals)
    normals = new smooth_container;

  double n[3] = { nx, ny, nz };
  xyzv xyz(x, y, z);

  iter it = normals->c.find(xyz);

  if(it == normals->c.end()) {
    xyz.update(3, n);
    normals->c.insert(xyz);
  }
  else {
    xyzv *xx = (xyzv *) & (*it);
    xx->update(3, n);
  }
}

double get_angle(double *aa, double *bb)
{
  double angplan, cosc, sinc, a[3], b[3], c[3];
  if(!aa || !bb)
    return 0.;
  a[0] = aa[0];
  a[1] = aa[1];
  a[2] = aa[2];
  b[0] = bb[0];
  b[1] = bb[1];
  b[2] = bb[2];
  norme(a);
  norme(b);
  prodve(a, b, c);
  prosca(a, b, &cosc);
  sinc = sqrt(c[0] * c[0] + c[1] * c[1] + c[2] * c[2]);
  angplan = myatan2(sinc, cosc);
  return angplan * 180. / Pi;
}

bool Post_View::get_normal(double x, double y, double z,
                           double &nx, double &ny, double &nz)
{
  if(!normals)
    return false;

  double n[3] = { nx, ny, nz };
  xyzv xyz(x, y, z);

  iter it = normals->c.find(xyz);

  if(it == normals->c.end())
    return false;

  double angle = get_angle((*it).vals, n);

  if(fabs(angle) < AngleSmoothNormals) {
    nx = (*it).vals[0];
    ny = (*it).vals[1];
    nz = (*it).vals[2];
  }

  return true;
}

// Transformation

static void transform(double mat[3][3], double v[3],
                      double *x, double *y, double *z)
{
  *x = mat[0][0] * v[0] + mat[0][1] * v[1] + mat[0][2] * v[2];
  *y = mat[1][0] * v[0] + mat[1][1] * v[1] + mat[1][2] * v[2];
  *z = mat[2][0] * v[0] + mat[2][1] * v[1] + mat[2][2] * v[2];
}

static void transform_list(List_T *list, int nbList, 
			   int nbVert, double mat[3][3])
{
  double *x, *y, *z, v[3];
  int i, j;

  if(!nbList) return;

  int nb = List_Nbr(list) / nbList;
  for(i = 0; i < List_Nbr(list); i += nb) {
    x = (double *)List_Pointer_Fast(list, i);
    y = (double *)List_Pointer_Fast(list, i + nbVert);
    z = (double *)List_Pointer_Fast(list, i + 2 * nbVert);
    for(j = 0; j < nbVert; j++) {
      v[0] = x[j];
      v[1] = y[j];
      v[2] = z[j];
      transform(mat, v, &x[j], &y[j], &z[j]);
    }
  }
}

void Post_View::transform(double mat[3][3])
{
  transform_list(SP, NbSP, 1, mat);
  transform_list(SL, NbSL, 2, mat);
  transform_list(ST, NbST, 3, mat);
  transform_list(SQ, NbSQ, 4, mat);
  transform_list(SS, NbSS, 4, mat);
  transform_list(SH, NbSH, 8, mat);
  transform_list(SI, NbSI, 6, mat);
  transform_list(SY, NbSY, 5, mat);

  transform_list(VP, NbVP, 1, mat);
  transform_list(VL, NbVL, 2, mat);
  transform_list(VT, NbVT, 3, mat);
  transform_list(VQ, NbVQ, 4, mat);
  transform_list(VS, NbVS, 4, mat);
  transform_list(VH, NbVH, 8, mat);
  transform_list(VI, NbVI, 6, mat);
  transform_list(VY, NbVY, 5, mat);

  transform_list(TP, NbTP, 1, mat);
  transform_list(TL, NbTL, 2, mat);
  transform_list(TT, NbTT, 3, mat);
  transform_list(TQ, NbTQ, 4, mat);
  transform_list(TS, NbTS, 4, mat);
  transform_list(TH, NbTH, 8, mat);
  transform_list(TI, NbTI, 6, mat);
  transform_list(TY, NbTY, 5, mat);

  Changed = 1;
}

// combine lists

static void combine(List_T * a, List_T * b)
{
  if(!a || !b)
    return;
  for(int i = 0; i < List_Nbr(a); i++) {
    List_Add(b, List_Pointer(a, i));
  }
}

void CombineViews(int all)
{
  // sanity check
  int first = 1, nbt = 0;
  for(int i = 0; i < List_Nbr(CTX.post.list) - 1; i++) {
    Post_View *v = (Post_View *) List_Pointer(CTX.post.list, i);
    if(all || v->Visible) {
      if(first){
	nbt = v->NbTimeStep;
	first = 0;
      }
      else{
	if(v->NbTimeStep != nbt){
	  Msg(GERROR, "Cannot combine views having different number of time steps");
	  return;
	}
      }
    }
  }

  Post_View *vm = BeginView(1);
  for(int i = 0; i < List_Nbr(CTX.post.list) - 1; i++) {
    Post_View *v = (Post_View *) List_Pointer(CTX.post.list, i);
    if(all || v->Visible) {
      Msg(DEBUG, "Merging view %d", i);
      // *INDENT-OFF*
      combine(v->SP,vm->SP); vm->NbSP += v->NbSP;
      combine(v->VP,vm->VP); vm->NbVP += v->NbVP; 
      combine(v->TP,vm->TP); vm->NbTP += v->NbTP;
      combine(v->SL,vm->SL); vm->NbSL += v->NbSL;
      combine(v->VL,vm->VL); vm->NbVL += v->NbVL;
      combine(v->TL,vm->TL); vm->NbTL += v->NbTL;
      combine(v->ST,vm->ST); vm->NbST += v->NbST;
      combine(v->VT,vm->VT); vm->NbVT += v->NbVT;
      combine(v->TT,vm->TT); vm->NbTT += v->NbTT;
      combine(v->SQ,vm->SQ); vm->NbSQ += v->NbSQ;
      combine(v->VQ,vm->VQ); vm->NbVQ += v->NbVQ;
      combine(v->TQ,vm->TQ); vm->NbTQ += v->NbTQ;
      combine(v->SS,vm->SS); vm->NbSS += v->NbSS;
      combine(v->VS,vm->VS); vm->NbVS += v->NbVS;
      combine(v->TS,vm->TS); vm->NbTS += v->NbTS;
      combine(v->SH,vm->SH); vm->NbSH += v->NbSH;
      combine(v->VH,vm->VH); vm->NbVH += v->NbVH;
      combine(v->TH,vm->TH); vm->NbTH += v->NbTH;
      combine(v->SI,vm->SI); vm->NbSI += v->NbSI;
      combine(v->VI,vm->VI); vm->NbVI += v->NbVI;
      combine(v->TI,vm->TI); vm->NbTI += v->NbTI;
      combine(v->SY,vm->SY); vm->NbSY += v->NbSY;
      combine(v->VY,vm->VY); vm->NbVY += v->NbVY;
      combine(v->TY,vm->TY); vm->NbTY += v->NbTY;
      // *INDENT-ON*
      /* this more complicated: have to change the indices
         combine(v->T2D,vm->T2D);
         combine(v->T2C,vm->T2C); v->NbT2 += vm->NbT2;
         combine(v->T3D,vm->T3D);
         combine(v->T3C,vm->T3C); v->NbT2 += vm->NbT2;
       */
    }
  }
  EndView(vm, 1, "combined.pos", "combined");
}

// generic access functions

int Post_View::get_val(int list, int node, int timestep, double *value)
{
  return 0;
}

void Post_View::add_val(int list, int node, int timestep, double value)
{
  ;
}

int Post_View::empty(){
  if(NbSP || NbVP || NbTP ||
     NbSL || NbVL || NbTL ||
     NbST || NbVT || NbTT ||
     NbSQ || NbVQ || NbTQ ||
     NbSS || NbVS || NbTS ||
     NbSH || NbVH || NbTH ||
     NbSI || NbVI || NbTI ||
     NbSY || NbVY || NbTY ||
     NbT2 || NbT3)
    return 0;
  else
    return 1;
}
