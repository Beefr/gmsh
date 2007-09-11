#ifndef _PVIEW_DATA_LIST_H_
#define _PVIEW_DATA_LIST_H_

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

#include <vector>
#include <string>
#include "PViewData.h"
#include "AdaptiveViews.h"
#include "SBoundingBox3d.h"
#include "List.h"

#define VAL_INF 1.e200

// data container using old-style lists of `discontinuous' element
class PViewDataList : public PViewData {
 public: 
  // FIXME: all these members will be made private once the plugins
  // have been rewritten
  int DataSize; // size(double) or sizeof(float)
  int NbTimeStep;
  double Min, Max;
  std::vector<double> TimeStepMin, TimeStepMax;
  SBoundingBox3d BBox;
  List_T *Time;
  int NbSP, NbVP, NbTP;
  List_T *SP, *VP, *TP; // points
  int NbSL, NbVL, NbTL, NbSL2, NbVL2, NbTL2;
  List_T *SL, *VL, *TL, *SL2, *VL2, *TL2; // lines
  int NbST, NbVT, NbTT, NbST2, NbVT2, NbTT2;
  List_T *ST, *VT, *TT, *ST2, *VT2, *TT2; // triangles
  int NbSQ, NbVQ, NbTQ, NbSQ2, NbVQ2, NbTQ2;
  List_T *SQ, *VQ, *TQ, *SQ2, *VQ2, *TQ2; // quadrangles
  int NbSS, NbVS, NbTS, NbSS2, NbVS2, NbTS2;
  List_T *SS, *VS, *TS, *SS2, *VS2, *TS2; // tetrahedra
  int NbSH, NbVH, NbTH, NbSH2, NbVH2, NbTH2;
  List_T *SH, *VH, *TH, *SH2, *VH2, *TH2; // hexahedra
  int NbSI, NbVI, NbTI, NbSI2, NbVI2, NbTI2;
  List_T *SI, *VI, *TI, *SI2, *VI2, *TI2; // prisms
  int NbSY, NbVY, NbTY, NbSY2, NbVY2, NbTY2;
  List_T *SY, *VY, *TY, *SY2, *VY2, *TY2; // pyramids
  int NbT2, NbT3;
  List_T *T2D, *T2C, *T3D, *T3C; // 2D and 3D text strings
  Adaptive_Post_View *adaptive;
 private:
  int _index[24];
  int _lastElement, _lastDimension;
  int _lastNumNodes, _lastNumComponents, _lastNumEdges;
  double *_lastXYZ, *_lastVal;
  void _stat(List_T *D, List_T *C, int nb);
  void _stat(List_T *list, int nbcomp, int nbelm, int nbnod);
  void _setLast(int ele);
  void _setLast(int ele, int dim, int nbnod, int nbcomp, int nbedg,
		List_T *list, int nblist);
  void _getString(int dim, int i, int timestep, std::string &str, 
		  double &x, double &y, double &z, double &style);
  void _splitCurvedElements();
 public:
  PViewDataList(bool allocate=true);
  ~PViewDataList();
  bool finalize();
  int getNumTimeSteps(){ return NbTimeStep; }
  double getTime(int step);
  double getMin(int step=-1);
  double getMax(int step=-1);
  SBoundingBox3d getBoundingBox(){ return BBox; }
  int getNumScalars();
  int getNumVectors();
  int getNumTensors();
  int getNumElements(int type=0);
  int getDimension(int ele);
  int getNumNodes(int ele);
  void getNode(int ele, int nod, double &x, double &y, double &z);
  int getNumComponents(int ele);
  void getValue(int ele, int node, int comp, int step, double &val);
  int getNumEdges(int ele);
  int getNumStrings2D(){ return NbT2; }
  int getNumStrings3D(){ return NbT3; }
  void getString2D(int i, int step, std::string &str, 
		   double &x, double &y, double &style);
  void getString3D(int i, int step, std::string &str, 
		   double &x, double &y, double &z, double &style);
  void smooth();
  bool combineTime(nameData &nd);
  bool combineSpace(nameData &nd);
  bool isAdaptive(){ return adaptive ? true : false; }

  // specific to list-based data sets
  void getRawData(int type, List_T **l, int **ne, int *nc, int *nn);

  // I/O routines
  bool read(FILE *fp, double version, int format, int size);
  bool writePOS(std::string name, bool binary=false, bool parsed=true,
		bool append=false);
  bool writeSTL(std::string name);
  bool writeTXT(std::string name);
  bool writeMSH(std::string name);
};

#endif
