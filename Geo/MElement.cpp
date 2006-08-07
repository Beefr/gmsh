#include <math.h>
#include "MElement.h"
#include "GEntity.h"
#include "Numeric.h"

int MElement::_globalNum = 0;

static double dist(MVertex *v1, MVertex *v2)
{
  double dx = v1->x() - v2->x();
  double dy = v1->y() - v2->y();
  double dz = v1->z() - v2->z();
  return sqrt(dx * dx + dy * dy + dz * dz);
}

double MElement::minEdge()
{
  double m = 1.e25;
  MVertex *v[2];
  for(int i = 0; i < getNumEdges(); i++){
    getEdge(i, v);
    m = std::min(m, dist(v[0], v[1]));
  }
  return m;
}

double MElement::maxEdge()
{
  double m = 0.;
  MVertex *v[2];
  for(int i = 0; i < getNumEdges(); i++){
    getEdge(i, v);
    m = std::max(m, dist(v[0], v[1]));
  }
  return m;
}

double MElement::rhoShapeMeasure()
{
  double min = minEdge();
  double max = maxEdge();
  if(max)
    return min / max;
  else
    return 0.;
}

void MElement::cog(double &x, double &y, double &z)
{
  x = y = z = 0.;
  int n = getNumVertices();
  for(int i = 0; i < n; i++) {
    MVertex *v = getVertex(i);
    x += v->x();
    y += v->y();
    z += v->z();
  }
  x /= (double)n;
  y /= (double)n;
  z /= (double)n;
}

void MElement::writeMSH(FILE *fp, double version, int num, int elementary, 
			int physical)
{
  int n = getNumVertices();
  int type = getTypeForMSH();

  // if necessary, change the ordering of the vertices to get positive
  // volume
  setVolumePositive();

  fprintf(fp, "%d %d", num ? num : _num, type);
  if(version < 2.0)
    fprintf(fp, " %d %d %d", physical, elementary, n);
  else
    fprintf(fp, " 3 %d %d %d", physical, elementary, _partition);
  
  if(physical >= 0){
    for(int i = 0; i < n; i++)
      fprintf(fp, " %d", getVertex(i)->getNum());
  }
  else{
    int nn = n - getNumEdgeVertices() - getNumFaceVertices() - getNumVolumeVertices();
    for(int i = 0; i < nn; i++)
      fprintf(fp, " %d", getVertex(nn - i - 1)->getNum());
    int ne = getNumEdgeVertices();
    for(int i = 0; i < ne; i++)
      fprintf(fp, " %d", getVertex(nn + ne - i - 1)->getNum());
    int nf = getNumFaceVertices();
    for(int i = 0; i < nf; i++)
      fprintf(fp, " %d", getVertex(nn + ne + nf - i - 1)->getNum());
    int nv = getNumVolumeVertices();
    for(int i = 0; i < nv; i++)
      fprintf(fp, " %d", getVertex(n - i - 1)->getNum());
  }

  fprintf(fp, "\n");
}

void MElement::writePOS(FILE *fp, double scalingFactor)
{
  int n = getNumVertices();
  double gamma = gammaShapeMeasure();
  double eta = etaShapeMeasure();
  double rho = rhoShapeMeasure();

  fprintf(fp, "%s(", getStringForPOS());
  for(int i = 0; i < n; i++){
    if(i) fprintf(fp, ",");
    fprintf(fp, "%g,%g,%g", getVertex(i)->x() * scalingFactor, 
	    getVertex(i)->y() * scalingFactor, getVertex(i)->z() * scalingFactor);
  }
  fprintf(fp, "){");
  for(int i = 0; i < n; i++)
    fprintf(fp, "%d,", getVertex(i)->onWhat()->tag());
  for(int i = 0; i < n; i++)
    fprintf(fp, "%d,", getNum());
  for(int i = 0; i < n; i++)
    fprintf(fp, "%g,", gamma);
  for(int i = 0; i < n; i++)
    fprintf(fp, "%g,", eta);
  for(int i = 0; i < n; i++){
    if(i == n - 1)
      fprintf(fp, "%g", rho);
    else
      fprintf(fp, "%g,", rho);
  }
  fprintf(fp, "};\n");
}

void MElement::writeSTL(FILE *fp, double scalingFactor)
{
  int n = getNumVertices();
  if(n < 3 || n > 4) return;

  MVertex *v0 = getVertex(0);
  MVertex *v1 = getVertex(1);
  MVertex *v2 = getVertex(2);
  double N[3];
  normal3points(v0->x(), v0->y(), v0->z(), 
		v1->x(), v1->y(), v1->z(), 
		v2->x(), v2->y(), v2->z(), N);
  fprintf(fp, "facet normal %g %g %g\n", N[0], N[1], N[2]);
  fprintf(fp, "  outer loop\n");
  fprintf(fp, "    vertex %g %g %g\n", v0->x() * scalingFactor, 
	  v0->y() * scalingFactor, v0->z() * scalingFactor);
  fprintf(fp, "    vertex %g %g %g\n", v1->x() * scalingFactor, 
	  v1->y() * scalingFactor, v1->z() * scalingFactor);
  fprintf(fp, "    vertex %g %g %g\n", v2->x() * scalingFactor, 
	  v2->y() * scalingFactor, v2->z() * scalingFactor);
  fprintf(fp, "  endloop\n");
  fprintf(fp, "endfacet\n");
  if(n == 4){
    MVertex *v3 = getVertex(3);
    fprintf(fp, "facet normal %g %g %g\n", N[0], N[1], N[2]);
    fprintf(fp, "  outer loop\n");
    fprintf(fp, "    vertex %g %g %g\n", v0->x() * scalingFactor, 
	    v0->y() * scalingFactor, v0->z() * scalingFactor);
    fprintf(fp, "    vertex %g %g %g\n", v2->x() * scalingFactor, 
	    v2->y() * scalingFactor, v2->z() * scalingFactor);
    fprintf(fp, "    vertex %g %g %g\n", v3->x() * scalingFactor, 
	    v3->y() * scalingFactor, v3->z() * scalingFactor);
    fprintf(fp, "  endloop\n");
    fprintf(fp, "endfacet\n");
  }
}

void MElement::writeVRML(FILE *fp)
{
  for(int i = 0; i < getNumVertices(); i++)
    fprintf(fp, "%d,", getVertex(i)->getNum() - 1);
  fprintf(fp, "-1,\n");
}

void MElement::writeUNV(FILE *fp, int type, int elementary)
{
  // if necessary, change the ordering of the vertices to get positive
  // volume
  setVolumePositive();

  int n = getNumVertices();
  fprintf(fp, "%10d%10d%10d%10d%10d%10d\n",
	  _num, type, elementary, elementary, 7, n);
  if(type == 21 || type == 24) // BEAM or BEAM2
    fprintf(fp, "%10d%10d%10d\n", 0, 0, 0);
  for(int k = 0; k < n; k++) {
    fprintf(fp, "%10d", getVertex(k)->getNum());
    if(k % 8 == 7)
      fprintf(fp, "\n");
  }
  if(n - 1 % 8 != 7)
    fprintf(fp, "\n");
}
