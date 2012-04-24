#include "GmshMessage.h"
#include "GRegion.h"
#include "MTriangle.h"
#include "MQuadrangle.h"
#include "MTetrahedron.h"
#include "ParamCoord.h"
#include "OptHomMesh.h"
#include "JacobianBasis.h"

std::map<int, std::vector<double> > Mesh::_jacBez;

static std::vector<double> _computeJB(const polynomialBasis *lagrange, const bezierBasis *bezier)
{
  int nbNodes = lagrange->points.size1();
  
  // bezier points are defined in the [0,1] x [0,1] quad
  fullMatrix<double> bezierPoints = bezier->points;
  if (lagrange->parentType == TYPE_QUA) {
    for (int i = 0; i < bezierPoints.size1(); ++i) {
      bezierPoints(i, 0) = -1 + 2 * bezierPoints(i, 0);
      bezierPoints(i, 1) = -1 + 2 * bezierPoints(i, 1);
    }
  }

  fullMatrix<double> allDPsi;
  lagrange->df(bezierPoints, allDPsi);
  int size = bezier->points.size1();
  std::vector<double> JB;
  for (int d = 0; d < lagrange->dimension; ++d) {
    size *= nbNodes;
  }
  JB.resize(size, 0.);
  for (int k = 0; k < bezier->points.size1(); ++k) {
    fullMatrix<double> dPsi(allDPsi, k * 3, 3);
    if (lagrange->dimension == 2) {
      for (int i = 0; i < nbNodes; i++) {
        for (int j = 0; j < nbNodes; j++) {
          double Jij = dPsi(i, 0) * dPsi(j, 1) - dPsi(i, 1) * dPsi(j,0);
          for (int l = 0; l < bezier->points.size1(); l++) {
            JB[(l * nbNodes + i) * nbNodes + j] += bezier->matrixLag2Bez(l, k) * Jij;
          }
        }
      }
    }
    if (lagrange->dimension == 3) {
      for (int i = 0; i < nbNodes; i++) {
        for (int j = 0; j < nbNodes; j++) {
          for (int m = 0; m < nbNodes; m++) {
            double Jijm = 
                (dPsi(j, 1) * dPsi(m, 2) - dPsi(j, 2) * dPsi(m, 1)) * dPsi(i, 0)
              + (dPsi(j, 2) * dPsi(m, 0) - dPsi(j, 0) * dPsi(m, 2)) * dPsi(i, 1)
              + (dPsi(j, 0) * dPsi(m, 1) - dPsi(j, 1) * dPsi(m, 0)) * dPsi(i, 2);
            for (int l = 0; l < bezier->points.size1(); l++) {
              JB[(l * nbNodes + (i * nbNodes + j)) * nbNodes + m] += bezier->matrixLag2Bez(l, k) * Jijm;
            }
          }
        }
      }
    }
  }
  return JB;
}

Mesh::Mesh(GEntity *ge, std::set<MVertex*> &toFix, int method) :
    _ge(ge)
{
  _dim = _ge->dim();

  if (method & METHOD_PHYSCOORD) {
    if (_dim == 2) _pc = new ParamCoordPhys2D;
    else _pc = new ParamCoordPhys3D;
    Msg::Debug("METHOD: Using physical coordinates");
  }
  else if (method & METHOD_SURFCOORD)  {
    if (_dim == 2) {
      _pc = new ParamCoordSurf(_ge);
      Msg::Debug("METHOD: Using surface parametric coordinates");
    }
    Msg::Error("ERROR: Surface parametric coordinates only for 2D optimization");
  }
  else {
    _pc = new ParamCoordParent;
    Msg::Debug("METHOD: Using parent parametric coordinates");
  }

  if (method & METHOD_RELAXBND)Msg::Debug("METHOD: Relaxing boundary vertices");
  else if (method & METHOD_FIXBND) Msg::Debug("METHOD: Fixing all boundary vertices");
  else Msg::Debug("METHOD: Fixing vertices on geometric points and \"toFix\" boundary");

  // Initialize elements, vertices, free vertices and element->vertices connectivity
  _nPC = 0;
  _el.resize(_ge->getNumMeshElements());
  _el2FV.resize(_ge->getNumMeshElements());
  _el2V.resize(_ge->getNumMeshElements());
  _nBezEl.resize(_ge->getNumMeshElements());
  _nNodEl.resize(_ge->getNumMeshElements());
  _indPCEl.resize(_ge->getNumMeshElements());
  for (int iEl = 0; iEl < nEl(); iEl++) {
    MElement *el = _ge->getMeshElement(iEl);
    _el[iEl] = el;
    const polynomialBasis *lagrange = el->getFunctionSpace();
    const bezierBasis *bezier = JacobianBasis::find(lagrange->type)->bezier;
    if (_jacBez.find(lagrange->type) == _jacBez.end()) {
      _jacBez[lagrange->type] = _computeJB(lagrange, bezier);
    }
    _nBezEl[iEl] = bezier->points.size1();
    _nNodEl[iEl] = lagrange->points.size1();
    for (int iVEl = 0; iVEl < lagrange->points.size1(); iVEl++) {
      MVertex *vert = el->getVertex(iVEl);
      int iV = addVert(vert);
      _el2V[iEl].push_back(iV);
      const int nPCV = _pc->nCoord(vert);
      bool isFV = false;
      if (method & METHOD_RELAXBND) isFV = true;
      else if (method & METHOD_FIXBND) isFV = (vert->onWhat()->dim() == _dim) && (toFix.find(vert) == toFix.end());
      else isFV = (vert->onWhat()->dim() >= 1) && (toFix.find(vert) == toFix.end());
      if (isFV) {
        int iFV = addFreeVert(vert,iV,nPCV,toFix);
        _el2FV[iEl].push_back(iFV);
        for (int i=_startPCFV[iFV]; i<_startPCFV[iFV]+nPCV; i++) _indPCEl[iEl].push_back(i);
      }
      else _el2FV[iEl].push_back(-1);
    }
  }

  // Initial coordinates
  _ixyz.resize(nVert());
  for (int iV = 0; iV < nVert(); iV++) _ixyz[iV] = _vert[iV]->point();
  _iuvw.resize(nFV());
  for (int iFV = 0; iFV < nFV(); iFV++) _iuvw[iFV] = _pc->getUvw(_freeVert[iFV]);

  // Set current coordinates
  _xyz = _ixyz;
  _uvw = _iuvw;

  // Set normals to 2D elements (with magnitude of inverse Jacobian) or initial Jacobians of 3D elements
  if (_dim == 2) {
    _normEl.resize(nEl());
    for (int iEl = 0; iEl < nEl(); iEl++) _normEl[iEl] = getNormalEl(iEl);
  }
  else {
    _invStraightJac.resize(nEl(),1.);
    double dumJac[3][3];
    for (int iEl = 0; iEl < nEl(); iEl++) _invStraightJac[iEl] = 1. / _el[iEl]->getPrimaryJacobian(0.,0.,0.,dumJac);
  }
  if ((_dim == 2) && (method & METHOD_PROJJAC)) {
    projJac = true;
    Msg::Debug("METHOD: Using projected Jacobians");
  }
  else {
    projJac = false;
    Msg::Debug("METHOD: Using usual Jacobians");
  }
}

SVector3 Mesh::getNormalEl(int iEl)
{

  switch (_el[iEl]->getType()) {
    case TYPE_TRI: {
      const int iV0 = _el2V[iEl][0], iV1 = _el2V[iEl][1], iV2 = _el2V[iEl][2];
      SVector3 v10 = _xyz[iV1]-_xyz[iV0], v20 = _xyz[iV2]-_xyz[iV0];
      SVector3 n = crossprod(v10, v20);
      double xxx = n.norm();
      n *= 1./(xxx*xxx);
      return n;
      break;
    }
    case TYPE_QUA: {
      const int iV0 = _el2V[iEl][0], iV1 = _el2V[iEl][1], iV3 = _el2V[iEl][3];
      SVector3 v10 = _xyz[iV1]-_xyz[iV0], v30 = _xyz[iV3]-_xyz[iV0];
      SVector3 n = crossprod(v10, v30);
      double xxx = n.norm();
      n *= 4./(xxx*xxx);
      return n;
      break;
    }
    case TYPE_TET: {
      return SVector3(0.);
      break;
    }
    default:
      std::cout << "ERROR: getNormalEl: Unknown element type" << std::endl;
      break;
  }

  return SVector3(0.);  // Just to avoid compilation warnings...

}



int Mesh::addVert(MVertex* vert)
{

  std::vector<MVertex*>::iterator itVert = find(_vert.begin(),_vert.end(),vert);
  if (itVert == _vert.end()) {
    _vert.push_back(vert);
    return _vert.size()-1;
  }
  else return std::distance(_vert.begin(),itVert);

}



int Mesh::addFreeVert(MVertex* vert, const int iV, const int nPCV, std::set<MVertex*> &toFix)
{

  std::vector<MVertex*>::iterator itVert = find(_freeVert.begin(),_freeVert.end(),vert);
  if (itVert == _freeVert.end()) {
    const int iStart = (_startPCFV.size() == 0)? 0 : _startPCFV.back()+_nPCFV.back();
    const bool forcedV = (vert->onWhat()->dim() < 2) || (toFix.find(vert) != toFix.end());
    _freeVert.push_back(vert);
    _fv2V.push_back(iV);
    _startPCFV.push_back(iStart);
    _nPCFV.push_back(nPCV);
    _nPC += nPCV;
    _forced.push_back(forcedV);
    return _freeVert.size()-1;
  }
  else return std::distance(_freeVert.begin(),itVert);

}



void Mesh::getUvw(double *it)
{

//  std::vector<double>::iterator it = uvw.begin();
  for (int iFV = 0; iFV < nFV(); iFV++) {
    SPoint3 &uvwV = _uvw[iFV];
    *it = uvwV[0]; it++;
    if (_nPCFV[iFV] >= 2) { *it = uvwV[1]; it++; }
    if (_nPCFV[iFV] == 3) { *it = uvwV[2]; it++; }
  }

}



void Mesh::updateMesh(const double *it)
{

//  std::vector<double>::const_iterator it = uvw.begin();
  for (int iFV = 0; iFV < nFV(); iFV++) {
    int iV = _fv2V[iFV];
    SPoint3 &uvwV = _uvw[iFV];
    uvwV[0] = *it; it++;
    if (_nPCFV[iFV] >= 2) { uvwV[1] = *it; it++; }
    if (_nPCFV[iFV] == 3) { uvwV[2] = *it; it++; }
    _xyz[iV] = _pc->uvw2Xyz(_freeVert[iFV],uvwV);
  }

}


void Mesh::distSqToStraight(std::vector<double> &dSq)
{
  std::vector<SPoint3> sxyz(nVert());
  for (int iEl = 0; iEl < nEl(); iEl++) {
    MElement *el = _el[iEl];
    const polynomialBasis *lagrange = el->getFunctionSpace();
    const polynomialBasis *lagrange1 = el->getFunctionSpace(1);
    int nV = lagrange->points.size1();
    int nV1 = lagrange1->points.size1();
    for (int i = 0; i < nV1; ++i) {
      sxyz[_el2V[iEl][i]] = _vert[_el2V[iEl][i]]->point();
    }
    for (int i = nV1; i < nV; ++i) {
      double f[256];
      lagrange1->f(lagrange->points(i, 0), lagrange->points(i, 1), lagrange->points(i, 2), f);
      for (int j = 0; j < nV1; ++j)
        sxyz[_el2V[iEl][i]] += sxyz[_el2V[iEl][j]] * f[j];
    }
  }

  for (int iV = 0; iV < nVert(); iV++) {
    SPoint3 d = _xyz[iV]-sxyz[iV];
    dSq[iV] = d[0]*d[0]+d[1]*d[1]+d[2]*d[2];
  }
}



void Mesh::updateGEntityPositions()
{

  for (int iV = 0; iV < nVert(); iV++) _vert[iV]->setXYZ(_xyz[iV].x(),_xyz[iV].y(),_xyz[iV].z());

}



void Mesh::scaledJac(int iEl, std::vector<double> &sJ)
{
  const std::vector<double> &jacBez = _jacBez[_el[iEl]->getTypeForMSH()];
  if (_dim == 2) {
    SVector3 &n = _normEl[iEl];
    if (projJac) {
      for (int l = 0; l < _nBezEl[iEl]; l++) {
        sJ[l] = 0.;
        for (int i = 0; i < _nNodEl[iEl]; i++) {
          int &iVi = _el2V[iEl][i];
          for (int j = 0; j < _nNodEl[iEl]; j++) {
            int &iVj = _el2V[iEl][j];
            sJ[l] += jacBez[indJB2D(iEl,l,i,j)]
                  * (_xyz[iVi].x() * _xyz[iVj].y() * n.z() - _xyz[iVi].x() * _xyz[iVj].z() * n.y()
                     + _xyz[iVi].y() * _xyz[iVj].z() * n.x());
          }
        }
      }
    }
    else
      for (int l = 0; l < _nBezEl[iEl]; l++) {
        sJ[l] = 0.;
        for (int i = 0; i < _nNodEl[iEl]; i++) {
          int &iVi = _el2V[iEl][i];
          for (int j = 0; j < _nNodEl[iEl]; j++) {
            int &iVj = _el2V[iEl][j];
            sJ[l] += jacBez[indJB2D(iEl,l,i,j)] * _xyz[iVi].x() * _xyz[iVj].y();
          }
        }
        sJ[l] *= n.z();
      }
 }
  else {
    for (int l = 0; l < _nBezEl[iEl]; l++) {
      sJ[l] = 0.;
      for (int i = 0; i < _nNodEl[iEl]; i++) {
        int &iVi = _el2V[iEl][i];
        for (int j = 0; j < _nNodEl[iEl]; j++) {
          int &iVj = _el2V[iEl][j];
          for (int m = 0; m < _nNodEl[iEl]; m++) {
            int &iVm = _el2V[iEl][m];
            sJ[l] += jacBez[indJB3D(iEl,l,i,j,m)] * _xyz[iVi].x() * _xyz[iVj].y() * _xyz[iVm].z();
          }
        }
      }
      sJ[l] *= _invStraightJac[iEl];
    }
  }

}



void Mesh::gradScaledJac(int iEl, std::vector<double> &gSJ)
{
  const std::vector<double> &jacBez = _jacBez[_el[iEl]->getTypeForMSH()];
  if (_dim == 2) {
    int iPC = 0;
    SVector3 n = _normEl[iEl];
    if (projJac) {
      for (int i = 0; i < _nNodEl[iEl]; i++) {
        int &iFVi = _el2FV[iEl][i];
        if (iFVi >= 0) {
          std::vector<SPoint3> gXyzV(_nBezEl[iEl],SPoint3(0.,0.,0.));
          std::vector<SPoint3> gUvwV(_nBezEl[iEl]);
          for (int m = 0; m < _nNodEl[iEl]; m++) {
            int &iVm = _el2V[iEl][m];
            const double vpx = _xyz[iVm].y() * n.z() - _xyz[iVm].z() * n.y();
            const double vpy = -_xyz[iVm].x() * n.z() + _xyz[iVm].z() * n.x();
            const double vpz = _xyz[iVm].x() * n.y() - _xyz[iVm].y() * n.x();
            for (int l = 0; l < _nBezEl[iEl]; l++) {
              gXyzV[l][0] += jacBez[indJB2D(iEl,l,i,m)] * vpx;
              gXyzV[l][1] += jacBez[indJB2D(iEl,l,i,m)] * vpy;
              gXyzV[l][2] += jacBez[indJB2D(iEl,l,i,m)] * vpz;
            }
          }
          _pc->gXyz2gUvw(_freeVert[iFVi],_uvw[iFVi],gXyzV,gUvwV);
          for (int l = 0; l < _nBezEl[iEl]; l++) {
            gSJ[indGSJ(iEl,l,iPC)] = gUvwV[l][0];
            if (_nPCFV[iFVi] >= 2) gSJ[indGSJ(iEl,l,iPC+1)] = gUvwV[l][1];
          }
          iPC += _nPCFV[iFVi];
        }
      }
    }
    else
      for (int i = 0; i < _nNodEl[iEl]; i++) {
        int &iFVi = _el2FV[iEl][i];
        if (iFVi >= 0) {
          std::vector<SPoint3> gXyzV(_nBezEl[iEl],SPoint3(0.,0.,0.));
          std::vector<SPoint3> gUvwV(_nBezEl[iEl]);
          for (int m = 0; m < _nNodEl[iEl]; m++) {
            int &iVm = _el2V[iEl][m];
            for (int l = 0; l < _nBezEl[iEl]; l++) {
              gXyzV[l][0] += jacBez[indJB2D(iEl,l,i,m)] * _xyz[iVm].y() * n.z();
              gXyzV[l][1] += jacBez[indJB2D(iEl,l,m,i)] * _xyz[iVm].x() * n.z();
            }
          }
          _pc->gXyz2gUvw(_freeVert[iFVi],_uvw[iFVi],gXyzV,gUvwV);
          for (int l = 0; l < _nBezEl[iEl]; l++) {
            gSJ[indGSJ(iEl,l,iPC)] = gUvwV[l][0];
            if (_nPCFV[iFVi] >= 2) gSJ[indGSJ(iEl,l,iPC+1)] = gUvwV[l][1];
          }
          iPC += _nPCFV[iFVi];
        }
      }
  }
  else {
    int iPC = 0;
    for (int i = 0; i < _nNodEl[iEl]; i++) {
      int &iFVi = _el2FV[iEl][i];
      if (iFVi >= 0) {
        std::vector<SPoint3> gXyzV(_nBezEl[iEl],SPoint3(0.,0.,0.));
        std::vector<SPoint3> gUvwV(_nBezEl[iEl]);
        for (int a = 0; a < _nNodEl[iEl]; a++) {
          int &iVa = _el2V[iEl][a];
          for (int b = 0; b < _nNodEl[iEl]; b++) {
            int &iVb = _el2V[iEl][b];
            for (int l = 0; l < _nBezEl[iEl]; l++) {
              gXyzV[l][0] += jacBez[indJB3D(iEl,l,i,a,b)] * _xyz[iVa].y() * _xyz[iVb].z() * _invStraightJac[iEl];
              gXyzV[l][1] += jacBez[indJB3D(iEl,l,a,i,b)] * _xyz[iVa].x() * _xyz[iVb].z() * _invStraightJac[iEl];
              gXyzV[l][2] += jacBez[indJB3D(iEl,l,a,b,i)] * _xyz[iVa].x() * _xyz[iVb].y() * _invStraightJac[iEl];
            }
          }
        }
        _pc->gXyz2gUvw(_freeVert[iFVi],_uvw[iFVi],gXyzV,gUvwV);
        for (int l = 0; l < _nBezEl[iEl]; l++) {
          gSJ[indGSJ(iEl,l,iPC)] = gUvwV[l][0];
          if (_nPCFV[iFVi] >= 2) gSJ[indGSJ(iEl,l,iPC)] = gUvwV[l][1];
          if (_nPCFV[iFVi] == 3) gSJ[indGSJ(iEl,l,iPC)] = gUvwV[l][2];
        }
        iPC += _nPCFV[iFVi];
      }
    }
  }

}

void Mesh::writeMSH(const char *filename)
{
  FILE *f = fopen(filename, "w");

  fprintf(f, "$MeshFormat\n");
  fprintf(f, "2.2 0 8\n");
  fprintf(f, "$EndMeshFormat\n");

  fprintf(f, "$Nodes\n");
  fprintf(f, "%d\n", nVert());
  for (int i = 0; i < nVert(); i++)
    fprintf(f, "%d %22.15E %22.15E %22.15E\n", i + 1, _xyz[i].x(), _xyz[i].y(), _xyz[i].z());
  fprintf(f, "$EndNodes\n");

  fprintf(f, "$Elements\n");
  fprintf(f, "%d\n", nEl());
  for (int iEl = 0; iEl < nEl(); iEl++) {
//    MElement *MEl = _el[iEl];
    fprintf(f, "%d %d 2 0 %d", iEl+1, _el[iEl]->getTypeForMSH(), _ge->tag());
    for (int iVEl = 0; iVEl < _el2V[iEl].size(); iVEl++) fprintf(f, " %d", _el2V[iEl][iVEl] + 1);
    fprintf(f, "\n");
  }
  fprintf(f, "$EndElements\n");

  fclose(f);

}
