#ifndef _CAD_H_
#define _CAD_H_

// Copyright (C) 1997 - 2002 C. Geuzaine, J.-F. Remacle
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
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#include "Mesh.h"

int NEWPOINT(void);
int NEWLINE(void);
int NEWLINELOOP(void);
int NEWSURFACE(void);
int NEWSURFACELOOP(void);
int NEWVOLUME(void);
int NEWPHYSICAL(void);
int NEWREG(void);


Vertex *FindPoint(int inum, Mesh *M);
Vertex *FindVertex(int inum, Mesh *M);
Curve *FindCurve(int inum, Mesh *M);
Surface *FindSurface(int inum, Mesh *M);
Volume *FindVolume(int inum, Mesh *M);
EdgeLoop *FindEdgeLoop(int inum, Mesh *M);
SurfaceLoop *FindSurfaceLoop(int inum, Mesh *M);


Curve * CreateReversedCurve (Mesh *M,Curve *c);
void ModifyLcPoint(int ip, double lc);


void TranslateShapes(double X,double Y,double Z,
                     List_T *ListShapes, int isFinal);
void DilatShapes(double X,double Y,double Z, double A,
                 List_T *ListShapes, int isFinal);
void RotateShapes(double Ax,double Ay,double Az,
		  double Px,double Py, double Pz,
		  double alpha, List_T *ListShapes);
void SymmetryShapes(double A,double B,double C,
		    double D, List_T *ListShapes, int x);
void CopyShape(int Type, int Num, int *New);
void DeleteShape(int Type, int Num);
void ColorShape(int Type, int Num, unsigned int Color);


void ProtudeXYZ(double &x, double &y, double &z, ExtrudeParams *e);
void Extrude_ProtudePoint(int type, int ip, 
			  double T0, double T1, double T2,
			  double A0, double A1, double A2,
                          double X0, double X1, double X2, double alpha,
                          Curve **pc, Curve **prc, 
			  ExtrudeParams *e);
Surface *Extrude_ProtudeCurve(int type, int ic,
			      double T0, double T1, double T2,
			      double A0, double A1, double A2,
			      double X0, double X1, double X2, double alpha,
			      int final, ExtrudeParams *e);
void Extrude_ProtudeSurface(int type, int is,
			    double T0, double T1, double T2,
			    double A0, double A1, double A2,
			    double X0, double X1, double X2, double alpha,
                            int NewVolume, ExtrudeParams *e);


void ReplaceAllDuplicates(Mesh *m);


bool ProjectPointOnCurve (Curve *c, Vertex *v, Vertex *RES, Vertex *DER);
bool ProjectPointOnSurface (Surface *s, Vertex &p);
bool ProjectPointOnSurface (Surface *s, Vertex *p,double *u, double *v);
bool IntersectAllSegmentsTogether(void);


int recognize_seg(int typ, List_T * liste, int *seg);
int recognize_loop(List_T * liste, int *loop);
int recognize_surfloop(List_T * liste, int *loop);


#endif
