#ifndef _VIEWS_H_
#define _VIEWS_H_

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
// Please report all bugs and problems to "gmsh@geuz.org".

#include "ColorTable.h"
#include "List.h"

#define VIEW_NB_ELEMENT_TYPES  (8*3)
#define VIEW_MAX_ELEMENT_NODES  8

class smooth_container;

class Post_View{
  public :
  // intrinsic to a view
  int Num, Index, Changed, DuplicateOf, Links, Dirty, DisplayListNum;
  char FileName[256], Name[256], AbscissaName[256];

  // the data
  int DataSize; // size(double) or sizeof(float)
  int NbTimeStep, ScalarOnly, TextOnly;
  double Min, Max, BBox[6], *TimeStepMin, *TimeStepMax;
  List_T *Time;
  int NbSP, NbVP, NbTP;
  List_T *SP, *VP, *TP; // points
  int NbSL, NbVL, NbTL;
  List_T *SL, *VL, *TL; // lines
  int NbST, NbVT, NbTT;
  List_T *ST, *VT, *TT; // triangles
  int NbSQ, NbVQ, NbTQ;
  List_T *SQ, *VQ, *TQ; // quadrangles
  int NbSS, NbVS, NbTS;
  List_T *SS, *VS, *TS; // tetrahedra
  int NbSH, NbVH, NbTH;
  List_T *SH, *VH, *TH; // hexahedra
  int NbSI, NbVI, NbTI;
  List_T *SI, *VI, *TI; // prisms
  int NbSY, NbVY, NbTY;
  List_T *SY, *VY, *TY; // pyramids
  int NbT2, NbT3;
  List_T *T2D, *T2C, *T3D, *T3C; // 2D and 3D text strings

  // options
  int Type, Position[2], AutoPosition, Size[2];
  char   Format[256], AbscissaFormat[256];
  double CustomMin, CustomMax;
  double Offset[3], Raise[3], ArrowSize, DisplacementFactor, Explode;
  int Visible, IntervalsType, NbIso, NbAbscissa, Light, SmoothNormals ;
  double AngleSmoothNormals, AlphaChannel;
  int SaturateValues;
  int ShowElement, ShowTime, ShowScale;
  int TransparentScale, ScaleType, RangeType;
  int VectorType, TensorType, ArrowLocation;
  int TimeStep;
  int DrawStrings;
  int DrawPoints, DrawLines, DrawTriangles, DrawQuadrangles;
  int DrawTetrahedra, DrawHexahedra, DrawPrisms, DrawPyramids;
  int DrawScalars, DrawVectors, DrawTensors;
  int Boundary, Grid, PointType, LineType;
  double PointSize, LineWidth;
  GmshColorTable CT;

  // dynamic
  double (*GVFI) (double min, double max, int nb, int index);
  int (*GIFV) (double min, double max, int nb, double value);
  // smooth the view
  void smooth();
  // smooth the normals
  smooth_container *normals;
  void reset_normals();
  void add_normal(double x, double y, double z, 
		  double nx, double ny, double nz);
  bool get_normal(double x, double y, double z, 
		  double &nx, double &ny, double &nz);
  // transform the view
  void transform(double mat[3][3]);

  // some generic access functions
  int get_val(int list, int node, int timestep, double *value);
  void add_val(int list, int node, int timestep, double value);
  int empty();
  void get_raw_data(int type, List_T **list, int **nbe, int *nbc, int *nbn);
};

// Type
#define DRAW_POST_3D       1
#define DRAW_POST_2D_SPACE 2
#define DRAW_POST_2D_TIME  3

// IntervalsType
#define DRAW_POST_ISO          1
#define DRAW_POST_CONTINUOUS   2
#define DRAW_POST_DISCRETE     3
#define DRAW_POST_NUMERIC      4

// VectorType
#define DRAW_POST_SEGMENT      1
#define DRAW_POST_ARROW        2
#define DRAW_POST_PYRAMID      3
#define DRAW_POST_CONE         4
#define DRAW_POST_DISPLACEMENT 5
#define DRAW_POST_ARROW3D      6
#define DRAW_POST_CONE3D       7

// ArrowLocation
#define DRAW_POST_LOCATE_COG     1
#define DRAW_POST_LOCATE_VERTEX  2

// TensorType
#define DRAW_POST_VONMISES      0
#define DRAW_POST_EIGENVECTORS  1

// RangeType
#define DRAW_POST_RANGE_DEFAULT  1
#define DRAW_POST_RANGE_CUSTOM   2
#define DRAW_POST_RANGE_PER_STEP 3

// ScaleType
#define DRAW_POST_LINEAR             1
#define DRAW_POST_LOGARITHMIC        2
#define DRAW_POST_DOUBLELOGARITHMIC  3 // for vorticity e.g.

// Public functions

int fcmpPostViewNum(const void *v1, const void *v2);
int fcmpPostViewDuplicateOf(const void *v1, const void *v2);

Post_View * BeginView (int alloc);
void EndView (Post_View *v, int AddInUI, char *FileName, char *Name);
void DuplicateView(Post_View *v1, int withoptions);
void DuplicateView(int num, int withoptions);
void FreeView(Post_View *v);
bool RemoveViewByIndex(int index);
bool RemoveViewByNumber(int num);
void ReadView(FILE *file, char *filename);
void WriteView(Post_View *v, char *filename, int binary, int append);
void CopyViewOptions(Post_View *src, Post_View *dest);
void CombineViews(int all, int remove);
void CombineViews_Time(int how, int remove);

int BGMWithView (Post_View *ErrView);
double ErrorInView(Post_View * ErrView, int *n);
Post_View *Create2DGraph(char *xname, char *yname, int nbdata, double *x, double *y);

GmshColorTable *Get_ColorTable(int num);
void Print_ColorTable(int num, char *prefix, FILE *file);

double ComputeVonMises(double* val);

#endif
