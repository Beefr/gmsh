// $Id: Generator.cpp,v 1.88 2006-08-05 13:31:28 geuzaine Exp $
//
// Copyright (C) 1997-2006 C. Geuzaine, J.-F. Remacle
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

#include "BDS.h"
#include "Gmsh.h"
#include "Numeric.h"
#include "Mesh.h"
#include "Create.h"
#include "Context.h"
#include "OpenFile.h"
#include "Views.h"
#include "PartitionMesh.h"
#include "OS.h"
#include "meshGEdge.h"
#include "meshGFace.h"
#include "GModel.h"

extern Mesh *THEM;
extern Context_T CTX;
extern GModel *GMODEL;

static int nbOrder2 = 0;

void countOrder2(void *a, void *b)
{
  Vertex *v = *(Vertex**)a;
  if(v->Degree == 2) nbOrder2++;
}

void GetStatistics(double stat[50])
{
  for(int i = 0; i < 50; i++)
    stat[i] = 0.;

  if(!THEM)
    return;

  stat[0] = Tree_Nbr(THEM->Points);
  stat[1] = Tree_Nbr(THEM->Curves);
  stat[2] = Tree_Nbr(THEM->Surfaces);
  stat[3] = Tree_Nbr(THEM->Volumes);
  stat[45] = List_Nbr(THEM->PhysicalGroups);
  
  stat[4] = 0.;
  if(Tree_Nbr(THEM->Curves)) {
    List_T *curves = Tree2List(THEM->Curves);
    for(int i = 0; i < List_Nbr(curves); i++){
      Curve *c;
      List_Read(curves, i, &c);
      stat[4] += List_Nbr(c->Vertices);
    }
    List_Delete(curves);
  }

  stat[5] = stat[7] = stat[8] = 0.;
  if(Tree_Nbr(THEM->Surfaces)) {
    List_T *surfaces = Tree2List(THEM->Surfaces);
    for(int i = 0; i < List_Nbr(surfaces); i++){
      Surface *s;
      List_Read(surfaces, i, &s);
      stat[5] += Tree_Nbr(s->Vertices);
      stat[7] += Tree_Nbr(s->Simplexes) + Tree_Nbr(s->SimplexesBase);
      stat[8] += Tree_Nbr(s->Quadrangles);
    }
    List_Delete(surfaces);
  }

  stat[6] = stat[9] = stat[10] = stat[11] = stat[12] = 0.;
  if(Tree_Nbr(THEM->Volumes)) {
    List_T *volumes = Tree2List(THEM->Volumes);
    for(int i = 0; i < List_Nbr(volumes); i++){
      Volume *v;
      List_Read(volumes, i, &v);
      stat[6] += Tree_Nbr(v->Vertices);
      stat[9] += Tree_Nbr(v->Simplexes) + Tree_Nbr(v->SimplexesBase);
      stat[10] += Tree_Nbr(v->Hexahedra);
      stat[11] += Tree_Nbr(v->Prisms);
      stat[12] += Tree_Nbr(v->Pyramids);
    }
    List_Delete(volumes);
  }

  // hack... (Read_Mesh does not fill-in the vertices)
  int nbnod = Tree_Nbr(THEM->Vertices);
  if(nbnod && !stat[4] && !stat[5] && !stat[6]){
    if(stat[9] || stat[10] || stat[11] || stat[12])
      stat[6] = nbnod;
    else if(stat[7] || stat[8])
      stat[5] = nbnod;
    else
      stat[4] = nbnod;
  }

  stat[13] = THEM->timing[0];
  stat[14] = THEM->timing[1];
  stat[15] = THEM->timing[2];

  nbOrder2 = 0;
  Tree_Action(THEM->Vertices, countOrder2);
  stat[16] = nbOrder2;

  stat[17] = THEM->quality_gamma[0];
  stat[18] = THEM->quality_gamma[1];
  stat[19] = THEM->quality_gamma[2];
  stat[20] = THEM->quality_eta[0];
  stat[21] = THEM->quality_eta[1];
  stat[22] = THEM->quality_eta[2];
  stat[23] = THEM->quality_rho[0];
  stat[24] = THEM->quality_rho[1];
  stat[25] = THEM->quality_rho[2];

  stat[26] = List_Nbr(CTX.post.list);
  for(int i = 0; i < List_Nbr(CTX.post.list); i++) {
    Post_View *v = *(Post_View **) List_Pointer(CTX.post.list, i);
    stat[27] += v->NbSP + v->NbVP + v->NbTP;
    stat[28] += v->NbSL + v->NbVL + v->NbTL;
    stat[29] += v->NbST + v->NbVT + v->NbTT;
    stat[30] += v->NbSQ + v->NbVQ + v->NbTQ;
    stat[31] += v->NbSS + v->NbVS + v->NbTS;
    stat[32] += v->NbSH + v->NbVH + v->NbTH;
    stat[33] += v->NbSI + v->NbVI + v->NbTI;
    stat[34] += v->NbSY + v->NbVY + v->NbTY;
    stat[35] += v->NbT2 + v->NbT3;
    if(v->Visible) {
      if(v->DrawPoints)
        stat[36] += 
	  (v->DrawScalars ? v->NbSP : 0) + (v->DrawVectors ? v->NbVP : 0) + 
	  (v->DrawTensors ? v->NbTP : 0);
      if(v->DrawLines)
        stat[37] += 
	  (v->DrawScalars ? v->NbSL : 0) + (v->DrawVectors ? v->NbVL : 0) + 
	  (v->DrawTensors ? v->NbTL : 0);
      if(v->DrawTriangles)
        stat[38] += 
	  (v->DrawScalars ? v->NbST : 0) + (v->DrawVectors ? v->NbVT : 0) + 
	  (v->DrawTensors ? v->NbTT : 0);
      if(v->DrawQuadrangles)
        stat[39] +=
	  (v->DrawScalars ? v->NbSQ : 0) + (v->DrawVectors ? v->NbVQ : 0) + 
	  (v->DrawTensors ? v->NbTQ : 0);
      if(v->DrawTetrahedra)
        stat[40] += 
	  (v->DrawScalars ? v->NbSS : 0) + (v->DrawVectors ? v->NbVS : 0) + 
	  (v->DrawTensors ? v->NbTS : 0);
      if(v->DrawHexahedra)
        stat[41] +=
	  (v->DrawScalars ? v->NbSH : 0) + (v->DrawVectors ? v->NbVH : 0) +
	  (v->DrawTensors ? v->NbTH : 0);
      if(v->DrawPrisms)
        stat[42] += 
	  (v->DrawScalars ? v->NbSI : 0) + (v->DrawVectors ? v->NbVI : 0) +
	  (v->DrawTensors ? v->NbTI : 0);
      if(v->DrawPyramids)
        stat[43] += 
	  (v->DrawScalars ? v->NbSY : 0) + (v->DrawVectors ? v->NbVY : 0) + 
	  (v->DrawTensors ? v->NbTY : 0);
      if(v->DrawStrings)
        stat[44] += v->NbT2 + v->NbT3;
    }
  }

}

static double SumOfAllLc = 0.;
void GetSumOfAllLc(void *a, void *b)
{
  Vertex *v = *(Vertex **) a;
  SumOfAllLc += v->lc;
}

void ApplyLcFactor_Point(void *a, void *b)
{
  Vertex *v = *(Vertex **) a;
  if(v->lc <= 0.0) {
    Msg(GERROR, 
	"Wrong characteristic length (%g <= 0) for Point %d, defaulting to 1.0",
        v->lc, v->Num);
    v->lc = 1.0;
  }
  v->lc *= CTX.mesh.lc_factor;
}

void ApplyLcFactor_Attractor(void *a, void *b)
{
  Attractor *v = *(Attractor **) a;
  v->lc1 *= CTX.mesh.lc_factor;
  v->lc2 *= CTX.mesh.lc_factor;
}

void ApplyLcFactor()
{
  Tree_Action(THEM->Points, ApplyLcFactor_Point);
  List_Action(THEM->Metric->Attractors, ApplyLcFactor_Attractor);
}

void Move_SimplexBaseToSimplex(int dimension)
{
  if(dimension >= 1){
    List_T *Curves = Tree2List(THEM->Curves);
    for(int i = 0; i < List_Nbr(Curves); i++) {
      Curve *c;
      List_Read(Curves, i, &c);
      Move_SimplexBaseToSimplex(&c->SimplexesBase, c->Simplexes);
    }
    List_Delete(Curves);
  }

  if(dimension >= 2){
    List_T *Surfaces = Tree2List(THEM->Surfaces);
    for(int i = 0; i < List_Nbr(Surfaces); i++){
      Surface *s;
      List_Read(Surfaces, i, &s);
      Move_SimplexBaseToSimplex(&s->SimplexesBase, s->Simplexes);
    }
    List_Delete(Surfaces);
  }
  
  if(dimension >= 3){
    List_T *Volumes = Tree2List(THEM->Volumes);
    for(int i = 0; i < List_Nbr(Volumes); i++){
      Volume *v;
      List_Read(Volumes, i, &v);
      Move_SimplexBaseToSimplex(&v->SimplexesBase, v->Simplexes);
    }
    List_Delete(Volumes);
  }
}

bool TooManyElements(int dim){
  if(CTX.expert_mode || !Tree_Nbr(THEM->Points)) return false;

  // try to detect obvious mistakes in characteristic lenghts (one of
  // the most common cause for erroneous bug reports on the mailing
  // list)
  SumOfAllLc = 0.;
  Tree_Action(THEM->Points, GetSumOfAllLc);
  SumOfAllLc /= (double)Tree_Nbr(THEM->Points);
  if(pow(CTX.lc / SumOfAllLc, dim) < 1.e7) return false;
  return !GetBinaryAnswer("Your choice of characteristic lengths will likely produce\n"
			  "a very large mesh. Do you really want to continue?\n\n"
			  "(To disable this warning in the future, select `Enable\n"
			  "expert mode' in the option dialog.)",
			  "Continue", "Cancel");
}

void Maillage_Dimension_1()
{
  if(TooManyElements(1)) return;

  double t1 = Cpu();

  //  Tree_Action(THEM->Curves, Maillage_Curve);

  std::for_each(GMODEL->firstEdge(), GMODEL->lastEdge(), meshGEdge());

  double t2 = Cpu();
  THEM->timing[0] = t2 - t1;
}

void Maillage_Dimension_2()
{
  if(TooManyElements(2)) return;

  double shortest = 1.e300;

  double t1 = Cpu();

  // create reverse 1D meshes

  List_T *Curves = Tree2List(THEM->Curves);
  for(int i = 0; i < List_Nbr(Curves); i++) {
    Curve *c;
    List_Read(Curves, i, &c);
    if(c->Num > 0) {
      if(c->l < shortest)
        shortest = c->l;
      Curve C;
      Curve *neew = &C;
      neew->Num = -c->Num;
      Tree_Query(THEM->Curves, &neew);
      neew->Vertices =
        List_Create(List_Nbr(c->Vertices), 1, sizeof(Vertex *));
      List_Invert(c->Vertices, neew->Vertices);
    }
  }
  List_Delete(Curves);

  Msg(DEBUG, "Shortest curve has length %g", shortest);

  // mesh 2D

  //  Tree_Action(THEM->Surfaces, Maillage_Surface);

  std::for_each (GMODEL->firstFace(),GMODEL->lastFace(), meshGFace() );

  // global "all-quad" recombine

  if(CTX.mesh.algo_recombine == 2)
    Recombine_All(THEM);

  double t2 = Cpu();

  THEM->timing[1] = t2 - t1;
}

static Volume *IVOL;

void TransferData(void *a, void *b)
{
  Simplex *s = *(Simplex**)a;
  if(s->iEnt == IVOL->Num){
    Tree_Add(IVOL->Simplexes, &s);
    for(int i = 0; i < 4; i++)
      Tree_Insert(IVOL->Vertices, &s->V[i]);
  }
}

void Maillage_Dimension_3()
{
  if(TooManyElements(3)) return;

  double t1 = Cpu();

  // merge all the delaunay parts in a single special volume
  Volume *v = Create_Volume(99999, 99999);
  List_T *list = Tree2List(THEM->Volumes);
  for(int i = 0; i < List_Nbr(list); i++) {
    Volume *vol;
    List_Read(list, i, &vol);
    if((!vol->Extrude || !vol->Extrude->mesh.ExtrudeMesh) &&
       (vol->Method != TRANSFINI)) {
      for(int j = 0; j < List_Nbr(vol->Surfaces); j++) {
        List_Replace(v->Surfaces, List_Pointer(vol->Surfaces, j),
                     compareSurface);
      }
    }
  }
  Tree_Insert(THEM->Volumes, &v);

  if(CTX.mesh.oldxtrude) {
    Extrude_Mesh_Old(); // old extrusion
  }
  else {
    Extrude_Mesh(THEM->Volumes); // new extrusion
    Tree_Action(THEM->Volumes, Maillage_Volume); // delaunay of remaining parts
  }

  // transfer data back to individual volumes and remove special volume
  for(int i = 0; i < List_Nbr(list); i++){
    List_Read(list, i, &IVOL);
    Tree_Action(v->Simplexes, TransferData);
  }
  Tree_Suppress(THEM->Volumes, &v);
  Free_Volume_But_Not_Elements(&v, NULL);

  List_Delete(list);

  double t2 = Cpu();

  THEM->timing[2] = t2 - t1;
}

void Init_Mesh0()
{
  THEM->bds = 0;
  THEM->bds_mesh = 0;
  THEM->Vertices = NULL;
  THEM->Simplexes = NULL;
  THEM->Points = NULL;
  THEM->Curves = NULL;
  THEM->SurfaceLoops = NULL;
  THEM->EdgeLoops = NULL;
  THEM->Surfaces = NULL;
  THEM->Volumes = NULL;
  THEM->PhysicalGroups = NULL;
  THEM->Partitions = NULL;
  THEM->Metric = NULL;
}

void Init_Mesh()
{
  THEM->MaxPointNum = 0;
  THEM->MaxLineNum = 0;
  THEM->MaxLineLoopNum = 0;
  THEM->MaxSurfaceNum = 0;
  THEM->MaxSurfaceLoopNum = 0;
  THEM->MaxVolumeNum = 0;
  THEM->MaxPhysicalNum = 0;

  Element::TotalNumber = 0;

  ExitExtrude();

  if(THEM->bds) delete THEM->bds;
  THEM->bds = 0;

  Tree_Action(THEM->Vertices, Free_Vertex);  
  Tree_Delete(THEM->Vertices);

  Tree_Action(THEM->Points, Free_Vertex);  
  Tree_Delete(THEM->Points);

  // Note: don't free the simplices here (with Tree_Action
  // (THEM->Simplexes, Free_Simplex)): we free them in each curve,
  // surface, volume
  Tree_Delete(THEM->Simplexes);

  Tree_Action(THEM->Curves, Free_Curve);
  Tree_Delete(THEM->Curves);

  Tree_Action(THEM->SurfaceLoops, Free_SurfaceLoop);
  Tree_Delete(THEM->SurfaceLoops);

  Tree_Action(THEM->EdgeLoops, Free_EdgeLoop);
  Tree_Delete(THEM->EdgeLoops);

  Tree_Action(THEM->Surfaces, Free_Surface);
  Tree_Delete(THEM->Surfaces);

  Tree_Action(THEM->Volumes, Free_Volume);
  Tree_Delete(THEM->Volumes);

  List_Action(THEM->PhysicalGroups, Free_PhysicalGroup);
  List_Delete(THEM->PhysicalGroups);

  List_Action(THEM->Partitions, Free_MeshPartition);
  List_Delete(THEM->Partitions);

  if(THEM->Metric)
    delete THEM->Metric;

  if(THEM->normals)
    delete THEM->normals;

  THEM->Vertices = Tree_Create(sizeof(Vertex *), compareVertex);
  THEM->Simplexes = Tree_Create(sizeof(Simplex *), compareSimplex);
  THEM->Points = Tree_Create(sizeof(Vertex *), compareVertex);
  THEM->Curves = Tree_Create(sizeof(Curve *), compareCurve);
  THEM->SurfaceLoops = Tree_Create(sizeof(SurfaceLoop *), compareSurfaceLoop);
  THEM->EdgeLoops = Tree_Create(sizeof(EdgeLoop *), compareEdgeLoop);
  THEM->Surfaces = Tree_Create(sizeof(Surface *), compareSurface);
  THEM->Volumes = Tree_Create(sizeof(Volume *), compareVolume);
  THEM->PhysicalGroups = List_Create(5, 5, sizeof(PhysicalGroup *));
  THEM->Partitions = List_Create(5, 5, sizeof(MeshPartition *));
  THEM->Metric = new GMSHMetric;
  THEM->normals = new smooth_normals(CTX.mesh.angle_smooth_normals);

  THEM->status = 0;

  for(int i = 0; i < 3; i++){
    THEM->timing[i] = 0.0;
    THEM->quality_gamma[i] = 0.0;
    THEM->quality_eta[i] = 0.0;
    THEM->quality_rho[i] = 0.0;
  }

  CTX.mesh.bgmesh_type = WITHPOINTS;
  CTX.mesh.changed = 1;
}

void mai3d(int Asked)
{
  double t1, t2;
  int oldstatus;

  if(CTX.threads_lock) {
    Msg(INFO, "I'm busy! Ask me that later...");
    return;
  }

  oldstatus = THEM->status;

  // Re-read data

  if((Asked > oldstatus && Asked >= 0 && oldstatus < 0) ||
     (Asked < oldstatus)) {
    OpenProblem(CTX.filename);
    THEM->status = 0;
  }

  CTX.threads_lock = 1;

  // Clean up all the 2nd order nodes and transfer all SimplexBase
  // into "real" Simplexes

  Degre1();

  // 1D mesh

  if((Asked > oldstatus && Asked > 0 && oldstatus < 1) ||
     (Asked < oldstatus && Asked > 0)) {
    Msg(STATUS1, "Mesh 1D...");
    t1 = Cpu();

    if(THEM->status > 1) {
      OpenProblem(CTX.filename);
    }

    Maillage_Dimension_1();
    t2 = Cpu();
    Msg(STATUS1, "Mesh 1D complete (%g s)", t2 - t1);
    THEM->status = 1;
  }

  // 2D mesh

  if((Asked > oldstatus && Asked > 1 && oldstatus < 2) ||
     (Asked < oldstatus && Asked > 1)) {
    Msg(STATUS1, "Mesh 2D...");
    t1 = Cpu();

    if(THEM->status == 3) {
      OpenProblem(CTX.filename);
      Maillage_Dimension_1();
    }

    Maillage_Dimension_2();
    t2 = Cpu();
    Msg(STATUS1, "Mesh 2D complete (%g s)", t2 - t1);
    THEM->status = 2;
  }

  // 3D mesh

  if((Asked > oldstatus && Asked > 2 && oldstatus < 3) ||
     (Asked < oldstatus && Asked > 2)) {
    Msg(STATUS1, "Mesh 3D...");
    t1 = Cpu();
    Maillage_Dimension_3();
    t2 = Cpu();
    Msg(STATUS1, "Mesh 3D complete (%g s)", t2 - t1);
    THEM->status = 3;
  }

  // Optimize quality

  if(THEM->status == 3 && CTX.mesh.optimize)
    Optimize_Netgen();

  // Create second order elements

  if(THEM->status && CTX.mesh.order == 2)
    Degre2(THEM->status);

  // Partition

  if(THEM->status > 1 && CTX.mesh.nbPartitions != 1)
    PartitionMesh(THEM, CTX.mesh.nbPartitions);

  CTX.threads_lock = 0;
  CTX.mesh.changed = 1;
}
