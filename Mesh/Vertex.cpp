// $Id: Vertex.cpp,v 1.14 2001-12-03 08:41:44 geuzaine Exp $

#include "Gmsh.h"
#include "Numeric.h"
#include "Vertex.h"
#include "Mesh.h"
#include "Context.h"

extern Context_T CTX ;
extern Mesh *THEM ;

Vertex::Vertex (){
  Frozen = 0;
  Visible = VIS_GEO|VIS_MESH;
  Pos.X = 0.0;
  Pos.Y = 0.0;
  Pos.Z = 0.0;
  lc = 1.0;
  Mov = NULL;
  ListSurf = NULL;
  ListCurves = NULL;
  Extruded_Points = NULL;
}

Vertex::Vertex (double X, double Y, double Z, double l, double W){
  Frozen = 0;
  Visible = VIS_GEO|VIS_MESH;
  Pos.X = X;
  Pos.Y = Y;
  Pos.Z = Z;
  w = W;
  lc = l;
  Mov = NULL;
  ListSurf = NULL;
  ListCurves = NULL;
  Extruded_Points = NULL;
}

void Vertex::norme (){
  double d = sqrt (Pos.X * Pos.X + Pos.Y * Pos.Y + Pos.Z * Pos.Z);
  if (d == 0.0)
    return;
  Pos.X /= d;
  Pos.Y /= d;
  Pos.Z /= d;
}


Vertex Vertex::operator + (const Vertex & other){
  return Vertex (Pos.X + other.Pos.X, Pos.Y + 
                 other.Pos.Y, Pos.Z + other.Pos.Z, lc, w);
}

Vertex Vertex::operator - (const Vertex & other){
  return Vertex (Pos.X - other.Pos.X, Pos.Y - 
                 other.Pos.Y, Pos.Z - other.Pos.Z, lc, w);
}

Vertex Vertex::operator / (double d){
  return Vertex (Pos.X / d, Pos.Y / d, Pos.Z / d, lc, w);
}
Vertex Vertex::operator * (double d){
  return Vertex (Pos.X * d, Pos.Y * d, Pos.Z * d, lc, w);
}

Vertex Vertex::operator % (Vertex & autre){ // cross product
  return Vertex (Pos.Y * autre.Pos.Z - Pos.Z * autre.Pos.Y,
                 -(Pos.X * autre.Pos.Z - Pos.Z * autre.Pos.X),
                 Pos.X * autre.Pos.Y - Pos.Y * autre.Pos.X, lc, w);
}

double Vertex::operator * (const Vertex & other){
  return Pos.X * other.Pos.X + Pos.Y * other.Pos.Y + Pos.Z * other.Pos.Z;
}

Vertex *Create_Vertex (int Num, double X, double Y, double Z, double lc, double u){
  Vertex *pV;

  pV = new Vertex (X, Y, Z, lc);
  pV->w = 1.0;
  pV->Num = Num;
  THEM->MaxPointNum = IMAX(THEM->MaxPointNum,Num);
  pV->u = u;
  return pV;
}

void Delete_Vertex ( Vertex *pV ){
  if(pV){
    List_Delete(pV->ListSurf);
    List_Delete(pV->ListCurves);
    if(CTX.mesh.oldxtrude){//old automatic extrusion algorithm
      List_Delete(pV->Extruded_Points);
    }
    else{
      Free_ExtrudedPoints(pV->Extruded_Points);
    }
    delete pV;
  }
}

void Free_Vertex (void *a, void *b){
  Delete_Vertex ( *(Vertex**)a );
}

int compareVertex (const void *a, const void *b){
  int i, j;
  Vertex **q, **w;

  q = (Vertex **) a;
  w = (Vertex **) b;
  i = abs ((*q)->Num);
  j = abs ((*w)->Num);
  return (i - j);
}

int comparePosition (const void *a, const void *b){
  int i, j;
  Vertex **q, **w;
  // TOLERANCE ! WARNING WARNING
  double eps = 1.e-08 * CTX.lc;

  q = (Vertex **) a;
  w = (Vertex **) b;
  i = ((*q)->Num);
  j = ((*w)->Num);

  if ((*q)->Pos.X - (*w)->Pos.X > eps)
    return (1);
  if ((*q)->Pos.X - (*w)->Pos.X < -eps)
    return (-1);
  if ((*q)->Pos.Y - (*w)->Pos.Y > eps)
    return (1);
  if ((*q)->Pos.Y - (*w)->Pos.Y < -eps)
    return (-1);
  if ((*q)->Pos.Z - (*w)->Pos.Z > eps)
    return (1);
  if ((*q)->Pos.Z - (*w)->Pos.Z < -eps)
    return (-1);

  if (i != j){
    /*
       *w = *q;
       printf("Les points %d et %d sont a la meme position\n",i,j);
       printf("%12.5E %12.5E %12.5E\n",(*w)->Pos.X,(*w)->Pos.Y,(*w)->Pos.Z);
       printf("%12.5E %12.5E %12.5E\n",(*q)->Pos.X,(*q)->Pos.Y,(*q)->Pos.Z);
    */
  }
  return 0;

}
