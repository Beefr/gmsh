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
// Please report all bugs and problems to <gmsh@geuz.org>.

#include "ColorTable.h"
#include "List.h"
#include "VertexArray.h"
#include "SmoothNormals.h"
#include "GmshMatrix.h"

#define VIEW_NB_ELEMENT_TYPES  (8*3)
#define VIEW_MAX_ELEMENT_NODES  8
#define VAL_INF 1.e200

class Post_View;

#define MAX_LEVEL_OF_ZOOM 40

// On a triangle, we suppose that there exists an
// interpolation scheme such that u = \sum_i u_i \phi_i
// phi_i being polynomials of order p, i goes from 1...(p+1)(p+2)/2
// and phi_i = \sum_j coeffs_{ij} monomials_j and
// monomials are 1,x,y,x^2,xy,y^2,x^3,x^2y,xy^2,y^3...

/// A zoom is a triangulation in reference coordinates
class Post_Zoom
{
public:
  Post_Zoom( int level , Double_Matrix *coeffs);
  ~Post_Zoom()
  {
    delete M;
    delete MGeom;
    delete Points;
    delete Simplices;
  }
  Double_Matrix *M;
  Double_Matrix *MGeom;
  Double_Matrix *Points;
  Int_Matrix *Simplices;
  void interpolate ( Double_Matrix *coefs , double u, double v, double *sf);
};

class Adaptive_Post_View 
{
  Post_Zoom* ZOOMS [MAX_LEVEL_OF_ZOOM+1];
  double tol;
  int presentZoomLevel;
  Double_Matrix * _coefs;
  Double_Matrix * _coefs_L2;
  Double_Matrix * _STposX;
  Double_Matrix * _STposY;
  Double_Matrix * _STposZ;
  Double_Matrix * _STval;
public:
  Adaptive_Post_View (Post_View *view, List_T *_coeffs);
  ~Adaptive_Post_View()
  {
    delete _coefs;
    delete _STposX;
    delete _STposY;
    delete _STposZ;
    delete _STval;
    for (int i=0;i<MAX_LEVEL_OF_ZOOM+1;i++)
      if (ZOOMS[i]) delete ZOOMS[i];
  }
  int getGlobalResolutionLevel ( ) const {return presentZoomLevel;}
  void setGlobalResolutionLevel ( Post_View * view , int level );
  void setAdaptiveResolutionLevel ( Post_View * view , int levelmax = MAX_LEVEL_OF_ZOOM );
  void initWithLowResolution (Post_View *view);
  void setTolerance (const double eps) {tol=eps;}
  void zoomElement (Post_View * view ,
		    int ielem ,
		    Post_Zoom *zoom);

};

class Post_View{
  public :
  // The view may be high order, coeffs are then interpreated as
  // coefficients of a high order interpolation. So, a pre-pro
  // is done at the end of the view that sets the view to the
  // minimal resolution. Then, we can interactively modify the
  // resolution.
  
  Adaptive_Post_View *adaptive;
  void setGlobalResolutionLevel (int level)
  {
    if ( adaptive )
      adaptive->setGlobalResolutionLevel(this, level);
  }
  void setAdaptiveResolutionLevel (int level)
  {
    if ( adaptive )
      adaptive->setAdaptiveResolutionLevel(this, level);
  }

  // intrinsic to a view
  int Num, Index, Changed, DuplicateOf, Links, Dirty;
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

  // a vertex array to draw triangles efficiently
  VertexArray *TriVertexArray;

  // options
  int Type, Position[2], AutoPosition, Size[2];
  char   Format[256], AbscissaFormat[256];
  double CustomMin, CustomMax;
  double Offset[3], Raise[3], DisplacementFactor, Explode;
  double ArrowSize, ArrowRelHeadRadius, ArrowRelStemRadius, ArrowRelStemLength;
  int Visible, IntervalsType, NbIso, NbAbscissa, ArrowSizeProportional;
  int Light, LightTwoSide, SmoothNormals;
  double AngleSmoothNormals, AlphaChannel;
  int SaturateValues;
  int ShowElement, ShowTime, ShowScale;
  int ScaleType, RangeType;
  int VectorType, TensorType, ArrowLocation;
  int TimeStep;
  int DrawStrings;
  int DrawPoints, DrawLines, DrawTriangles, DrawQuadrangles;
  int DrawTetrahedra, DrawHexahedra, DrawPrisms, DrawPyramids;
  int DrawScalars, DrawVectors, DrawTensors;
  int Boundary, Grid, PointType, LineType;
  double PointSize, LineWidth;
  GmshColorTable CT;
  int ExternalViewIndex;

  // dynamic
  double (*GVFI) (double min, double max, int nb, int index);
  int (*GIFV) (double min, double max, int nb, double value);
  int ExternalElementIndex;
  double ExternalMin, ExternalMax;

  // smooth the view
  void smooth();
  // smooth normals
  smooth_normals *normals;
  void reset_normals();
  // transform the view
  void transform(double mat[3][3]);

  // some generic access functions
  int get_val(int list, int node, int timestep, double *value);
  void add_val(int list, int node, int timestep, double value);
  int empty();
  void get_raw_data(int type, List_T **list, int **nbe, int *nbc, int *nbn);
};


// We have here a post processing map that is the solution of a high order
// interpolation. The principle is that we are able to produce adaptive 
// visualizations i.e. we can Zoom on the picture and generate automatically
// levels of accuracy. 


// Type
#define DRAW_POST_3D       1
#define DRAW_POST_2D_SPACE 2
#define DRAW_POST_2D_TIME  3

// IntervalsType
#define DRAW_POST_ISO        1
#define DRAW_POST_CONTINUOUS 2
#define DRAW_POST_DISCRETE   3
#define DRAW_POST_NUMERIC    4

// VectorType
#define DRAW_POST_SEGMENT      1
#define DRAW_POST_ARROW        2
#define DRAW_POST_PYRAMID      3
#define DRAW_POST_ARROW3D      4
#define DRAW_POST_DISPLACEMENT 5

// ArrowLocation
#define DRAW_POST_LOCATE_COG    1
#define DRAW_POST_LOCATE_VERTEX 2

// TensorType
#define DRAW_POST_VONMISES     0
#define DRAW_POST_EIGENVECTORS 1

// RangeType
#define DRAW_POST_RANGE_DEFAULT  1
#define DRAW_POST_RANGE_CUSTOM   2
#define DRAW_POST_RANGE_PER_STEP 3

// ScaleType
#define DRAW_POST_LINEAR            1
#define DRAW_POST_LOGARITHMIC       2
#define DRAW_POST_DOUBLELOGARITHMIC 3 // for vorticity e.g.

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
void WriteView(Post_View *v, char *filename, int format, int append);
void CopyViewOptions(Post_View *src, Post_View *dest);
void CombineViews(int time, int how, int remove);

int BGMWithView (Post_View *ErrView);
double ErrorInView(Post_View * ErrView, int *n);
Post_View *Create2DGraph(char *xname, char *yname, int nbdata, double *x, double *y);

GmshColorTable *Get_ColorTable(int num);
void Print_ColorTable(int num, int diff, char *prefix, FILE *file);

double ComputeVonMises(double* val);

#endif
