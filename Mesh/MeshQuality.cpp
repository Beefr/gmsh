// $Id: MeshQuality.cpp,v 1.6 2002-04-26 00:15:30 geuzaine Exp $

#include "Gmsh.h"
#include "Numeric.h"
#include "Mesh.h"

/* Fonctions calculant differents parametres donnant la qualite
   d'un maillage (surtout 3-D)   */

/* Fonction calculant le facteur gamma pour un element tetraedrique :
               12       rho(in)
   gamma = --------   ---------
             sqrt(6)    max(lij)
 */

static double GAMMAMAX, GAMMAMIN, GAMMA;
static int NbCalcGamma, *Histogram;

void CalculateGamma (void *a, void *b){
  Simplex *s = *(Simplex **) a;
  double gamma = s->GammaShapeMeasure ();
  NbCalcGamma++;
  GAMMAMAX = DMAX (GAMMAMAX, gamma);
  GAMMA += gamma;
  GAMMAMIN = DMIN (GAMMAMIN, gamma);
  for(int i=0; i<NB_HISTOGRAM; i++)
    if(gamma > i/(double)NB_HISTOGRAM && gamma < (i+1)/(double)NB_HISTOGRAM)
      Histogram[i]++;
}

void CalculateEta (void *a, void *b){
  Simplex *s = *(Simplex **) a;
  double gamma = s->EtaShapeMeasure ();
  NbCalcGamma++;
  GAMMAMAX = DMAX (GAMMAMAX, gamma);
  GAMMA += gamma;
  GAMMAMIN = DMIN (GAMMAMIN, gamma);
  for(int i=0; i<NB_HISTOGRAM; i++)
    if(gamma > i/(double)NB_HISTOGRAM && gamma < (i+1)/(double)NB_HISTOGRAM)
      Histogram[i]++;
}

void CalculateR (void *a, void *b){
  Simplex *s = *(Simplex **) a;
  double gamma = s->RhoShapeMeasure ();
  NbCalcGamma++;
  GAMMAMAX = DMAX (GAMMAMAX, gamma);
  GAMMA += gamma;
  GAMMAMIN = DMIN (GAMMAMIN, gamma);
  for(int i=0; i<NB_HISTOGRAM; i++)
    if(gamma > i/(double)NB_HISTOGRAM && gamma < (i+1)/(double)NB_HISTOGRAM)
      Histogram[i]++;
}



static void g(void *a, void *b){
  Volume *v = *(Volume**)a;
  Tree_Action (v->Simplexes, CalculateGamma);  
  Msg(DEBUG, "Gamma computed in volume %d (%d values)", v->Num, NbCalcGamma);
}

void Gamma_Maillage (Mesh * m, double *gamma, double *gammamax, double *gammamin){
  GAMMA = 0.0;
  GAMMAMAX = 0.0;
  GAMMAMIN = 1.0;
  NbCalcGamma = 0;
  Histogram = m->Histogram[0];
  for(int i=0; i<NB_HISTOGRAM; i++) Histogram[i] = 0;
  Tree_Action (m->Volumes, g);
  if(!NbCalcGamma) NbCalcGamma = 1;
  *gamma = GAMMA / (double) NbCalcGamma;
  *gammamax = GAMMAMAX;
  *gammamin = GAMMAMIN;
}

static void e(void *a, void *b){
  Volume *v = *(Volume**)a;
  Tree_Action (v->Simplexes, CalculateEta);  
  Msg(DEBUG, "Eta computed in volume %d (%d values)", v->Num, NbCalcGamma);
}

void Eta_Maillage (Mesh * m, double *gamma, double *gammamax, double *gammamin){
  GAMMA = 0.0;
  GAMMAMAX = 0.0;
  GAMMAMIN = 1.0;
  NbCalcGamma = 0;
  Histogram = m->Histogram[1];
  for(int i=0; i<NB_HISTOGRAM; i++) Histogram[i] = 0;
  Tree_Action (m->Volumes, e);
  if(!NbCalcGamma) NbCalcGamma = 1;
  *gamma = GAMMA / (double) NbCalcGamma;
  *gammamax = GAMMAMAX;
  *gammamin = GAMMAMIN;
}

static void r(void *a, void *b){
  Volume *v = *(Volume**)a;
  Tree_Action (v->Simplexes, CalculateR);  
  Msg(DEBUG, "Rho computed in volume %d (%d values)", v->Num, NbCalcGamma);
}

void R_Maillage (Mesh * m, double *gamma, double *gammamax, double *gammamin){
  GAMMA = 0.0;
  GAMMAMAX = 0.0;
  GAMMAMIN = 1.0;
  NbCalcGamma = 0;
  Histogram = m->Histogram[2];
  for(int i=0; i<NB_HISTOGRAM; i++) Histogram[i] = 0;
  Tree_Action (m->Volumes, r);
  if(!NbCalcGamma) NbCalcGamma = 1;
  *gamma = GAMMA / (double) NbCalcGamma;
  *gammamax = GAMMAMAX;
  *gammamin = GAMMAMIN;
}

void Mesh_Quality(Mesh *m){
  Gamma_Maillage (m, &m->Statistics[17], &m->Statistics[18], &m->Statistics[19]);
  Eta_Maillage (m, &m->Statistics[20], &m->Statistics[21], &m->Statistics[22]);
  R_Maillage (m, &m->Statistics[23], &m->Statistics[24], &m->Statistics[25]);
}

void Print_Histogram(int *h){
  for(int i=0;i<NB_HISTOGRAM;i++)
    Msg(DIRECT, "%g %d", (i+1)/(double)NB_HISTOGRAM, h[i]);
}

